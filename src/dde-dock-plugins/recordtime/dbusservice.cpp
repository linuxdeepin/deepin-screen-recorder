// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusservice.h"
#include <QDebug>
DBusService::DBusService(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

bool DBusService::onStart()
{
    qInfo() << "dbus 开始计时";
    emit start();
    return true;
}

bool DBusService::onStop()
{
    qInfo() << "dbus 停止计时";
    emit stop();
    return true;
}

bool DBusService::onRecording()
{
    emit recording();
    return true;
}

bool DBusService::onPause()
{
    emit pause();
    return true;
}
