// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ScreenshotAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ScreenshotAdaptor
 */

ScreenshotAdaptor::ScreenshotAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ScreenshotAdaptor::~ScreenshotAdaptor()
{
    // destructor
}

void ScreenshotAdaptor::DelayScreenshot(qlonglong in0)
{
    // handle method call com.deepin.Screenshot.DelayScreenshot
    QMetaObject::invokeMethod(parent(), "DelayScreenshot", Q_ARG(qlonglong, in0));
}

void ScreenshotAdaptor::FullscreenScreenshot()
{
    // handle method call com.deepin.Screenshot.FullscreenScreenshot
    QMetaObject::invokeMethod(parent(), "FullscreenScreenshot");
}

void ScreenshotAdaptor::NoNotifyScreenshot()
{
    // handle method call com.deepin.Screenshot.NoNotifyScreenshot
    QMetaObject::invokeMethod(parent(), "NoNotifyScreenshot");
}

void ScreenshotAdaptor::SavePathScreenshot(const QString &in0)
{
    // handle method call com.deepin.Screenshot.SavePathScreenshot
    QMetaObject::invokeMethod(parent(), "SavePathScreenshot", Q_ARG(QString, in0));
}

void ScreenshotAdaptor::StartScreenshot()
{
    // handle method call com.deepin.Screenshot.StartScreenshot
    QMetaObject::invokeMethod(parent(), "StartScreenshot");
}

void ScreenshotAdaptor::TopWindowScreenshot()
{
    // handle method call com.deepin.Screenshot.TopWindowScreenshot
    QMetaObject::invokeMethod(parent(), "TopWindowScreenshot");
}

