// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localscreenshotresultsink.h"

#include "../../dbusinterface/pinscreenshotsinterface.h"
#include "../../utils/configsettings.h"
#include "../../utils/saveutils.h"
#include "../../widgets/imagemenu.h"
#include "../platform/treeland/treelandclipboardholder.h"

#include <QApplication>
#include <QBuffer>
#include <QCoreApplication>
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

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
    if (!decorated.image.save(path, writerFormatForPath(path, format).constData())) {
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
    QByteArray png;
    QBuffer buffer(&png);
    if (!buffer.open(QIODevice::WriteOnly) || !decorated.image.save(&buffer, "PNG")) {
        if (error)
            *error = QStringLiteral("截图编码为 PNG 失败");
        return false;
    }

    const QString clipboardImage = QDir(QStandardPaths::writableLocation(
                                                QStandardPaths::TempLocation))
                                           .filePath(QStringLiteral("dsr-clipboard-%1-%2.png")
                                                             .arg(QCoreApplication::applicationPid())
                                                             .arg(QDateTime::currentMSecsSinceEpoch()));
    QFile imageFile(clipboardImage);
    if (!imageFile.open(QIODevice::WriteOnly | QIODevice::Truncate)
            || imageFile.write(png) != png.size()) {
        if (error)
            *error = QStringLiteral("无法创建截图剪贴板临时文件");
        return false;
    }
    imageFile.close();

    const bool copied = launchTreelandClipboardHolder(QStringLiteral("image"),
                                                       clipboardImage,
                                                       error);
    QFile::remove(clipboardImage);
    return copied;
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

} // namespace Dsr::Screenshot
