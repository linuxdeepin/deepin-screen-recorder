// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryFile>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/record_process.h"
#include "../../src/utils.h"
#include "../../src/utils/configsettings.h"

using namespace testing;

// Covers the SAFE remainder of RecordProcess that ut_record_process.h does not
// reach without spawning ffmpeg: ctor/dtor, setRecordInfo, setFullScreenRecord,
// setRecordSavingNotifyId, getScreenRecordSavePath (all branches), save2Clipboard,
// and initProcess (path-string construction only; no subprocess spawn).
//
// DELIBERATELY SKIPPED (would spawn ffmpeg/pipe or terminate the test process):
//   - startRecord / recordVideo / waylandRecord / treelandRecord / GstStartRecord
//     (launch external processes).
//   - stopRecord / GstStopRecord (dereferences m_recorderProcess / m_gstRecordX).
//   - onRecordFinish / onTranscodeFinish / onExitGstRecord (call exitRecord).
//   - exitRecord (QApplication::quit() + _Exit(0) via QTimer -> kills the harness).
//   - emitRecording (infinite while-loop).
//   - onStartTranscode / onTranscodePaletteFinished (spawn ffmpeg).

ACCESS_PRIVATE_FUN(RecordProcess, void(), initProcess);
ACCESS_PRIVATE_FIELD(RecordProcess, QString, savePath);
ACCESS_PRIVATE_FIELD(RecordProcess, QString, saveBaseName);
ACCESS_PRIVATE_FIELD(RecordProcess, QString, saveDir);
ACCESS_PRIVATE_FIELD(RecordProcess, QString, saveTempDir);
ACCESS_PRIVATE_FIELD(RecordProcess, QString, saveAreaName);
ACCESS_PRIVATE_FIELD(RecordProcess, int, m_recordType);
ACCESS_PRIVATE_FIELD(RecordProcess, bool, m_isFullScreenRecord);

class RecordProcessCovTest : public Test
{
public:
    RecordProcess *m_p = nullptr;
    Stub stub;
    void SetUp() override { m_p = new RecordProcess; }
    void TearDown() override
    {
        delete m_p;
        // restore defaults so later suites are unaffected
        Utils::isFFmpegEnv = true;
        Utils::isWaylandMode = false;
        Utils::isTreelandMode = false;
    }
};

TEST_F(RecordProcessCovTest, constructorSetsDefaults)
{
    // construct a second one to confirm stable ctor
    RecordProcess *p2 = nullptr;
    EXPECT_NO_FATAL_FAILURE(p2 = new RecordProcess);
    delete p2;
    SUCCEED();
}

TEST_F(RecordProcessCovTest, setRecordInfoReadsConfig)
{
    ConfigSettings::instance()->setValue("recorder", "format", static_cast<int>(Utils::kMKV));
    ConfigSettings::instance()->setValue("recorder", "audio", static_cast<int>(Utils::kMic));
    ConfigSettings::instance()->setValue("recorder", "cursor", 1);
    ConfigSettings::instance()->setValue("recorder", "frame_rate", 24);
    EXPECT_NO_FATAL_FAILURE(m_p->setRecordInfo(QRect(0, 0, 640, 480), QStringLiteral("area1")));
    SUCCEED();
}

TEST_F(RecordProcessCovTest, setFullScreenRecord)
{
    EXPECT_NO_FATAL_FAILURE(m_p->setFullScreenRecord(true));
    EXPECT_NO_FATAL_FAILURE(m_p->setFullScreenRecord(false));
    SUCCEED();
}

TEST_F(RecordProcessCovTest, setRecordSavingNotifyId)
{
    EXPECT_NO_FATAL_FAILURE(m_p->setRecordSavingNotifyId(42u));
    EXPECT_NO_FATAL_FAILURE(m_p->setRecordSavingNotifyId(RecordProcess::RECORD_SAVING_NOTIFY_ID_INVALID));
    SUCCEED();
}

// ---------- getScreenRecordSavePath branches ----------

TEST_F(RecordProcessCovTest, getScreenRecordSavePathEmptyDir)
{
    ConfigSettings::instance()->setValue("recorder", "save_dir", QString());
    EXPECT_NO_FATAL_FAILURE(m_p->getScreenRecordSavePath());
    SUCCEED();
}

TEST_F(RecordProcessCovTest, getScreenRecordSavePathSaveOp1)
{
    ConfigSettings::instance()->setValue("recorder", "save_dir", QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    ConfigSettings::instance()->setValue("recorder", "save_op", 1);
    EXPECT_NO_FATAL_FAILURE(m_p->getScreenRecordSavePath());
    SUCCEED();
}

TEST_F(RecordProcessCovTest, getScreenRecordSavePathSaveOpDefault)
{
    ConfigSettings::instance()->setValue("recorder", "save_dir", QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    ConfigSettings::instance()->setValue("recorder", "save_op", 3);
    EXPECT_NO_FATAL_FAILURE(m_p->getScreenRecordSavePath());
    SUCCEED();
}

// ---------- save2Clipboard ----------

TEST_F(RecordProcessCovTest, save2ClipboardPopulatesMime)
{
    // create a temp file so QUrl::fromLocalFile is meaningful
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("hello");
    tmp.close();
    EXPECT_NO_FATAL_FAILURE(m_p->save2Clipboard(tmp.fileName()));
    // verify the clipboard received urls
    const QMimeData *md = qApp->clipboard()->mimeData(QClipboard::Clipboard);
    EXPECT_NE(md, nullptr);
    SUCCEED();
}

// ---------- initProcess: path construction only, no spawn ----------

TEST_F(RecordProcessCovTest, initProcessEmptyAreaNameMp4)
{
    access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    access_private_field::RecordProcesssaveAreaName(*m_p) = QString();
    access_private_field::RecordProcessm_isFullScreenRecord(*m_p) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessinitProcess(*m_p));
    QString base = access_private_field::RecordProcesssaveBaseName(*m_p);
    EXPECT_TRUE(base.endsWith(QStringLiteral(".mp4")));
}

TEST_F(RecordProcessCovTest, initProcessEmptyAreaNameMkv)
{
    access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMKV);
    access_private_field::RecordProcesssaveAreaName(*m_p) = QString();
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessinitProcess(*m_p));
    QString base = access_private_field::RecordProcesssaveBaseName(*m_p);
    EXPECT_TRUE(base.endsWith(QStringLiteral(".mkv")));
}

TEST_F(RecordProcessCovTest, initProcessFullScreenWithName)
{
    access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    access_private_field::RecordProcesssaveAreaName(*m_p) = QStringLiteral("myclip");
    access_private_field::RecordProcessm_isFullScreenRecord(*m_p) = true;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessinitProcess(*m_p));
    QString base = access_private_field::RecordProcesssaveBaseName(*m_p);
    EXPECT_TRUE(base.startsWith(QStringLiteral("myclip")));
}

TEST_F(RecordProcessCovTest, initProcessRegionWithName)
{
    access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    access_private_field::RecordProcesssaveAreaName(*m_p) = QStringLiteral("region1");
    access_private_field::RecordProcessm_isFullScreenRecord(*m_p) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessinitProcess(*m_p));
    QString base = access_private_field::RecordProcesssaveBaseName(*m_p);
    EXPECT_TRUE(base.contains(QStringLiteral("region1")));
}
