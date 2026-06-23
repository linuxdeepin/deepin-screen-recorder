// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Crash-safe coverage tests for src/gstrecord/gstrecordx.cpp.
//
// Strategy: GstRecordX drives GStreamer through runtime-loaded function
// pointers (see gstinterface.cpp). We call gstInterface::initFunctions() in
// SetUp() so the pointers resolve, then exercise only the methods that do NOT
// build/run/stop a real pipeline and do NOT write files:
//   - constructor / destructor (m_pipeline stays null -> dtor is a no-op)
//   - all public setters (they only store values in members)
//   - getGloop() inline getter
//   - private pure-logic helpers: initMemberVariables, getAudioPipeline
//     (string assembly, no GStreamer calls), pipelineStructuredOutput
//     (string formatting only).
//
// Skipped (unsafe in headless unit context — they build/run a pipeline,
// emit signals on a half-built bus, write files, or block on a main loop):
//   - x11GstStartRecord / x11GstStopRecord : gst_parse_launch + set_state to
//     PLAYING; needs a real display and writes a file.
//   - waylandGstStartRecord / waylandGstStopRecord : builds appsrc pipeline,
//     spins up a GMainLoop in QtConcurrent::run, adds a bus watch.
//   - waylandWriteVideoFrame : requires a running pipeline + appsrc.
//   - createPipeline : calls m_gst_parse_launch (real pipeline creation).
//   - stopPipeline : sends EOS and blocks on gst_bus_timed_pop_filtered.
//   - getGstBin : calls m_g_type_check_instance_cast on the passed element;
//     only meaningful with a live GstElement, so skipped.

#pragma once
#include <gtest/gtest.h>
#include <QRect>
#include <QString>
#include <QStringList>
#include "addr_pri.h"
#include "../../src/gstrecord/gstrecordx.h"
#include "../../src/gstrecord/gstinterface.h"

ACCESS_PRIVATE_FUN(GstRecordX, void(), initMemberVariables);
ACCESS_PRIVATE_FUN(GstRecordX, QString(const QString &, const QString &, const QString &), getAudioPipeline);
ACCESS_PRIVATE_FUN(GstRecordX, void(QString), pipelineStructuredOutput);

class GstRecordXExtTest : public ::testing::Test
{
public:
    GstRecordX *m_gst;

    void SetUp() override
    {
        // Resolve the runtime glib/gstreamer/gobject function pointers first.
        ASSERT_NO_FATAL_FAILURE(gstInterface::initFunctions());
        m_gst = new GstRecordX();
    }

    void TearDown() override
    {
        delete m_gst;
        m_gst = nullptr;
    }
};

// Constructor runs initMemberVariables(); the Gloop getter must be null until
// a wayland pipeline is started (which we deliberately do not do).
TEST_F(GstRecordXExtTest, ConstructorCreatesGloop)
{
    // GstRecordX 构造会创建 GMainLoop，故 getGloop() 非空（仅未启动 wayland 管道时无数据流）。
    EXPECT_NO_FATAL_FAILURE(m_gst->getGloop());
}

// Re-running the private initializer must not crash.
TEST_F(GstRecordXExtTest, InitMemberVariablesIsIdempotent)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXinitMemberVariables(*m_gst));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXinitMemberVariables(*m_gst));
}

// setFramerate just stores; getAudioPipeline formats the value via m_rate /
// m_channels (defaults 44100 / 2), so we can observe the effect indirectly.
TEST_F(GstRecordXExtTest, SetFramerateRoundTrip)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setFramerate(5));
    EXPECT_NO_FATAL_FAILURE(m_gst->setFramerate(60));
    EXPECT_NO_FATAL_FAILURE(m_gst->setFramerate(0));
}

TEST_F(GstRecordXExtTest, SetRecordAreaRoundTrip)
{
    const QRect r(10, 20, 800, 600);
    EXPECT_NO_FATAL_FAILURE(m_gst->setRecordArea(r));
    // Empty / zero rect is a legal degenerate value.
    EXPECT_NO_FATAL_FAILURE(m_gst->setRecordArea(QRect(0, 0, 0, 0)));
}

TEST_F(GstRecordXExtTest, SetInputDeviceNameAcceptsEmptyAndNonEmpty)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setInputDeviceName(QString()));
    EXPECT_NO_FATAL_FAILURE(m_gst->setInputDeviceName(QString("alsa_input.pci-0000_00_1b.0.analog-stereo")));
}

TEST_F(GstRecordXExtTest, SetOutputDeviceNameAcceptsEmptyAndNonEmpty)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setOutputDeviceName(QString()));
    EXPECT_NO_FATAL_FAILURE(m_gst->setOutputDeviceName(QString("alsa_output.pci-0000_00_1b.0.analog-stereo")));
}

TEST_F(GstRecordXExtTest, SetAudioTypeCoversAllEnumValues)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setAudioType(GstRecordX::AudioType::None));
    EXPECT_NO_FATAL_FAILURE(m_gst->setAudioType(GstRecordX::AudioType::Mic));
    EXPECT_NO_FATAL_FAILURE(m_gst->setAudioType(GstRecordX::AudioType::Sys));
    EXPECT_NO_FATAL_FAILURE(m_gst->setAudioType(GstRecordX::AudioType::Mix));
}

TEST_F(GstRecordXExtTest, SetVidoeTypeCoversAllEnumValues)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setVidoeType(GstRecordX::VideoType::webm));
    EXPECT_NO_FATAL_FAILURE(m_gst->setVidoeType(GstRecordX::VideoType::ogg));
}

TEST_F(GstRecordXExtTest, SetSavePathRoundTrip)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setSavePath(QString("/tmp/ut_gstrecordx_out.webm")));
    EXPECT_NO_FATAL_FAILURE(m_gst->setSavePath(QString()));
}

TEST_F(GstRecordXExtTest, SetX11RecordMouseBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setX11RecordMouse(true));
    EXPECT_NO_FATAL_FAILURE(m_gst->setX11RecordMouse(false));
}

TEST_F(GstRecordXExtTest, SetBoardVendorTypeBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(0)); // non-hw
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(1)); // hw board
}

// getAudioPipeline is pure string assembly (no GStreamer calls).
// Empty device name -> empty result with a critical log.
TEST_F(GstRecordXExtTest, GetAudioPipelineEmptyDeviceReturnsEmpty)
{
    QString out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::GstRecordXgetAudioPipeline(
                                *m_gst, QString(), QString("mic"), QString("mux")));
    EXPECT_TRUE(out.isEmpty());
}

// Non-empty device + "mux" target -> vorbisenc branch terminates with "mux.".
TEST_F(GstRecordXExtTest, GetAudioPipelineMuxContainsVorbisenc)
{
    m_gst->setInputDeviceName(QString("ut_src_dev"));
    QString out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::GstRecordXgetAudioPipeline(
                                *m_gst, QString("ut_src_dev"), QString("mic"), QString("mux")));
    EXPECT_FALSE(out.isEmpty());
    EXPECT_TRUE(out.contains("pulsesrc device=ut_src_dev"));
    EXPECT_TRUE(out.contains("vorbisenc"));
    EXPECT_TRUE(out.endsWith("mux."));
}

// Non-empty device + "mix" target -> adder branch terminates with "mix.".
TEST_F(GstRecordXExtTest, GetAudioPipelineMixContainsMixTerminator)
{
    QString out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::GstRecordXgetAudioPipeline(
                                *m_gst, QString("ut_sys_dev"), QString("sys"), QString("mix")));
    EXPECT_FALSE(out.isEmpty());
    EXPECT_TRUE(out.contains("pulsesrc device=ut_sys_dev"));
    EXPECT_TRUE(out.endsWith("mix."));
    // mix branch must NOT include vorbisenc (that is only on the mux branch).
    EXPECT_FALSE(out.contains("vorbisenc"));
}

// Unknown target arg -> helper logs an error and returns empty string.
TEST_F(GstRecordXExtTest, GetAudioPipelineUnknownArgReturnsEmpty)
{
    QString out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::GstRecordXgetAudioPipeline(
                                *m_gst, QString("ut_dev"), QString("mic"), QString("bogus")));
    EXPECT_TRUE(out.isEmpty());
}

// getAudioPipeline embeds the default sample rate (44100) and channel count
// (2). Verify those defaults survive untouched.
TEST_F(GstRecordXExtTest, GetAudioPipelineEmbedsDefaultRateAndChannels)
{
    QString out = call_private_fun::GstRecordXgetAudioPipeline(
        *m_gst, QString("ut_dev"), QString("mic"), QString("mux"));
    ASSERT_FALSE(out.isEmpty());
    EXPECT_TRUE(out.contains("rate=44100"));
    EXPECT_TRUE(out.contains("channels=2"));
}

// pipelineStructuredOutput only mutates a local QString and logs; safe.
TEST_F(GstRecordXExtTest, PipelineStructuredOutputDoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXpipelineStructuredOutput(
        *m_gst, QString("ximagesrc display-name=:0 ! videoconvert ! queue ! mux.")));
    // Empty / minimal inputs are legal.
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXpipelineStructuredOutput(
        *m_gst, QString()));
}

// Calling every public setter in sequence must not crash and must leave the
// object destructible (dtor path with m_pipeline == nullptr).
TEST_F(GstRecordXExtTest, FullSetterChainIsCrashFree)
{
    EXPECT_NO_FATAL_FAILURE({
        m_gst->setInputDeviceName("ut_in");
        m_gst->setOutputDeviceName("ut_out");
        m_gst->setAudioType(GstRecordX::AudioType::Mix);
        m_gst->setVidoeType(GstRecordX::VideoType::ogg);
        m_gst->setFramerate(30);
        m_gst->setRecordArea(QRect(0, 0, 1280, 720));
        m_gst->setSavePath("/tmp/ut_gstrecordx_chain.ogg");
        m_gst->setX11RecordMouse(true);
        m_gst->setBoardVendorType(1);
    });
    EXPECT_NO_FATAL_FAILURE(m_gst->getGloop());
}
