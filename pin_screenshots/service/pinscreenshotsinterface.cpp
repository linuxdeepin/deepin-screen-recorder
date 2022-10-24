// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pinscreenshotsinterface.h"

PinScreenShotsInterface::PinScreenShotsInterface(const QString &serviceName,
                                                 const QString &ObjectPath,
                                                 const QDBusConnection &connection,
                                                 QObject *parent)
    : QDBusAbstractInterface(serviceName, ObjectPath, staticInterfaceName(), connection, parent)

{

}

PinScreenShotsInterface::~PinScreenShotsInterface()
{

}
