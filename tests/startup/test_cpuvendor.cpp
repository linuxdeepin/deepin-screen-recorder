// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#include <QBuffer>
#include <QDebug>
#include "../../src/utils/cpuvendor.h"
static bool detect(QIODevice &source)
{
    return CpuVendor::isZhaoxin(source);
}
int main()
{
    int failures = 0;
    const struct { QByteArray input; bool expected; } cases[] = {
        {"vendor_id : CentaurHauls\n", true},
        {"processor : 0\n vendor_id\t: centaurhauls\r\n", true},
        {"vendor_id : GenuineIntel\n", false},
        {"vendor_id : AuthenticAMD\n", false},
        {"model name : CentaurHauls\n", false},
        {"vendor_id : CentaurHaulsExtra\n", false},
        {"vendor_id :\n", false},
        {"", false},
        {"CPU implementer : 0x41\n", false},
        {"vendor_id : GenuineIntel\n\nvendor_id : CentaurHauls", true}
    };
    for (const auto &test : cases) {
        QBuffer input;
        input.setData(test.input);
        input.open(QIODevice::ReadOnly);
        if (detect(input) != test.expected) {
            qCritical() << "FAIL: CPU vendor detection" << test.input;
            ++failures;
        }
    }
    QBuffer closed;
    if (detect(closed)) {
        qCritical() << "FAIL: Unreadable metadata must not identify a vendor";
        ++failures;
    }
    qInfo() << "CPU vendor tests:" << failures << "failures";
    return failures ? 1 : 0;
}
