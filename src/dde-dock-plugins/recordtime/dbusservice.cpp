/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbusservice.h"
#include <QDebug>
DBusService::DBusService(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

bool DBusService::onStart()
{
    emit start();
    return true;
}

bool DBusService::onStop()
{
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
