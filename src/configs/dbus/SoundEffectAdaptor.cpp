// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SoundEffectAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class SoundEffectAdaptor
 */

SoundEffectAdaptor::SoundEffectAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

SoundEffectAdaptor::~SoundEffectAdaptor()
{
    // destructor
}

bool SoundEffectAdaptor::enabled() const
{
    // get the value of property Enabled
    return qvariant_cast< bool >(parent()->property("Enabled"));
}

void SoundEffectAdaptor::setEnabled(bool value)
{
    // set the value of property Enabled
    parent()->setProperty("Enabled", QVariant::fromValue(value));
}

void SoundEffectAdaptor::PlaySystemSound(const QString &in0)
{
    // handle method call com.deepin.daemon.SoundEffect.PlaySystemSound
    QMetaObject::invokeMethod(parent(), "PlaySystemSound", Q_ARG(QString, in0));
}

