// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QDBusConnection>
#include <gtest/gtest.h>
#include "../../src/utils/voicevolumewatcher_interface.h"

using namespace testing;

// Coverage tests for voicevolumewatcher_interface — a generated QDBus proxy.
// All methods are inline wrappers around property()/asyncCallWithArgumentList().
// Without a live audio daemon they return errors quickly but execute the body.
class VoiceVolumeWatcherInterfaceCovTest : public testing::Test
{
public:
    voicevolumewatcher_interface *m_iface = nullptr;
    void SetUp() override
    {
        m_iface = new voicevolumewatcher_interface(
            QStringLiteral("org.deepin.daemon.Audio"),
            QStringLiteral("/org/deepin/daemon/Audio"),
            QDBusConnection::sessionBus());
    }
    void TearDown() override { delete m_iface; }
};

// ---- property getters ----
TEST_F(VoiceVolumeWatcherInterfaceCovTest, BluetoothAudioMode_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->bluetoothAudioMode());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, BluetoothAudioModeOpts_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->bluetoothAudioModeOpts());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, Cards_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->cards());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, CardsWithoutUnavailable_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->cardsWithoutUnavailable());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, DefaultSink_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->defaultSink());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, DefaultSource_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->defaultSource());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, IncreaseVolume_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->increaseVolume());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, MaxUIVolume_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->maxUIVolume());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, ReduceNoise_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->reduceNoise());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, SinkInputs_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->sinkInputs());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, Sinks_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->sinks());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, Sources_GetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->sources());
}

// ---- property setters ----
TEST_F(VoiceVolumeWatcherInterfaceCovTest, SetIncreaseVolume_SetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setIncreaseVolume(true));
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, SetReduceNoise_SetterNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->setReduceNoise(false));
}

// ---- D-Bus methods ----
TEST_F(VoiceVolumeWatcherInterfaceCovTest, IsPortEnabled_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->IsPortEnabled(0u, QStringLiteral("speaker")));
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, Reset_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->Reset());
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, SetBluetoothAudioMode_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetBluetoothAudioMode(QStringLiteral("a2dp")));
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, SetDefaultSink_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetDefaultSink(QStringLiteral("/sink0")));
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, SetDefaultSource_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetDefaultSource(QStringLiteral("/source0")));
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, SetPort_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetPort(0u, QStringLiteral("speaker"), 1));
}

TEST_F(VoiceVolumeWatcherInterfaceCovTest, SetPortEnabled_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_iface->SetPortEnabled(0u, QStringLiteral("speaker"), true));
}
