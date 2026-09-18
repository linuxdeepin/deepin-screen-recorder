// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "NotificationsAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class NotificationsAdaptor
 */

NotificationsAdaptor::NotificationsAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

NotificationsAdaptor::~NotificationsAdaptor()
{
    // destructor
}

void NotificationsAdaptor::ClearRecords()
{
    // handle method call org.freedesktop.Notifications.ClearRecords
    QMetaObject::invokeMethod(parent(), "ClearRecords");
}

void NotificationsAdaptor::CloseNotification(uint in0)
{
    // handle method call org.freedesktop.Notifications.CloseNotification
    QMetaObject::invokeMethod(parent(), "CloseNotification", Q_ARG(uint, in0));
}

QString NotificationsAdaptor::GetAllRecords()
{
    // handle method call org.freedesktop.Notifications.GetAllRecords
    QString out0;
    QMetaObject::invokeMethod(parent(), "GetAllRecords", Q_RETURN_ARG(QString, out0));
    return out0;
}

QStringList NotificationsAdaptor::GetCapbilities()
{
    // handle method call org.freedesktop.Notifications.GetCapbilities
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "GetCapbilities", Q_RETURN_ARG(QStringList, out0));
    return out0;
}

QString NotificationsAdaptor::GetServerInformation(QString &out1, QString &out2, QString &out3)
{
    // handle method call org.freedesktop.Notifications.GetServerInformation
    //return static_cast<YourObjectType *>(parent())->GetServerInformation(out1, out2, out3);
}

uint NotificationsAdaptor::Notify(const QString &in0, uint in1, const QString &in2, const QString &in3, const QString &in4, const QStringList &in5, const QVariantMap &in6, int in7)
{
    // handle method call org.freedesktop.Notifications.Notify
    uint out0;
    QMetaObject::invokeMethod(parent(), "Notify", Q_RETURN_ARG(uint, out0), Q_ARG(QString, in0), Q_ARG(uint, in1), Q_ARG(QString, in2), Q_ARG(QString, in3), Q_ARG(QString, in4), Q_ARG(QStringList, in5), Q_ARG(QVariantMap, in6), Q_ARG(int, in7));
    return out0;
}

void NotificationsAdaptor::RemoveRecord(const QString &in0)
{
    // handle method call org.freedesktop.Notifications.RemoveRecord
    QMetaObject::invokeMethod(parent(), "RemoveRecord", Q_ARG(QString, in0));
}

