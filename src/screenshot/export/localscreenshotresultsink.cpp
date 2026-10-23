// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localscreenshotresultsink.h"

#include "../../dbus_name.h"

#include <QBuffer>
#include <QClipboard>
#include <QCoreApplication>
#include <QDateTime>
#include <QDBusConnection>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeData>
#include <QStandardPaths>
#include <QTimer>

namespace Dsr::Screenshot {

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
                                     QString *error)
{
    if (!result.isValid()) {
        if (error)
            *error = QStringLiteral("截图结果无效");
        return false;
    }

    const QString path = resolvedPath(requestedPath);
    QDir directory = QFileInfo(path).dir();
    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        if (error)
            *error = QStringLiteral("无法创建截图保存目录：%1").arg(directory.absolutePath());
        return false;
    }
    if (!result.image.save(path)) {
        if (error)
            *error = QStringLiteral("截图保存失败：%1").arg(path);
        return false;
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

    // Wayland 下 QClipboard::setImage 只暴露 application/x-qt-image，
    // DDE 剪贴板服务无法稳定接管。沿用旧链路的 image/png MIME 数据，并
    // 等待 clipboard loader 的 dataComing 反馈；超时后仍保留系统剪贴板
    // 的写入结果，不能因为剪贴板服务异常而让截图流程失败。
    QByteArray png;
    QBuffer buffer(&png);
    if (!buffer.open(QIODevice::WriteOnly) || !result.image.save(&buffer, "PNG")) {
        if (error)
            *error = QStringLiteral("截图编码为 PNG 失败");
        return false;
    }

    QEventLoop waitLoop;
    QTimer timeout;
    if (m_clipboardSignalConnected) {
        timeout.setSingleShot(true);
        connect(&timeout, &QTimer::timeout, &waitLoop, &QEventLoop::quit);
        connect(this, &LocalScreenshotResultSink::clipboardDataComing,
                &waitLoop, &QEventLoop::quit);
        timeout.start(1500);
    }

    auto *mimeData = new QMimeData;
    mimeData->setData(QStringLiteral("image/png"), png);
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setMimeData(mimeData, QClipboard::Clipboard);

    if (m_clipboardSignalConnected) {
        waitLoop.exec();
    } else {
        // 没有 clipboard loader 时给 Wayland 数据源至少一次事件循环，
        // 避免调用方立即退出导致刚设置的 MIME 数据被销毁。
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    return true;
}

void LocalScreenshotResultSink::onClipboardDataComing(const QByteArray &data)
{
    Q_UNUSED(data)
    emit clipboardDataComing();
}

QString LocalScreenshotResultSink::resolvedPath(const QString &requestedPath) const
{
    if (!requestedPath.isEmpty()) {
        const QFileInfo info(requestedPath);
        return info.suffix().isEmpty() ? requestedPath + QStringLiteral(".png") : requestedPath;
    }

    QString directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (directory.isEmpty())
        directory = QDir::homePath();
    directory = QDir(directory).filePath(QStringLiteral("Screenshots"));
    const QString fileName = QStringLiteral("Screenshot_%1.png")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz")));
    return QDir(directory).filePath(fileName);
}

} // namespace Dsr::Screenshot
