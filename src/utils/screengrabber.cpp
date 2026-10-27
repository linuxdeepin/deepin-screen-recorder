// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screengrabber.h"

#include "../utils.h"
#include "configsettings.h"
#include "saveutils.h"
#include "../dbusinterface/dbusnotify.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDir>
#include <QPixmap>
#include <QScreen>
#include <QDebug>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QThread>
#include <QMimeData>
#include <QClipboard>
#include <QBuffer>
#include <QDateTime>
#include <QFileInfo>
#include <QCoreApplication>
#include <QPainter>
#include <QTemporaryFile>
#include <QDBusUnixFileDescriptor>
#include <ctime>
ScreenGrabber::ScreenGrabber(QObject *parent)
    : QObject(parent)
{
}

// 调用 KWin ScreenShot2 的指定方法并读取共享内存文件中的原生帧。
// leadingArgs 为方法在 options/fd 之前的参数（如 CaptureScreen 的屏幕名）
static QPixmap callScreenShot2(const QString &method, const QVariantList &leadingArgs, bool &ok)
{
    ok = false;

    QVariantMap options;
    options.insert(QStringLiteral("include-cursor"), false);
    options.insert(QStringLiteral("include-decoration"), true);
    options.insert(QStringLiteral("include-shadow"), false);
    options.insert(QStringLiteral("native-resolution"), true);

    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        qWarning() << method << "cannot create temp file for frame buffer";
        return QPixmap();
    }

    QVariantList args = leadingArgs;
    args << QVariant::fromValue(options)
         << QVariant::fromValue(QDBusUnixFileDescriptor(tmpFile.handle()));

    // 注意：不能用 QDBusInterface::call(method, args)——它没有 QVariantList 重载，
    // 整个参数列表会被当作单个 av 参数发出，kwin 报 No such method (signature 'av')。
    // 必须用 QDBusMessage::setArguments 逐个编组（CaptureActiveScreen 为 a{sv},h）。
    QDBusMessage msg = QDBusMessage::createMethodCall(
                           QStringLiteral("org.kde.KWin"),
                           QStringLiteral("/org/kde/KWin/ScreenShot2"),
                           QStringLiteral("org.kde.KWin.ScreenShot2"), method);
    msg.setArguments(args);
    QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qWarning() << method << "failed:" << reply.error().message();
        return QPixmap();
    }

    const QVariantMap meta = reply.value();
    const int width = meta.value(QStringLiteral("width")).toInt();
    const int height = meta.value(QStringLiteral("height")).toInt();
    const int stride = meta.value(QStringLiteral("stride")).toInt();
    const int format = meta.value(QStringLiteral("format")).toInt();
    qDebug() << method << "native frame:" << width << height << stride << format;
    if (width <= 0 || height <= 0 || stride < width * 4
            || format != static_cast<int>(QImage::Format_ARGB32)) {
        qWarning() << method << "unexpected metadata";
        return QPixmap();
    }

    // KWin 异步向共享内存文件写入帧数据，轮询等待写入完成
    const qint64 needBytes = static_cast<qint64>(stride) * height;
    for (int i = 0; i < 20; ++i) {
        if (QFileInfo(tmpFile.fileName()).size() >= needBytes) {
            break;
        }
        QThread::msleep(50);
    }
    if (QFileInfo(tmpFile.fileName()).size() < needBytes) {
        qWarning() << method << "buffer incomplete";
        return QPixmap();
    }

    QFile file(tmpFile.fileName());
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << method << "buffer open failed";
        return QPixmap();
    }
    QByteArray buffer;
    buffer.resize(int(needBytes));
    if (file.read(buffer.data(), buffer.size()) != buffer.size()) {
        qWarning() << method << "buffer read failed";
        return QPixmap();
    }

    QImage img(reinterpret_cast<const uchar *>(buffer.constData()), width, height, stride,
               QImage::Format_ARGB32);
    // 注意：必须置 ok=true，否则调用方会把抓到的原生帧当作失败丢弃、回落旧接口
    ok = true;
    return QPixmap::fromImage(img.copy());
}

// 通过老接口 org.kde.kwin.Screenshot.screenshotFullscreen 抓整桌图（返回逻辑分辨率），
// 成功后删除 kwin 生成的临时文件
static QPixmap grabFullscreenViaKWin(bool &ok)
{
    ok = false;
    QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                 QStringLiteral("/Screenshot"),
                                 QStringLiteral("org.kde.kwin.Screenshot"));
    QDBusReply<QString> reply;
    QPixmap res;
    for (int count = 0; count < 6; ++count) {
        reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
        res = QPixmap(reply.value());
        if (!res.isNull()) {
            break;
        }
        QThread::msleep(1000);
    }
    if (res.isNull()) {
        qDebug() << "screenshotFullscreen failed after retries";
        return QPixmap();
    }
    QFile dbusResult(reply.value());
    dbusResult.remove();
    ok = true;
    return res;
}

// 分数缩放下 kwin screenshotFullscreen 返回的是原生帧左下角裁剪图（内容被放大），
// 改用 ScreenShot2 接口按 native-resolution 抓取完整原生帧
QPixmap ScreenGrabber::grabNativeFramebuffer(bool &ok)
{
    return callScreenShot2(QStringLiteral("CaptureActiveScreen"), QVariantList(), ok);
}

/**
 * @brief 多屏（各屏缩放比可能不同，如内屏 150% + 外屏 100%）时按屏抓取原生分辨率图像，
 *        拼接到统一缩放比为 canvasScale 的画布上：屏幕 i 的内容绘制在该屏逻辑区域
 *        × canvasScale 的位置，原生图像缩放适配该区域。画布坐标系满足
 *        "画布像素 = 逻辑坐标 × canvasScale"，下游按单一 devicePixelRatio 走的
 *        裁剪/绘制逻辑无需感知各屏缩放比差异。
 *        某块屏按屏抓取失败时整体回落 screenshotFullscreen 整桌图（逻辑分辨率），
 *        等比放大到画布尺寸，几何仍正确。
 */
static QPixmap grabNativeDesktop(bool &ok, qreal canvasScale)
{
    ok = false;
    if (canvasScale <= 0.0) {
        canvasScale = 1.0;
    }

    const QList<QScreen *> screens = qApp->screens();
    if (screens.size() < 2) {
        return QPixmap();
    }

    QRect logicalUnion;
    for (QScreen *screen : screens) {
        logicalUnion = logicalUnion.united(screen->geometry());
    }
    if (logicalUnion.isEmpty()) {
        return QPixmap();
    }

    QImage canvas(static_cast<int>(logicalUnion.width() * canvasScale),
                  static_cast<int>(logicalUnion.height() * canvasScale),
                  QImage::Format_ARGB32);
    canvas.fill(Qt::black);

    int grabbedScreens = 0;
    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    for (QScreen *screen : screens) {
        const QRect geometry = screen->geometry();
        bool screenOk = false;
        const QPixmap native = callScreenShot2(QStringLiteral("CaptureScreen"),
                                               QVariantList{QVariant(screen->name())}, screenOk);
        if (!screenOk || native.isNull()) {
            qWarning() << "CaptureScreen failed for screen" << screen->name();
            break;
        }
        ++grabbedScreens;
        const QRectF target((geometry.x() - logicalUnion.x()) * canvasScale,
                            (geometry.y() - logicalUnion.y()) * canvasScale,
                            geometry.width() * canvasScale,
                            geometry.height() * canvasScale);
        painter.drawPixmap(target, native, QRectF(0.0, 0.0, native.width(), native.height()));
    }
    painter.end();

    if (grabbedScreens == screens.size()) {
        ok = true;
        qDebug() << "multi-screen native canvas" << canvas.size() << "scale" << canvasScale;
        return QPixmap::fromImage(canvas);
    }

    // 按屏抓取不可用，回落整桌接口：逻辑整桌图放大到画布尺寸（几何一致，画质略降）
    qWarning() << "per-screen capture unavailable, fallback to screenshotFullscreen";
    bool fullOk = false;
    QPixmap res = grabFullscreenViaKWin(fullOk);
    if (!fullOk) {
        return QPixmap();
    }
    if (res.width() != canvas.width() || res.height() != canvas.height()) {
        res = res.scaled(canvas.width(), canvas.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    ok = true;
    return res;
}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    ok = true;
    if (Utils::isWaylandMode) {
        // 多屏时各屏缩放比可能不同（如内屏 150% + 外屏 100%），逻辑矩形无法用
        // 单一 ratio 换算成设备像素区域；按屏抓原生帧拼接到统一比例为
        // devicePixelRatio 的画布后，仍按 rect × devicePixelRatio 裁剪
        if (qApp->screens().size() > 1) {
            bool multiOk = false;
            const QPixmap canvas = grabNativeDesktop(multiOk, devicePixelRatio);
            if (!multiOk) {
                ok = false;
                qDebug() << __FUNCTION__ << __LINE__ << "multi-screen grab failed";
                return QPixmap();
            }
            QRect recordRect {
                static_cast<int>(rect.x() * devicePixelRatio),
                static_cast<int>(rect.y() * devicePixelRatio),
                static_cast<int>(rect.width() * devicePixelRatio),
                static_cast<int>(rect.height() * devicePixelRatio)
            };
            return canvas.copy(recordRect);
        }

        QRect recordRect {
            static_cast<int>(rect.x() * devicePixelRatio),
            static_cast<int>(rect.y() * devicePixelRatio),
            static_cast<int>(rect.width() * devicePixelRatio),
            static_cast<int>(rect.height() * devicePixelRatio)
        };
        QPixmap res;
        bool nativeOk = false;
        // 分数缩放下优先用 ScreenShot2 抓原生分辨率帧，失败回落整桌接口
        if (!qFuzzyCompare(devicePixelRatio, qreal(qRound(devicePixelRatio)))) {
            res = grabNativeFramebuffer(nativeOk);
            if (nativeOk)
                qDebug() << __FUNCTION__ << __LINE__ << "native framebuffer image" << res.size();
        }
        if (!nativeOk) {
            bool fullOk = false;
            res = grabFullscreenViaKWin(fullOk);
            if (!fullOk) {
                ok = false;
                qDebug() << __FUNCTION__ << __LINE__ << "Get Pixmap:" << res.size() << "screenshotFullscreen try failed";
                return QPixmap();
            }
            // kwin 缩放下 screenshotFullscreen 返回的是逻辑分辨率的图（分数缩放下还是原生帧的
            // 左下角裁剪图），需先放大到物理桌面尺寸，再按物理坐标裁剪；
            // 否则按物理坐标裁剪会越界，越界部分被填黑（保存图片黑底/内容偏左上角的根源）。
            // 整数缩放同样需保证尺寸，否则坐标系不一致。
            // 注意：这里不能用 nativeOk 判断——它已被 grabFullscreenViaKWin 覆写成 true
            const QSize physicalSize(static_cast<int>(rect.width() * devicePixelRatio),
                                     static_cast<int>(rect.height() * devicePixelRatio));
            if (res.size() != physicalSize) {
                qWarning() << __FUNCTION__ << __LINE__ << "fallback image" << res.size()
                           << "scaled to physical size" << physicalSize;
                res = res.scaled(physicalSize.width(), physicalSize.height(),
                                 Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
        }
        return res.copy(recordRect);
    }

    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    return t_primaryScreen->grabWindow(QApplication::desktop()->winId(), rect.x(), rect.y(), rect.width(), rect.height());
}

bool ScreenGrabber::quickFullScreenshot()
{
    qInfo() << "Starting quick fullscreen screenshot...";

    int count = 0;

    // 调用 KWin D-Bus 接口获取全屏截图
    QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                QStringLiteral("/Screenshot"),
                                QStringLiteral("org.kde.kwin.Screenshot"));
    QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
    
    QPixmap screenshot;
    // 优先走 ScreenShot2 原生帧抓取，失败再回落旧接口
    const qreal t_ratio = Utils::devicePixelRatioF();
    bool nativeOk = false;
    if (qApp->screens().size() > 1) {
        // 多屏：CaptureActiveScreen 只能抓到一块屏，须按屏抓取后拼接；
        // 各屏缩放比可能不同(如内屏 150% + 外屏 100%)，拼接到统一比例的画布
        screenshot = grabNativeDesktop(nativeOk, t_ratio);
        if (nativeOk)
            qDebug() << "quick fullscreen multi-screen image" << screenshot.size();
    } else if (!qFuzzyCompare(t_ratio, qreal(qRound(t_ratio)))) {
        screenshot = grabNativeFramebuffer(nativeOk);
        if (nativeOk)
            qDebug() << "quick fullscreen native image" << screenshot.size();
    }
    while (count < 6 && screenshot.isNull()) {
        reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
        screenshot = QPixmap(reply.value());
        qDebug() << __FUNCTION__ << __LINE__ << "screenshotFullscreen reply:" << reply;
        if (!screenshot.isNull()) {
            break;
        }
        count++;
        QThread::msleep(1000);
    }

    if (!reply.isValid() || reply.value().isEmpty()) {
        qWarning() << "Failed to get fullscreen screenshot:" << reply.error().message();
        qDebug() << __FUNCTION__ << __LINE__ << "Get Pixmap:" << screenshot.size() << "try failed at " << count << "times";
        return false;
    }
    
    QString tempImagePath = reply.value();
    
    if (screenshot.isNull()) {
        qWarning() << "Unable to load screenshot file:" << tempImagePath;
        QFile::remove(tempImagePath);
        return false;
    }
    
    // 获取用户配置
    ConfigSettings *config = ConfigSettings::instance();
    SaveAction saveAction = static_cast<SaveAction>(config->value("save", "save_op").toInt());
    int pictureFormat = config->value("save", "format").toInt();
    
    QString formatStr, formatSuffix;
    switch (pictureFormat) {
    case 0:
        formatStr = "PNG";
        formatSuffix = "png";
        break;
    case 1:
        formatStr = "JPG";
        formatSuffix = "jpg";
        break;
    case 2:
        formatStr = "BMP";
        formatSuffix = "bmp";
        break;
    default:
        formatStr = "PNG";
        formatSuffix = "png";
    }
    
    QString currentTime = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString functionTypeStr = QObject::tr("FullScreenshot");
    QString saveFileName;
    
    // 保存到文件
    QString savePath;
    switch (saveAction) {
    case SaveToDesktop:
        savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        break;
    case SaveToImage:
        savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first() + QDir::separator() + "Screenshots";
        break;
    case SaveToSpecificDir:
        savePath = config->value("save", "savepath").toString();
        if (savePath.isEmpty()) {
            savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first() + QDir::separator() + "Screenshots";
        }
        break;
    default:
        savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first() + QDir::separator() + "Screenshots";
    }

    QDir saveDir(savePath);
    if (!saveDir.exists()) {
        saveDir.mkpath(savePath);
    }
    QFileInfo dirInfo(savePath);
    if (dirInfo.isDir() && dirInfo.exists()) {
        if (!dirInfo.isWritable()) {
            qWarning() << "Check the write permissions, the file cannot be written to: " << savePath;
        }
    }
    saveFileName = QString("%1/%2_%3.%4").arg(savePath, functionTypeStr, currentTime, formatSuffix);

    if (!screenshot.save(saveFileName, formatStr.toLatin1().data())) {
        qWarning() << "Failed to save screenshot:" << saveFileName;
        QFile::remove(tempImagePath);
        return false;
    }
    qDebug() << "saveFilePath:" << saveFileName;

    // 保存到剪贴板
    QMimeData *imageData = new QMimeData;
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    screenshot.save(&buffer, "PNG", 75);  // 使用 PNG 格式，75% 质量以优化性能
    imageData->setData("image/png", bytes);

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setMimeData(imageData, QClipboard::Clipboard);

    // Wayland 模式下添加等待机制，确保剪贴板数据传输完成
    if (Utils::isWaylandMode) {
        qInfo() << "Waiting for clipboard data transfer to complete in Wayland mode...";
        time_t endTime = time(nullptr) + 1;  // 等待 1 秒
        while (time(nullptr) < endTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
        qInfo() << "Clipboard wait completed";
    }

    // 发送系统通知
    QStringList actions;
    QVariantMap hints;
    QString notifyTitle = QString("");
    QString notifyBody;
    
    if (saveAction == SaveToClipboard) {
        notifyBody = QCoreApplication::translate("MainWindow", "Screenshot finished and copy to clipboard");
    } else {
        notifyBody = QCoreApplication::translate("MainWindow", "Screenshot finished");
        actions << "_open" << QCoreApplication::translate("MainWindow", "View");
        actions << "_open1" << QCoreApplication::translate("MainWindow", "Open Folder");
        
        QString command = QString("xdg-open,%1").arg(saveFileName);
        QString savepathcommand;
        if (!QStandardPaths::findExecutable("dde-file-manager").isEmpty()) {
            savepathcommand = QString("dde-file-manager,--show-item,%1").arg(saveFileName);
        }
        hints["x-deepin-action-_open"] = command;
        hints["x-deepin-action-_open1"] = savepathcommand;
    }
    QList<QVariant> arg;
    arg << QCoreApplication::applicationName()
        << (unsigned int) 0
        << QString("deepin-screen-recorder")
        << QCoreApplication::translate("MainWindow", "Screen Capture")
        << notifyBody
        << actions
        << hints
        << 5000;
    const QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.isConnected()) {
        QDBusMessage notifyCall = QDBusMessage::createMethodCall(
            QStringLiteral("org.freedesktop.Notifications"),
            QStringLiteral("/org/freedesktop/Notifications"),
            QStringLiteral("org.freedesktop.Notifications"),
            QStringLiteral("Notify"));
        notifyCall.setArguments(arg);
        sessionBus.call(notifyCall, QDBus::NoBlock);
    } else {
        qWarning() << "Session D-Bus is not connected, skip notification";
    }
    
    // 清理临时文件
    QFile::remove(tempImagePath);
    
    qInfo() << "Quick fullscreen screenshot completed!";
    return true;
}
