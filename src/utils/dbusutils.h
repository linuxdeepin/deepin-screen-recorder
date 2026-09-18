// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSUTILS_H
#define DBUSUTILS_H
#include <QVariant>
#include <QString>


class DBusUtils
{
public:
    DBusUtils();
    ~DBusUtils();
    static QVariant redDBusProperty(const QString &service, const QString &path, const QString &interface = QString(), const char* propert = "");
    static QVariant redDBusMethod(const QString &service, const QString &path, const QString &interface, const char *method);
    // 检测 AI 助手能力
    static bool isAiAssistantAvailable();
};

#endif // DBUSUTILS_H
