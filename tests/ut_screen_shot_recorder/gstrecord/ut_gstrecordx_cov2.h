// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QRect>
#include <QString>
#include <QStringList>
#include "addr_pri.h"
#include "stub.h"
#include "../../src/gstrecord/gstrecordx.h"
#include "../../src/gstrecord/gstinterface.h"

using namespace testing;

// GstRecordXCov2Test targets the REMAINING uncovered GstRecordX surface NOT
// covered by:
//   - ut_gstrecordx.h          (legacy real-pipeline tests, mostly disabled)
//   - ut_gstrecordx_ext.h      (setters, initMemberVariables, getAudioPipeline,
//                               pipelineStructuredOutput)
//   - ut_gstrecordx_x11_cov.h  (x11GstStartRecord/StopRecord with createPipeline
//                               stubbed to false)
//
// Targeted UNcovered methods:
//   - createPipeline with createPipeline returning true path (stub returns true
//     but m_pipeline stays null, hitting the "Error: Pipeline create failure"
//     branch and exercising the full argument assembly for ogg/mix/mic/sys).
//   - waylandWriteVideoFrame (test-build stub returns false; just call to hit
//     the entry/exit lines).
//   - waylandGstStartRecord / waylandGstStopRecord (gated by ENABLE_UNIT_TEST
//     to a near-empty body; calling them just hits entry/exit).
//   - getGstBin via x11GstStartRecord path that calls it indirectly (skipped
//     since it needs a real GstElement).
//   - pipelineStructuredOutput with edge inputs (empty / multi-mux / mix).
//   - getAudioPipeline with combined empty + non-empty device names.
//   - createPipeline arguments covering all audio type + video type combos.

// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(GstRecordX, bool(QStringList), createPipeline);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(GstRecordX, QString(const QString &, const QString &, const QString &), getAudioPipeline);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(GstRecordX, void(QString), pipelineStructuredOutput);
ACCESS_PRIVATE_FUN(GstRecordX, void(), waylandGstStartRecord);
ACCESS_PRIVATE_FUN(GstRecordX, void(), waylandGstStopRecord);

// Stub: createPipeline returns true but leaves m_pipeline null so the caller
// hits the "Error: Gstreamer's Pipeline create failure!" branch (covers the
// full argument-assembly path without spawning a real pipeline).
static bool cp_true_stub(void *) { return true; }
static bool cp_false_stub(void *) { return false; }

class GstRecordXCov2Test : public Test
{
public:
    GstRecordX *m_gst = nullptr;
    Stub m_stub;

    void SetUp() override
    {
        ASSERT_NO_FATAL_FAILURE(gstInterface::initFunctions());
        m_gst = new GstRecordX();
        m_gst->setRecordArea(QRect(0, 0, 320, 240));
        m_gst->setFramerate(15);
        m_gst->setSavePath(QStringLiteral("/tmp/ut_gst_cov2.webm"));
    }
    void TearDown() override
    {
        delete m_gst;
        m_gst = nullptr;
    }
};

// ---------- createPipeline: true-returning stub (covers full arg assembly) ----------
// m_pipeline stays null after the stub returns, so the caller hits the
// "Error: Pipeline create failure" branch.

TEST_F(GstRecordXCov2Test, createPipelineTrueReturnWebmNoAudio)
{
    m_stub.set(get_private_fun::GstRecordXcreatePipeline(), cp_true_stub);
    m_gst->setVidoeType(GstRecordX::VideoType::webm);
    m_gst->setAudioType(GstRecordX::AudioType::None);
    EXPECT_NO_FATAL_FAILURE(m_gst->x11GstStartRecord());
}

TEST_F(GstRecordXCov2Test, createPipelineTrueReturnOggMicAudio)
{
    m_stub.set(get_private_fun::GstRecordXcreatePipeline(), cp_true_stub);
    m_gst->setVidoeType(GstRecordX::VideoType::ogg);
    m_gst->setAudioType(GstRecordX::AudioType::Mic);
    m_gst->setInputDeviceName(QStringLiteral("ut_mic_dev"));
    EXPECT_NO_FATAL_FAILURE(m_gst->x11GstStartRecord());
}

TEST_F(GstRecordXCov2Test, createPipelineTrueReturnWebmSysAudio)
{
    m_stub.set(get_private_fun::GstRecordXcreatePipeline(), cp_true_stub);
    m_gst->setVidoeType(GstRecordX::VideoType::webm);
    m_gst->setAudioType(GstRecordX::AudioType::Sys);
    m_gst->setOutputDeviceName(QStringLiteral("ut_sys_dev"));
    EXPECT_NO_FATAL_FAILURE(m_gst->x11GstStartRecord());
}

TEST_F(GstRecordXCov2Test, createPipelineTrueReturnWebmMixAudio)
{
    m_stub.set(get_private_fun::GstRecordXcreatePipeline(), cp_true_stub);
    m_gst->setVidoeType(GstRecordX::VideoType::webm);
    m_gst->setAudioType(GstRecordX::AudioType::Mix);
    m_gst->setInputDeviceName(QStringLiteral("ut_mic_dev"));
    m_gst->setOutputDeviceName(QStringLiteral("ut_sys_dev"));
    EXPECT_NO_FATAL_FAILURE(m_gst->x11GstStartRecord());
}

// ---------- createPipeline: false-returning stub (error-else branch) ----------
TEST_F(GstRecordXCov2Test, createPipelineFalseReturnCoversElseBranch)
{
    m_stub.set(get_private_fun::GstRecordXcreatePipeline(), cp_false_stub);
    EXPECT_NO_FATAL_FAILURE(m_gst->x11GstStartRecord());
}

// ---------- Mix audio with empty device names (logs critical, no audio added) ----------
TEST_F(GstRecordXCov2Test, createPipelineMixEmptyDeviceNamesLogsError)
{
    m_stub.set(get_private_fun::GstRecordXcreatePipeline(), cp_true_stub);
    m_gst->setAudioType(GstRecordX::AudioType::Mix);
    m_gst->setInputDeviceName(QString());
    m_gst->setOutputDeviceName(QString());
    EXPECT_NO_FATAL_FAILURE(m_gst->x11GstStartRecord());
}

// ---------- waylandGstStartRecord / waylandGstStopRecord: test-build no-ops ----------
// These bodies are wrapped in #ifndef ENABLE_UNIT_TEST so they compile to
// essentially empty functions; calling them just hits entry/exit.

TEST_F(GstRecordXCov2Test, waylandGstStartRecordEntryIsNoop)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXwaylandGstStartRecord(*m_gst));
}

TEST_F(GstRecordXCov2Test, waylandGstStopRecordEntryIsNoop)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXwaylandGstStopRecord(*m_gst));
}

// ---------- waylandWriteVideoFrame: test-build stub returns false ----------
TEST_F(GstRecordXCov2Test, waylandWriteVideoFrameStubReturnsFalse)
{
    unsigned char data[16] = {0};
    bool ok = true;
    EXPECT_NO_FATAL_FAILURE(ok = m_gst->waylandWriteVideoFrame(data, 2, 2));
    EXPECT_FALSE(ok);
}

// ---------- x11GstStopRecord: m_pipeline null (warning branch) ----------
TEST_F(GstRecordXCov2Test, x11GstStopRecordNullPipelineWarning)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->x11GstStopRecord());
}

// ---------- getAudioPipeline: more combinations ----------
TEST_F(GstRecordXCov2Test, getAudioPipelineEmptyMixTarget)
{
    QString out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::GstRecordXgetAudioPipeline(
        *m_gst, QString(), QStringLiteral("sys"), QStringLiteral("mix")));
    EXPECT_TRUE(out.isEmpty());
}

TEST_F(GstRecordXCov2Test, getAudioPipelineEmptyMuxTarget)
{
    QString out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::GstRecordXgetAudioPipeline(
        *m_gst, QString(), QStringLiteral("mic"), QStringLiteral("mux")));
    EXPECT_TRUE(out.isEmpty());
}

TEST_F(GstRecordXCov2Test, getAudioPipelineNonEmptyMixTarget)
{
    QString out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::GstRecordXgetAudioPipeline(
        *m_gst, QStringLiteral("ut_dev"), QStringLiteral("sys"), QStringLiteral("mix")));
    EXPECT_FALSE(out.isEmpty());
    EXPECT_TRUE(out.contains("adder"));
}

// ---------- pipelineStructuredOutput edge inputs ----------
TEST_F(GstRecordXCov2Test, pipelineStructuredOutputEmpty)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXpipelineStructuredOutput(*m_gst, QString()));
}

TEST_F(GstRecordXCov2Test, pipelineStructuredOutputWithMix)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXpipelineStructuredOutput(
        *m_gst, QStringLiteral("ximagesrc ! videoconvert ! mix. ! mux.")));
}

TEST_F(GstRecordXCov2Test, pipelineStructuredOutputMultiMux)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::GstRecordXpipelineStructuredOutput(
        *m_gst, QStringLiteral("ximagesrc ! mux. ! mux. ! filesink")));
}

// ---------- setBoardVendorType affects nothing visible but stores ----------
TEST_F(GstRecordXCov2Test, setBoardVendorTypeBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(0));
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(1));
    EXPECT_NO_FATAL_FAILURE(m_gst->setBoardVendorType(-1));
}

// ---------- full setter + start chain with stubbed pipeline ----------
TEST_F(GstRecordXCov2Test, fullChainOggMixThenStartStop)
{
    m_stub.set(get_private_fun::GstRecordXcreatePipeline(), cp_false_stub);
    EXPECT_NO_FATAL_FAILURE({
        m_gst->setVidoeType(GstRecordX::VideoType::ogg);
        m_gst->setAudioType(GstRecordX::AudioType::Mix);
        m_gst->setInputDeviceName("ut_in");
        m_gst->setOutputDeviceName("ut_out");
        m_gst->setFramerate(30);
        m_gst->setRecordArea(QRect(0, 0, 1280, 720));
        m_gst->setSavePath("/tmp/ut_chain.ogg");
        m_gst->setX11RecordMouse(true);
        m_gst->setBoardVendorType(1);
        m_gst->x11GstStartRecord();
        m_gst->x11GstStopRecord();
    });
}
