// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "cpuvendor.h"
#include <QIODevice>
#include <QByteArray>

bool CpuVendor::isZhaoxin(QIODevice &cpuInfo)
{
    if (!cpuInfo.isReadable())
        return false;
    // /proc files can report size zero even while they contain readable data.
    for (QByteArray line = cpuInfo.readLine(); !line.isEmpty(); line = cpuInfo.readLine()) {
        const int separator = line.indexOf(':');
        if (separator < 0)
            continue;
        if (line.left(separator).trimmed().compare("vendor_id", Qt::CaseInsensitive) == 0
            && line.mid(separator + 1).trimmed().compare("CentaurHauls", Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}
