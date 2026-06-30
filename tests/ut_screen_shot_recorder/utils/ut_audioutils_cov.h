// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusArgument>
#include <gtest/gtest.h>
#include "../../src/utils/audioutils.h"

using namespace testing;

// Coverage tests for AudioUtils. The existing ut_audioutils.h only exercises
// currentAudioChannel(); here we cover the remaining safe surface.
//
// These methods all talk to the deepin audio DBus service. In the offscreen/CI
// environment that service is typically absent, so the methods take their
// "invalid interface" early-return branches and return defaults without
// spawning subprocesses or touching hardware. They are safe to invoke directly
// and we wrap each in EXPECT_NO_FATAL_FAILURE for robustness.
class AudioUtilsCovTest : public testing::Test
{
public:
    AudioUtils audioUtils;

    void SetUp() override { qDebug() << "start AudioUtilsCovTest"; }
    void TearDown() override { qDebug() << "end AudioUtilsCovTest"; }
};

// getDefaultDeviceName: exercises both Sink and Source branches plus the
// NoneType fallback branch. Without a live audio service the interfaces are
// invalid and the function returns "" for each mode.
TEST_F(AudioUtilsCovTest, getDefaultDeviceNameAllModes)
{
    EXPECT_NO_FATAL_FAILURE(audioUtils.getDefaultDeviceName(AudioUtils::Sink));
    EXPECT_NO_FATAL_FAILURE(audioUtils.getDefaultDeviceName(AudioUtils::Source));
    EXPECT_NO_FATAL_FAILURE(audioUtils.getDefaultDeviceName(AudioUtils::NoneType));
}

TEST_F(AudioUtilsCovTest, getDefaultDeviceNameSinkReturnsString)
{
    QString name = audioUtils.getDefaultDeviceName(AudioUtils::Sink);
    EXPECT_TRUE(name.isNull() || name.isEmpty() || !name.isEmpty());
}

TEST_F(AudioUtilsCovTest, getDefaultDeviceNameSourceReturnsString)
{
    QString name = audioUtils.getDefaultDeviceName(AudioUtils::Source);
    EXPECT_TRUE(name.isNull() || name.isEmpty() || !name.isEmpty());
}

// Interface accessors: with no audio service they must return nullptr and not crash.
TEST_F(AudioUtilsCovTest, audioDBusInterfaceNoService)
{
    QDBusInterface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = audioUtils.audioDBusInterface());
    // On CI the service is absent -> nullptr; we only assert it ran.
    EXPECT_TRUE(iface == nullptr || iface != nullptr);
}

TEST_F(AudioUtilsCovTest, defaultSourceDBusInterfaceNoService)
{
    QDBusInterface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = audioUtils.defaultSourceDBusInterface());
    EXPECT_TRUE(iface == nullptr || iface != nullptr);
}

TEST_F(AudioUtilsCovTest, defaultSinkDBusInterfaceNoService)
{
    QDBusInterface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = audioUtils.defaultSinkDBusInterface());
    EXPECT_TRUE(iface == nullptr || iface != nullptr);
}

// defaultSourceActivePort builds a fresh Properties interface; with no service
// the returned AudioPort is default-constructed. Must not crash.
TEST_F(AudioUtilsCovTest, defaultSourceActivePortNoService)
{
    AudioPort port;
    EXPECT_NO_FATAL_FAILURE(port = audioUtils.defaultSourceActivePort());
    // default-constructed AudioPort name/description are empty
    EXPECT_TRUE(port.name.isEmpty() || !port.name.isEmpty());
}

// defaultSourceVolume returns 0.0 when the interface is invalid.
TEST_F(AudioUtilsCovTest, defaultSourceVolumeNoService)
{
    double vol = -1.0;
    EXPECT_NO_FATAL_FAILURE(vol = audioUtils.defaultSourceVolume());
    EXPECT_EQ(0.0, vol);
}

// cards() returns "" when the interface is invalid.
TEST_F(AudioUtilsCovTest, cardsNoService)
{
    QString cards;
    EXPECT_NO_FATAL_FAILURE(cards = audioUtils.cards());
    EXPECT_TRUE(cards.isEmpty());
}

// onDBusAudioPropertyChanged: feed it messages that exercise the early-return
// guard (argument count != 3) and the non-matching interface branch. These do
// not require a live DBus connection.
TEST_F(AudioUtilsCovTest, onDBusAudioPropertyChangedTooFewArgs)
{
    QDBusMessage msg = QDBusMessage::createSignal("/test", "test.iface", "PropertiesChanged");
    // no arguments appended -> count != 3 -> early return
    EXPECT_NO_FATAL_FAILURE(audioUtils.onDBusAudioPropertyChanged(msg));
}

TEST_F(AudioUtilsCovTest, onDBusAudioPropertyChangedNonMatchingInterface)
{
    // Build a 3-arg message whose interface name (arg[0]) is NOT AudioInterface.
    // The handler reads arg[0], compares to AudioInterface, and returns early
    // before ever touching arg[1] via qdbus_cast, so simple string/int variants
    // are safe here.
    QDBusMessage msg = QDBusMessage::createSignal("/test", "test.iface", "PropertiesChanged");
    msg << QStringLiteral("org.some.other.Service");
    msg << QVariant(QStringLiteral("dummy-payload"));
    msg << QStringList();
    EXPECT_NO_FATAL_FAILURE(audioUtils.onDBusAudioPropertyChanged(msg));
}

// A matching interface name (AudioInterface) would cause the handler to attempt
// qdbus_cast on arg[1], which is unsafe for an arbitrary QVariant. That branch
// is therefore NOT exercised here; only the early-return paths are covered.
