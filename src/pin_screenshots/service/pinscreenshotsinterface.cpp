// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pinscreenshotsinterface.h"
#include "../../utils/log.h"

PinScreenShotsInterface::PinScreenShotsInterface(const QString &serviceName,
                                                 const QString &ObjectPath,
                                                 const QDBusConnection &connection,
                                                 QObject *parent)
    : QDBusAbstractInterface(serviceName, ObjectPath, staticInterfaceName(), connection, parent)

{
    qCDebug(dsrApp) << "PinScreenShotsInterface constructor called. Service:" << serviceName << ", Path:" << ObjectPath;
}

PinScreenShotsInterface::~PinScreenShotsInterface()
{
    qCDebug(dsrApp) << "PinScreenShotsInterface destructor called.";
}
