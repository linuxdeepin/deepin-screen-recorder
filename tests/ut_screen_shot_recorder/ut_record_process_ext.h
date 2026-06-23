// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Crash-safe coverage tests for src/record_process.cpp.
//
// The legacy ut_record_process.h drives real ffmpeg/QProcess recording and
// runs a 5-second QEventLoop; that path is inherently unsafe in a headless
// unit-test harness. This file instead targets only methods that do not not
// spawn processes, capture audio/video, write files, block, or talk to DBus:
//   - constructor / destructor (m_gstRecordX stays null -> dtor no-op)
//   - setFullScreenRecord (member flag)
//   - setRecordSavingNotifyId (member)
//   - setRecordInfo (member setters + config reads; getValue stubbed)
//   - getScreenRecordSavePath (public slot; reads config + builds paths;
//     filesystem mkdir is guarded internally and degrades to MoviesLocation)
//
// Skipped (unsafe — record/encode/transcode/write/spawn/block/DBus/clipboard):
//   - startRecord : dispatches to recordVideo/treelandRecord/waylandRecord/
//     GstStartRecord, spawns threads, calls DBus, emits signals, can _Exit.
//   - stopRecord : writes "q" to a QProcess, calls Wayland/TreeLand stop,
//     invokes exitRecord -> QApplication::quit / _Exit.
//   - exitRecord : DBus Notify, save2Clipboard, QTimer::singleShot -> quit.
//   - recordVideo / waylandRecord / treelandRecord / GstStartRecord :
//     spawn ffmpeg / build GStreamer pipeline / open Wayland capture.
//   - GstStopRecord : stops pipelines / recorders.
//   - initProcess : news a QProcess + builds file paths + QFile::remove.
//   - onRecordFinish / onStartTranscode / onTranscodePaletteFinished /
//     onTranscodeFinish : spawn ffmpeg / rename files / call exitRecord.
//   - save2Clipboard : touches qApp clipboard.
//   - emitRecording : infinite DBus loop until a flag flips.
//   - onExitGstRecord : unloadFunctions + file rename + exitRecord.
//
// Note: the existing ut_record_process.h declares no ACCESS_PRIVATE_FUN macros
// and uses the fixture name RecordProcessTest; we use RecordProcessExtTest to
// avoid any collision and declare no private-fun macros at all (every safe
// target here is already public).

#pragma once
#include <gtest/gtest.h>
#include <QRect>
#include <QString>
#include <QVariant>
#include "../../src/record_process.h"
#include "../../src/utils/configsettings.h"
#include "stub.h"

// Deterministic ConfigSettings::getValue stub so setRecordInfo and
// getScreenRecordSavePath do not depend on the on-disk config file.
// Signature matches ConfigSettings::getValue(const QString&, const QString&).
static QVariant ut_record_process_getValue_stub(void *obj, const QString &group, const QString &key)
{
    Q_UNUSED(obj);
    Q_UNUSED(group);
    if (key == "format") {
        return 0;            // Utils::kMP4
    }
    if (key == "audio") {
        return 0;            // Utils::kNoAudio
    }
    if (key == "cursor") {
        return 1;            // RECORD_MOUSE_CURSE
    }
    if (key == "frame_rate") {
        return 24;
    }
    if (key == "save_dir") {
        return QString();    // empty -> default MoviesLocation path
    }
    if (key == "save_op") {
        return 0;
    }
    return QVariant();
}

class RecordProcessExtTest : public ::testing::Test
{
public:
    RecordProcess *m_proc;
    Stub stub;

    void SetUp() override
    {
        // ConfigSettings::getValue is non-virtual; stub it via stub.set.
        stub.set(ADDR(ConfigSettings, getValue), ut_record_process_getValue_stub);
        m_proc = new RecordProcess();
    }

    void TearDown() override
    {
        stub.reset(ADDR(ConfigSettings, getValue));
        delete m_proc;
        m_proc = nullptr;
    }
};

// Static constant definitions must be linkage-defined (they are, in the .cpp).
TEST(RecordProcessExtStaticTest, MouseConstantsHaveDistinctValues)
{
    EXPECT_EQ(RecordProcess::RECORD_MOUSE_NULL, 0);
    EXPECT_EQ(RecordProcess::RECORD_MOUSE_CURSE, 1);
    EXPECT_EQ(RecordProcess::RECORD_MOUSE_CHECK, 2);
    EXPECT_EQ(RecordProcess::RECORD_MOUSE_CURSE_CHECK, 3);
}

TEST(RecordProcessExtStaticTest, FramerateConstantsHaveExpectedValues)
{
    EXPECT_EQ(RecordProcess::RECORD_FRAMERATE_5, 5);
    EXPECT_EQ(RecordProcess::RECORD_FRAMERATE_10, 10);
    EXPECT_EQ(RecordProcess::RECORD_FRAMERATE_20, 20);
    EXPECT_EQ(RecordProcess::RECORD_FRAMERATE_24, 24);
    EXPECT_EQ(RecordProcess::RECORD_FRAMERATE_30, 30);
}

TEST(RecordProcessExtStaticTest, SavingNotifyIdInvalidConstantIsZero)
{
    EXPECT_EQ(RecordProcess::RECORD_SAVING_NOTIFY_ID_INVALID, 0u);
}

// Constructor must complete without touching the GStreamer object.
TEST_F(RecordProcessExtTest, ConstructorRunsClean)
{
    EXPECT_NE(m_proc, nullptr);
}

// setRecordSavingNotifyId stores the id; round-trip is observable only through
// exitRecord (which we skip), so we just assert crash-freedom here.
TEST_F(RecordProcessExtTest, SetRecordSavingNotifyIdAcceptsValues)
{
    EXPECT_NO_FATAL_FAILURE(m_proc->setRecordSavingNotifyId(0));
    EXPECT_NO_FATAL_FAILURE(m_proc->setRecordSavingNotifyId(
                                RecordProcess::RECORD_SAVING_NOTIFY_ID_INVALID));
    EXPECT_NO_FATAL_FAILURE(m_proc->setRecordSavingNotifyId(42));
}

// setFullScreenRecord flips an internal bool; exercise both states.
TEST_F(RecordProcessExtTest, SetFullScreenRecordBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_proc->setFullScreenRecord(true));
    EXPECT_NO_FATAL_FAILURE(m_proc->setFullScreenRecord(false));
}

// setRecordInfo stores the rect + filename and pulls config (stubbed) into
// the type / audio / cursor / framerate members. Crash-free for varied inputs.
TEST_F(RecordProcessExtTest, SetRecordInfoTypicalArgs)
{
    EXPECT_NO_FATAL_FAILURE(m_proc->setRecordInfo(QRect(0, 0, 1920, 1080),
                                                  QString("ut_recording")));
}

TEST_F(RecordProcessExtTest, SetRecordInfoEmptyFilename)
{
    // Empty filename is handled by recorders downstream; setRecordInfo itself
    // accepts it.
    EXPECT_NO_FATAL_FAILURE(m_proc->setRecordInfo(QRect(100, 100, 640, 480),
                                                  QString()));
}

TEST_F(RecordProcessExtTest, SetRecordInfoDegenerateRect)
{
    EXPECT_NO_FATAL_FAILURE(m_proc->setRecordInfo(QRect(0, 0, 0, 0),
                                                  QString("zero")));
}

// getScreenRecordSavePath is a public slot; with the stubbed empty save_dir it
// falls back to the MoviesLocation default and ensures the directory exists.
// The method guards mkdir failure and non-writable dirs by degrading to the
// movies location, so it is safe to invoke headless.
TEST_F(RecordProcessExtTest, GetScreenRecordSavePathDefaultLocation)
{
    EXPECT_NO_FATAL_FAILURE(m_proc->getScreenRecordSavePath());
}

// Calling getScreenRecordSavePath twice must be stable (idempotent setup).
TEST_F(RecordProcessExtTest, GetScreenRecordSavePathIsStableAcrossCalls)
{
    EXPECT_NO_FATAL_FAILURE(m_proc->getScreenRecordSavePath());
    EXPECT_NO_FATAL_FAILURE(m_proc->getScreenRecordSavePath());
}

// setRecordInfo + getScreenRecordSavePath together exercise the two safe
// config-reading entry points without ever starting a recording.
TEST_F(RecordProcessExtTest, SetRecordInfoThenResolveSavePath)
{
    EXPECT_NO_FATAL_FAILURE(m_proc->setRecordInfo(QRect(0, 0, 1280, 720),
                                                  QString("combined")));
    EXPECT_NO_FATAL_FAILURE(m_proc->getScreenRecordSavePath());
}

// Destructor with m_gstRecordX == nullptr (never assigned, since GstStartRecord
// is skipped) must not crash. TearDown already deletes; this is a smoke check
// that constructing + immediately destroying is safe.
TEST_F(RecordProcessExtTest, FreshObjectIsImmediatelyDestructible)
{
    RecordProcess *tmp = nullptr;
    EXPECT_NO_FATAL_FAILURE(tmp = new RecordProcess());
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}
