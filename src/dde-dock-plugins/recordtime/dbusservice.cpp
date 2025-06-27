// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusservice.h"
#include "../../utils/log.h"

#include <QDebug>
DBusService::DBusService(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(dsrApp) << "DBusService constructor called.";
}

bool DBusService::onStart()
{
    qCDebug(dsrApp) << "onStart method called.";
    emit start();
    return true;
}

bool DBusService::onStop()
{
    qCDebug(dsrApp) << "onStop method called.";
    emit stop();
    return true;
}

bool DBusService::onRecording()
{
    qCDebug(dsrApp) << "onRecording method called.";
    emit recording();
    return true;
}

bool DBusService::onPause()
{
    qCDebug(dsrApp) << "onPause method called.";
    emit pause();
    return true;
}
