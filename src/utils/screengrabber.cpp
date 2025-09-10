// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screengrabber.h"

#include "../utils.h"
#include "configsettings.h"
#include "saveutils.h"
#include "../dbusinterface/dbusnotify.h"

#include <QDBusInterface>
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
#include <ctime>
ScreenGrabber::ScreenGrabber(QObject *parent)
    : QObject(parent)
{
}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    ok = true;
    if (Utils::isWaylandMode) {
        int count = 0;
        QRect recordRect {
            static_cast<int>(rect.x() * devicePixelRatio),
            static_cast<int>(rect.y() * devicePixelRatio),
            static_cast<int>(rect.width() * devicePixelRatio),
            static_cast<int>(rect.height() * devicePixelRatio)
        };
        QPixmap res;
        QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                     QStringLiteral("/Screenshot"),
                                     QStringLiteral("org.kde.kwin.Screenshot"));
        QDBusReply<QString> reply;
        while (count < 6) {
            reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
            res = QPixmap(reply.value());
            qDebug() << __FUNCTION__ << __LINE__ << "screenshotFullscreen reply:" << reply;
            if (!res.isNull()) {
                break;
            }
            count++;
            QThread::msleep(1000);
        }
        if (!res.isNull()) {
            QFile dbusResult(reply.value());
            dbusResult.remove();
        } else {
            ok = false;
            qDebug() << __FUNCTION__ << __LINE__ << "Get Pixmap:" << res.size() << "try failed at " << count << "times";
        }
        return res.copy(recordRect);
    }

    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    return t_primaryScreen->grabWindow(QApplication::desktop()->winId(), rect.x(), rect.y(), rect.width(), rect.height());
}

bool ScreenGrabber::quickFullScreenshot()
{
    qInfo() << "开始执行快速全屏截图...";

    int count = 0;

    // 调用 KWin D-Bus 接口获取全屏截图
    QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                QStringLiteral("/Screenshot"),
                                QStringLiteral("org.kde.kwin.Screenshot"));
    QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
    
    QPixmap screenshot;
    while (count < 6) {
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
        qWarning() << "获取全屏截图失败:" << reply.error().message();
        qDebug() << __FUNCTION__ << __LINE__ << "Get Pixmap:" << screenshot.size() << "try failed at " << count << "times";
        return false;
    }
    
    QString tempImagePath = reply.value();
    
    if (screenshot.isNull()) {
        qWarning() << "无法加载截图文件:" << tempImagePath;
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
        qWarning() << "保存截图失败:" << saveFileName;
        QFile::remove(tempImagePath);
        return false;
    }

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
        qInfo() << "Wayland 模式下等待剪贴板数据传输完成...";
        time_t endTime = time(nullptr) + 1;  // 等待 1 秒
        while (time(nullptr) < endTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
        qInfo() << "剪贴板等待完成";
    }

    // 发送系统通知
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());
    
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
    qDebug() << "saveFilePath:" << saveFileName;
    QList<QVariant> arg;
    arg << QCoreApplication::applicationName()
        << (unsigned int) 0
        << QString("deepin-screen-recorder")
        << QCoreApplication::translate("MainWindow", "Screen Capture")
        << notifyBody
        << actions
        << hints
        << 5000;
    
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
    
    // 清理临时文件
    QFile::remove(tempImagePath);
    
    qInfo() << "快速全屏截图完成！";
    return true;
}
