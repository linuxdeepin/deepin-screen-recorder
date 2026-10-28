// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordingfiletransaction.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <utility>

namespace Dsr::Screenshot {

RecordingFileTransaction::RecordingFileTransaction(const QString &finalPath)
    : m_finalPath(finalPath)
{
}

void RecordingFileTransaction::setFinalPath(const QString &finalPath)
{
    m_finalPath = finalPath;
}

QString RecordingFileTransaction::finalPath() const
{
    return m_finalPath;
}

QString RecordingFileTransaction::createTemporaryPath(const QString &extension)
{
    if (m_finalPath.isEmpty())
        return QString();

    QString normalizedExtension = extension;
    while (normalizedExtension.startsWith(QLatin1Char('.')))
        normalizedExtension.remove(0, 1);
    if (normalizedExtension.isEmpty())
        normalizedExtension = QStringLiteral("tmp");

    const QFileInfo finalInfo(m_finalPath);
    const QString baseName = finalInfo.fileName().isEmpty()
            ? QStringLiteral("recording")
            : finalInfo.fileName();

    for (int attempt = 0; attempt < 10; ++attempt) {
        const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const QString fileName = QStringLiteral(".%1.%2.%3")
                                         .arg(baseName, token, normalizedExtension);
        const QString path = finalInfo.dir().filePath(fileName);
        QFile reservation(path);
        if (reservation.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
            reservation.close();
            m_temporaryPaths.append(path);
            return path;
        }
    }

    return QString();
}

bool RecordingFileTransaction::publish(const QString &temporaryPath, QString *error)
{
    auto fail = [error](const QString &message) {
        if (error)
            *error = message;
        return false;
    };

    if (m_finalPath.isEmpty())
        return fail(QStringLiteral("最终保存路径为空"));
    if (!m_temporaryPaths.contains(temporaryPath))
        return fail(QStringLiteral("临时文件不属于当前录屏会话"));
    if (!QFileInfo(temporaryPath).isFile())
        return fail(QStringLiteral("录屏临时文件不存在：%1").arg(temporaryPath));

    const QByteArray source = QFile::encodeName(temporaryPath);
    const QByteArray destination = QFile::encodeName(m_finalPath);
    if (std::rename(source.constData(), destination.constData()) != 0) {
        const QString reason = QString::fromLocal8Bit(std::strerror(errno));
        return fail(QStringLiteral("替换录屏文件失败：%1").arg(reason));
    }

    m_temporaryPaths.removeAll(temporaryPath);
    return true;
}

bool RecordingFileTransaction::discard()
{
    bool success = true;
    for (const QString &path : std::as_const(m_temporaryPaths)) {
        if (!path.isEmpty() && QFileInfo::exists(path) && !QFile::remove(path))
            success = false;
    }
    m_temporaryPaths.clear();
    return success;
}

} // namespace Dsr::Screenshot
