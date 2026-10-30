// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDING_FILE_TRANSACTION_H
#define RECORDING_FILE_TRANSACTION_H

#include <QString>
#include <QStringList>

namespace Dsr::Screenshot {

class RecordingFileTransaction final
{
public:
    explicit RecordingFileTransaction(const QString &finalPath = QString());

    void setFinalPath(const QString &finalPath);
    QString finalPath() const;

    QString createTemporaryPath(const QString &extension);
    bool publish(const QString &temporaryPath, QString *error = nullptr);
    bool discard();

private:
    QString m_finalPath;
    QStringList m_temporaryPaths;
};

} // namespace Dsr::Screenshot

#endif // RECORDING_FILE_TRANSACTION_H
