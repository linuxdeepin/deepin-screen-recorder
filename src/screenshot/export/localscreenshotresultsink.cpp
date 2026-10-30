// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localscreenshotresultsink.h"

#include "../../dbus_name.h"
#include "../../dbusinterface/pinscreenshotsinterface.h"
#include "../../utils/configsettings.h"
#include "../../utils/imageeffects.h"
#include "../../utils/saveutils.h"
#include "../../widgets/imagemenu.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QCoreApplication>
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QGuiApplication>
#include <QMimeData>
#include <QStandardPaths>
#include <QTimer>

namespace Dsr::Screenshot {

namespace {

enum class ScreenshotFormat {
    Png,
    Jpeg,
    Bmp
};

struct SavePlan {
    enum class Action {
        Direct,
        Dialog,
        Invalid
    };

    Action action = Action::Invalid;
    QString path;
    QString dialogPath;
    QString filter;
    QString error;
    bool rememberAskDirectory = false;
    bool rememberDialogDirectory = false;
};

ScreenshotFormat configuredFormat()
{
    switch (ConfigSettings::instance()->getValue("shot", "format").toInt()) {
    case 1:
        return ScreenshotFormat::Jpeg;
    case 2:
        return ScreenshotFormat::Bmp;
    default:
        return ScreenshotFormat::Png;
    }
}

QString extensionFor(ScreenshotFormat format)
{
    switch (format) {
    case ScreenshotFormat::Jpeg:
        return QStringLiteral("jpg");
    case ScreenshotFormat::Bmp:
        return QStringLiteral("bmp");
    case ScreenshotFormat::Png:
    default:
        return QStringLiteral("png");
    }
}

QString filterFor(ScreenshotFormat format)
{
    switch (format) {
    case ScreenshotFormat::Jpeg:
        return QStringLiteral("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)");
    case ScreenshotFormat::Bmp:
        return QStringLiteral("BMP (*.bmp);;PNG (*.png);;JPEG (*.jpg *.jpeg)");
    case ScreenshotFormat::Png:
    default:
        return QStringLiteral("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)");
    }
}

QByteArray writerFormatForPath(const QString &path, ScreenshotFormat fallback)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == QStringLiteral("jpg") || suffix == QStringLiteral("jpeg"))
        return QByteArrayLiteral("JPEG");
    if (suffix == QStringLiteral("bmp"))
        return QByteArrayLiteral("BMP");
    if (suffix == QStringLiteral("png"))
        return QByteArrayLiteral("PNG");
    return extensionFor(fallback) == QStringLiteral("jpg")
            ? QByteArrayLiteral("JPEG")
            : extensionFor(fallback).toUpper().toLatin1();
}

QString pathWithExtension(QString path, ScreenshotFormat format)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == QStringLiteral("png")
            || suffix == QStringLiteral("jpg")
            || suffix == QStringLiteral("jpeg")
            || suffix == QStringLiteral("bmp")) {
        return path;
    }
    return path + QLatin1Char('.') + extensionFor(format);
}

QString writableDirectoryOrFallback(const QString &directory, const QString &fallback)
{
    const QFileInfo info(directory);
    if (info.isDir() && info.isWritable())
        return info.absoluteFilePath();
    return fallback;
}

QString desktopDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
}

QString picturesDirectory()
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (directory.isEmpty())
        directory = QDir::homePath();
    return QDir(directory).filePath(QStringLiteral("Screenshots"));
}

QString defaultFileName(ScreenshotFormat format)
{
    const QString base = QStringLiteral("Screenshot_%1")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz")));
    return base + QLatin1Char('.') + extensionFor(format);
}

SavePlan makeDialogPlan(const QString &directory,
                        ScreenshotFormat format,
                        bool rememberAskDirectory,
                        bool rememberDialogDirectory)
{
    SavePlan plan;
    plan.action = SavePlan::Action::Dialog;
    plan.dialogPath = QDir(directory).filePath(defaultFileName(format));
    plan.filter = filterFor(format);
    plan.rememberAskDirectory = rememberAskDirectory;
    plan.rememberDialogDirectory = rememberDialogDirectory;
    return plan;
}

SavePlan makeSavePlan(const QString &requestedPath, ScreenshotFormat format)
{
    if (!requestedPath.isEmpty()) {
        SavePlan plan;
        plan.action = SavePlan::Action::Direct;
        plan.path = pathWithExtension(requestedPath, format);
        return plan;
    }

    auto *settings = ConfigSettings::instance();
    const int saveWays = settings->getValue("shot", "save_ways").toInt();
    if (saveWays == static_cast<int>(SaveWays::Ask)) {
        QString directory = settings->getValue("shot", "save_ask_dir").toString();
        directory = writableDirectoryOrFallback(directory, desktopDirectory());
        return makeDialogPlan(directory, format, true, false);
    }

    const int saveAction = settings->getValue("shot", "save_op").toInt();
    if (saveAction == static_cast<int>(SaveAction::SaveToDesktop)) {
        SavePlan plan;
        plan.action = SavePlan::Action::Direct;
        plan.path = QDir(desktopDirectory()).filePath(defaultFileName(format));
        return plan;
    }
    if (saveAction == static_cast<int>(SaveAction::SaveToImage)) {
        SavePlan plan;
        plan.action = SavePlan::Action::Direct;
        plan.path = QDir(picturesDirectory()).filePath(defaultFileName(format));
        return plan;
    }
    if (saveAction == static_cast<int>(SaveAction::SaveToSpecificDir)) {
        const QString configuredDirectory = settings->getValue("shot", "save_dir").toString();
        const bool chooseOnSave = settings->getValue("shot", "save_dir_change").toBool();
        const QFileInfo configuredInfo(configuredDirectory);
        if (chooseOnSave || !configuredInfo.isDir()) {
            const QString directory = writableDirectoryOrFallback(configuredDirectory,
                                                                  desktopDirectory());
            return makeDialogPlan(directory, format, false, chooseOnSave);
        }

        SavePlan plan;
        plan.action = SavePlan::Action::Direct;
        plan.path = QDir(configuredDirectory).filePath(defaultFileName(format));
        return plan;
    }

    SavePlan plan;
    plan.error = QStringLiteral("不支持的截图保存方式：%1").arg(saveAction);
    return plan;
}

int clipboardTimeoutMs(const QImage &image)
{
    const qint64 pixels = static_cast<qint64>(image.width()) * image.height();
    if (pixels <= 921600)
        return 2000;
    if (pixels <= 2073600)
        return 4000;
    if (pixels <= 3686400)
        return 6000;
    if (pixels <= 8294400)
        return 8000;
    if (pixels <= 33177600)
        return 10000;
    return 15000;
}

Result applyConfiguredBorder(const Result &result)
{
    Result decorated = result;
    QPixmap pixmap = QPixmap::fromImage(result.image);
    decorated.image = ImageBorderHelper::instance()->getPixmapAddBorder(pixmap).toImage();
    return decorated;
}

} // namespace

LocalScreenshotResultSink::LocalScreenshotResultSink(QObject *parent)
    : IScreenshotResultSink(parent)
{
    m_clipboardSignalConnected = QDBusConnection::sessionBus().connect(
            QString::fromLatin1(CLIPBOARD_NAME),
            QString::fromLatin1(CLIPBOARD_PATH),
            QString::fromLatin1(CLIPBOARD_INTERFACE),
            QStringLiteral("dataComing"),
            this,
            SLOT(onClipboardDataComing(const QByteArray &)));
}

bool LocalScreenshotResultSink::save(const Result &result,
                                     const QString &requestedPath,
                                     QString *savedPath,
                                     QString *error,
                                     bool *cancelled)
{
    if (cancelled)
        *cancelled = false;
    if (!result.isValid()) {
        if (error)
            *error = QStringLiteral("截图结果无效");
        return false;
    }

    const Result decorated = applyConfiguredBorder(result);

    const ScreenshotFormat format = configuredFormat();
    SavePlan plan = makeSavePlan(requestedPath, format);
    if (plan.action == SavePlan::Action::Invalid) {
        if (error)
            *error = plan.error;
        return false;
    }

    QString path = plan.path;
    if (plan.action == SavePlan::Action::Dialog) {
        // TreeLand mask windows are compositor-tagged as always-on-top. Keep
        // them visible while selecting/capturing, but hide this process's
        // top-level windows before opening the save dialog; otherwise the
        // mask and toolbar intercept every pointer/keyboard event intended
        // for the native file dialog.
        const QList<QWidget *> topLevelWidgets = QApplication::topLevelWidgets();
        for (QWidget *widget : topLevelWidgets) {
            if (widget && widget->isVisible())
                widget->hide();
        }
        path = QFileDialog::getSaveFileName(nullptr,
                                            QCoreApplication::translate("MainWindow", "Save"),
                                            plan.dialogPath,
                                            plan.filter);
        if (path.isEmpty()) {
            if (cancelled)
                *cancelled = true;
            return false;
        }
        path = pathWithExtension(path, format);
    }

    QDir directory = QFileInfo(path).dir();
    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        if (error)
            *error = QStringLiteral("无法创建截图保存目录：%1").arg(directory.absolutePath());
        return false;
    }
    if (!QFileInfo(directory.absolutePath()).isWritable()) {
        if (error)
            *error = QStringLiteral("截图保存目录不可写：%1").arg(directory.absolutePath());
        return false;
    }
    const QByteArray writerFormat = writerFormatForPath(path, format);
    QImage imageToSave = decorated.image;
    if (writerFormat == QByteArrayLiteral("JPEG")
            || writerFormat == QByteArrayLiteral("BMP")) {
        // Match X11: rounded corners on formats without alpha are composited on
        // white instead of exposing an undefined transparent background.
        imageToSave = Dsr::ImageEffects::compositeOnWhite(imageToSave);
    }
    if (!imageToSave.save(path, writerFormat.constData())) {
        if (error)
            *error = QStringLiteral("截图保存失败：%1").arg(path);
        return false;
    }

    if (plan.rememberAskDirectory) {
        ConfigSettings::instance()->setValue("shot", "save_ask_dir",
                                             QFileInfo(path).dir().absolutePath());
    }
    if (plan.rememberDialogDirectory) {
        ConfigSettings::instance()->setValue("shot", "save_dir",
                                             QFileInfo(path).dir().absolutePath());
        ConfigSettings::instance()->setValue("shot", "save_dir_change", false);
    }

    if (savedPath)
        *savedPath = path;
    return true;
}

bool LocalScreenshotResultSink::copy(const Result &result, QString *error)
{
    if (!result.isValid()) {
        if (error)
            *error = QStringLiteral("截图结果无效");
        return false;
    }

    const Result decorated = applyConfiguredBorder(result);

    // Wayland 下 QClipboard::setImage 只暴露 application/x-qt-image，
    // DDE 剪贴板服务无法稳定接管。沿用旧链路的 image/png MIME 数据，并
    // 等待 clipboard loader 的 dataComing 反馈。只有服务确认接管后才结束
    // 会话，否则 Wayland 数据源会随进程退出而失效。
    QByteArray png;
    QBuffer buffer(&png);
    if (!buffer.open(QIODevice::WriteOnly) || !decorated.image.save(&buffer, "PNG")) {
        if (error)
            *error = QStringLiteral("截图编码为 PNG 失败");
        return false;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *currentMimeData = clipboard->mimeData(QClipboard::Clipboard);
    if (currentMimeData
            && currentMimeData->data(QStringLiteral("image/png")) == png) {
        // dde-clipboard-daemon intentionally ignores identical clipboard data
        // and therefore does not emit dataComing for a repeated copy. The
        // requested payload is already available, so this copy has succeeded.
        return true;
    }

    QEventLoop waitLoop;
    QTimer timeout;
    bool clipboardConfirmed = false;
    m_clipboardReborned = false;
    m_clipboardRebornSucceeded = false;
    m_clipboardRebornError.clear();
    if (m_clipboardSignalConnected) {
        timeout.setSingleShot(true);
        connect(&timeout, &QTimer::timeout, &waitLoop, &QEventLoop::quit);
        connect(this, &LocalScreenshotResultSink::clipboardDataComing,
                &waitLoop, [&clipboardConfirmed, &waitLoop]() {
                    clipboardConfirmed = true;
                    waitLoop.quit();
                });
        timeout.start(clipboardTimeoutMs(decorated.image));
    }

    auto *mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("image/png"), png);
    clipboard->setMimeData(mimeData, QClipboard::Clipboard);

    if (m_clipboardSignalConnected) {
        waitLoop.exec();
        if (!clipboardConfirmed) {
            if (error) {
                *error = QStringLiteral("剪贴板服务未确认接管截图数据");
            }
            return false;
        }
        if (!m_clipboardRebornSucceeded) {
            if (error) {
                *error = m_clipboardRebornError.isEmpty()
                        ? QStringLiteral("剪贴板服务未能重新持有截图数据")
                        : m_clipboardRebornError;
            }
            return false;
        }
    } else {
        // 没有 clipboard loader 时给 Wayland 数据源至少一次事件循环，
        // 避免调用方立即退出导致刚设置的 MIME 数据被销毁。
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    return true;
}

bool LocalScreenshotResultSink::pin(const Result &result, QString *error)
{
    if (!result.isValid()) {
        if (error)
            *error = QStringLiteral("截图结果无效");
        return false;
    }

    const Result decorated = applyConfiguredBorder(result);
    PinScreenShotsInterface pinInterface(QStringLiteral("com.deepin.PinScreenShots"),
                                         QStringLiteral("/com/deepin/PinScreenShots"),
                                         QDBusConnection::sessionBus());
    QDBusPendingReply<> reply = pinInterface.openImageAndName(
            decorated.image,
            defaultFileName(ScreenshotFormat::Png),
            decorated.globalSelection.topLeft());
    reply.waitForFinished();
    if (!reply.isValid() || reply.isError()) {
        if (error) {
            *error = reply.isError()
                    ? QStringLiteral("贴图服务调用失败：%1").arg(reply.error().message())
                    : QStringLiteral("贴图服务调用未返回有效结果");
        }
        return false;
    }

    return true;
}

void LocalScreenshotResultSink::onClipboardDataComing(const QByteArray &data)
{
    if (!m_clipboardReborned) {
        m_clipboardReborned = true;
        // dataComing only means the clipboard daemon has cached the payload.
        // Under Wayland the source is owned by this process, so the daemon must
        // explicitly take ownership before the screenshot process exits.
        QDBusMessage message = QDBusMessage::createMethodCall(
                QString::fromLatin1(CLIPBOARD_NAME),
                QString::fromLatin1(CLIPBOARD_PATH),
                QString::fromLatin1(CLIPBOARD_INTERFACE),
                QStringLiteral("dataReborned"));
        message << data;
        const QDBusMessage reply = QDBusConnection::sessionBus().call(
                message, QDBus::Block, 3000);
        m_clipboardRebornSucceeded = reply.type() != QDBusMessage::ErrorMessage;
        if (!m_clipboardRebornSucceeded) {
            m_clipboardRebornError = QStringLiteral("剪贴板服务重新接管失败：%1")
                    .arg(reply.errorMessage());
            qWarning() << m_clipboardRebornError;
        }
    }
    emit clipboardDataComing();
}

} // namespace Dsr::Screenshot
