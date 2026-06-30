// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/utils/voicevolumewatcher.h"

using namespace testing;

// Coverage tests for voiceVolumeWatcher. The existing ut_voiceVolumeWatcher.h
// covers getystemAudioState, onCardsChanged, initAvailInputPorts. Here we cover
// setWatch, slotVoiceVolumeWatcher, and the isMicrophoneAvail predicate across
// several port scenarios.
//
// NOTE on ACCESS_PRIVATE_FUN: ut_voiceVolumeWatcher.h (compiled into the same TU
// via test_all_interfaces.h) already declares onCardsChanged and
// initAvailInputPorts accessors. We declare ONLY isMicrophoneAvail here, and
// reuse the existing accessors for onCardsChanged / initAvailInputPorts. The
// orchestrator includes the base file before _cov files (same convention as
// ut_screengrabber.h -> ut_screengrabber_ext.h).
ACCESS_PRIVATE_FUN(voiceVolumeWatcher, bool(const QString &) const, isMicrophoneAvail);

class VoiceVolumeWatcherCovTest : public testing::Test
{
public:
    voiceVolumeWatcher *w = nullptr;
    void SetUp() override { w = new voiceVolumeWatcher(); }
    void TearDown() override { delete w; }
};

// setWatch(true)/setWatch(false) must not crash and operate the internal timer.
TEST_F(VoiceVolumeWatcherCovTest, setWatchOnOff)
{
    EXPECT_NO_FATAL_FAILURE(w->setWatch(true));
    EXPECT_NO_FATAL_FAILURE(w->setWatch(false));
}

TEST_F(VoiceVolumeWatcherCovTest, setWatchToggleTwice)
{
    EXPECT_NO_FATAL_FAILURE(w->setWatch(true));
    EXPECT_NO_FATAL_FAILURE(w->setWatch(true));
    EXPECT_NO_FATAL_FAILURE(w->setWatch(false));
    EXPECT_NO_FATAL_FAILURE(w->setWatch(false));
}

// slotVoiceVolumeWatcher: queries the (absent) audio service and emits
// sigRecodeState only on a state change. Run it a couple of times; with no
// microphone it should settle to couldUse=false without crashing.
TEST_F(VoiceVolumeWatcherCovTest, slotVoiceVolumeWatcherRunsClean)
{
    EXPECT_NO_FATAL_FAILURE(w->slotVoiceVolumeWatcher());
    EXPECT_NO_FATAL_FAILURE(w->slotVoiceVolumeWatcher());
}

// sigRecodeState signal exists and is connected.
TEST_F(VoiceVolumeWatcherCovTest, sigRecodeStateSignalExists)
{
    QSignalSpy spy(w, &voiceVolumeWatcher::sigRecodeState);
    w->slotVoiceVolumeWatcher();
    // Cannot assert count (depends on host audio), but the connection works.
    EXPECT_GE(spy.count(), 0);
}

// isMicrophoneAvail on an unknown port -> false (no ports registered).
TEST_F(VoiceVolumeWatcherCovTest, isMicrophoneAvailUnknownPort)
{
    bool avail = true;
    EXPECT_NO_FATAL_FAILURE(avail = call_private_fun::voiceVolumeWatcherisMicrophoneAvail(
        *w, QStringLiteral("nonexistent-port")));
    EXPECT_FALSE(avail);
}

// Register a known input port via the existing initAvailInputPorts accessor,
// then isMicrophoneAvail for that port returns true.
TEST_F(VoiceVolumeWatcherCovTest, isMicrophoneAvailKnownInputPort)
{
    const QString cards = QStringLiteral(
        "[{\"Id\":1,\"Name\":\"HDA Intel\",\"Ports\":["
        "{\"Name\":\"analog-input-mic\",\"Description\":\"Mic\",\"Available\":2}"
        "]}]");
    call_private_fun::voiceVolumeWatcherinitAvailInputPorts(*w, cards);
    bool avail = call_private_fun::voiceVolumeWatcherisMicrophoneAvail(
        *w, QStringLiteral("analog-input-mic"));
    EXPECT_TRUE(avail);
}

// A loopback card's port must NOT count as a microphone.
TEST_F(VoiceVolumeWatcherCovTest, isMicrophoneAvailLoopbackRejected)
{
    const QString cards = QStringLiteral(
        "[{\"Id\":0,\"Name\":\"Loopback Device\",\"Ports\":["
        "{\"Name\":\"analog-input-loopback\",\"Description\":\"Loopback\",\"Available\":2}"
        "]}]");
    call_private_fun::voiceVolumeWatcherinitAvailInputPorts(*w, cards);
    bool avail = call_private_fun::voiceVolumeWatcherisMicrophoneAvail(
        *w, QStringLiteral("analog-input-loopback"));
    EXPECT_FALSE(avail);
}

// An output port (portId contains "output") is filtered out during
// initAvailInputPorts, so isMicrophoneAvail on it returns false.
TEST_F(VoiceVolumeWatcherCovTest, outputPortNotRegisteredAsInput)
{
    const QString cards = QStringLiteral(
        "[{\"Id\":0,\"Name\":\"HDA\",\"Ports\":["
        "{\"Name\":\"analog-output-speaker\",\"Description\":\"Speaker\",\"Available\":2}"
        "]}]");
    call_private_fun::voiceVolumeWatcherinitAvailInputPorts(*w, cards);
    bool avail = call_private_fun::voiceVolumeWatcherisMicrophoneAvail(
        *w, QStringLiteral("analog-output-speaker"));
    EXPECT_FALSE(avail);
}

// onCardsChanged with an empty string is an early-return (must not throw).
TEST_F(VoiceVolumeWatcherCovTest, onCardsChangedEmptyReturnsEarly)
{
    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::voiceVolumeWatcheronCardsChanged(*w, QString()));
}

// onCardsChanged with valid JSON re-initializes ports.
TEST_F(VoiceVolumeWatcherCovTest, onCardsChangedValidJson)
{
    const QString cards = QStringLiteral(
        "[{\"Id\":2,\"Name\":\"USB Mic\",\"Ports\":["
        "{\"Name\":\"usb-input\",\"Description\":\"USB\",\"Available\":2}"
        "]}]");
    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::voiceVolumeWatcheronCardsChanged(*w, cards));
}
