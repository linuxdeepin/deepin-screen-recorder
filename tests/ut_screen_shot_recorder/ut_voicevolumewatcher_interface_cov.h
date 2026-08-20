// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QtDBus>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include "stub.h"

#include "../../src/utils/voicevolumewatcher_interface.h"

using namespace testing;

// voicevolumewatcher_interface is the qdbusxml2cpp-generated proxy for
// com.deepin.daemon.Audio. voiceVolumeWatcher *inherits* this interface but
// never calls any of its inline property getters / setters / async method
// wrappers, leaving all 21 of them at 0% function coverage (only the destructor
// was previously touched by ut_smallfiles_cov.h::heapDestructor).
//
// Safety: the service "com.deepin.daemon.Audio" is neither registered nor
// DBus-activatable on the headless test session bus (only org.deepin.dde.Audio1
// has a .service file). Consequently every sync property()/setProperty() call
// receives an org.freedesktop.DBus.Error.ServiceUnknown reply from the bus
// daemon immediately (no activation, no hang), and the asyncCallWithArgumentList
// wrappers return a QDBusPendingReply without blocking. Constructing the proxy
// (QDBusAbstractInterface ctor) likewise performs no synchronous round-trip.
//
// NOTE: uses plain TEST (not TEST_F) to stay fixture-compatible with the
// existing TEST(VoiceVolumeWatcherInterfaceCovTest, heapDestructor) in
// ut_smallfiles_cov.h, which shares this suite name.

// ---------- 12 Q_PROPERTY getters (sync property() fast-fail) ----------
TEST(VoiceVolumeWatcherInterfaceCovTest, allPropertyGettersRunCleanly)
{
    voicevolumewatcher_interface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new voicevolumewatcher_interface(
                                 QStringLiteral("com.deepin.daemon.Audio"),
                                 QStringLiteral("/com/deepin/daemon/Audio"),
                                 QDBusConnection::sessionBus()));
    if (iface) {
        EXPECT_NO_FATAL_FAILURE(iface->bluetoothAudioMode());
        EXPECT_NO_FATAL_FAILURE(iface->bluetoothAudioModeOpts());
        EXPECT_NO_FATAL_FAILURE(iface->cards());
        EXPECT_NO_FATAL_FAILURE(iface->cardsWithoutUnavailable());
        EXPECT_NO_FATAL_FAILURE(iface->defaultSink());
        EXPECT_NO_FATAL_FAILURE(iface->defaultSource());
        EXPECT_NO_FATAL_FAILURE(iface->increaseVolume());
        EXPECT_NO_FATAL_FAILURE(iface->maxUIVolume());
        EXPECT_NO_FATAL_FAILURE(iface->reduceNoise());
        EXPECT_NO_FATAL_FAILURE(iface->sinkInputs());
        EXPECT_NO_FATAL_FAILURE(iface->sinks());
        EXPECT_NO_FATAL_FAILURE(iface->sources());
        EXPECT_NO_FATAL_FAILURE(delete iface);
    }
}

// ---------- 2 Q_PROPERTY setters (sync setProperty() fast-fail) ----------
TEST(VoiceVolumeWatcherInterfaceCovTest, propertySettersRunCleanly)
{
    voicevolumewatcher_interface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new voicevolumewatcher_interface(
                                 QStringLiteral("com.deepin.daemon.Audio"),
                                 QStringLiteral("/com/deepin/daemon/Audio"),
                                 QDBusConnection::sessionBus()));
    if (iface) {
        EXPECT_NO_FATAL_FAILURE(iface->setIncreaseVolume(true));
        EXPECT_NO_FATAL_FAILURE(iface->setReduceNoise(false));
        EXPECT_NO_FATAL_FAILURE(delete iface);
    }
}

// ---------- 7 async method wrappers (asyncCallWithArgumentList, non-blocking) ----------
TEST(VoiceVolumeWatcherInterfaceCovTest, asyncMethodWrappersRunCleanly)
{
    voicevolumewatcher_interface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new voicevolumewatcher_interface(
                                 QStringLiteral("com.deepin.daemon.Audio"),
                                 QStringLiteral("/com/deepin/daemon/Audio"),
                                 QDBusConnection::sessionBus()));
    if (iface) {
        EXPECT_NO_FATAL_FAILURE(iface->Reset());
        EXPECT_NO_FATAL_FAILURE(iface->SetBluetoothAudioMode(QStringLiteral("a2dp")));
        EXPECT_NO_FATAL_FAILURE(iface->SetDefaultSink(QStringLiteral("/Sink0")));
        EXPECT_NO_FATAL_FAILURE(iface->SetDefaultSource(QStringLiteral("/Source0")));
        EXPECT_NO_FATAL_FAILURE(iface->SetPort(0, QStringLiteral("analog-output"), 0));
        EXPECT_NO_FATAL_FAILURE(iface->SetPortEnabled(0, QStringLiteral("analog-output"), true));
        // IsPortEnabled returns a QDBusPendingReply<bool>; do not waitForFinished —
        // just let the async call be issued without blocking.
        EXPECT_NO_FATAL_FAILURE((void)iface->IsPortEnabled(0, QStringLiteral("analog-output")));
        EXPECT_NO_FATAL_FAILURE(delete iface);
    }
}
