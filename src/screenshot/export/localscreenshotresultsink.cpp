// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localscreenshotresultsink.h"

#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QStandardPaths>

namespace Dsr::Screenshot {

LocalScreenshotResultSink::LocalScreenshotResultSink(QObject *parent)
    : IScreenshotResultSink(parent)
{
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

    QGuiApplication::clipboard()->setImage(result.image);
    return true;
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
