// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZoneAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ZoneAdaptor
 */

ZoneAdaptor::ZoneAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ZoneAdaptor::~ZoneAdaptor()
{
    // destructor
}

QString ZoneAdaptor::BottomLeftAction()
{
    // handle method call com.deepin.daemon.Zone.BottomLeftAction
    QString out0;
    QMetaObject::invokeMethod(parent(), "BottomLeftAction", Q_RETURN_ARG(QString, out0));
    return out0;
}

QString ZoneAdaptor::BottomRightAction()
{
    // handle method call com.deepin.daemon.Zone.BottomRightAction
    QString out0;
    QMetaObject::invokeMethod(parent(), "BottomRightAction", Q_RETURN_ARG(QString, out0));
    return out0;
}

void ZoneAdaptor::EnableZoneDetected(bool in0)
{
    // handle method call com.deepin.daemon.Zone.EnableZoneDetected
    QMetaObject::invokeMethod(parent(), "EnableZoneDetected", Q_ARG(bool, in0));
}

void ZoneAdaptor::SetBottomLeft(const QString &in0)
{
    // handle method call com.deepin.daemon.Zone.SetBottomLeft
    QMetaObject::invokeMethod(parent(), "SetBottomLeft", Q_ARG(QString, in0));
}

void ZoneAdaptor::SetBottomRight(const QString &in0)
{
    // handle method call com.deepin.daemon.Zone.SetBottomRight
    QMetaObject::invokeMethod(parent(), "SetBottomRight", Q_ARG(QString, in0));
}

void ZoneAdaptor::SetTopLeft(const QString &in0)
{
    // handle method call com.deepin.daemon.Zone.SetTopLeft
    QMetaObject::invokeMethod(parent(), "SetTopLeft", Q_ARG(QString, in0));
}

void ZoneAdaptor::SetTopRight(const QString &in0)
{
    // handle method call com.deepin.daemon.Zone.SetTopRight
    QMetaObject::invokeMethod(parent(), "SetTopRight", Q_ARG(QString, in0));
}

QString ZoneAdaptor::TopLeftAction()
{
    // handle method call com.deepin.daemon.Zone.TopLeftAction
    QString out0;
    QMetaObject::invokeMethod(parent(), "TopLeftAction", Q_RETURN_ARG(QString, out0));
    return out0;
}

QString ZoneAdaptor::TopRightAction()
{
    // handle method call com.deepin.daemon.Zone.TopRightAction
    QString out0;
    QMetaObject::invokeMethod(parent(), "TopRightAction", Q_RETURN_ARG(QString, out0));
    return out0;
}

