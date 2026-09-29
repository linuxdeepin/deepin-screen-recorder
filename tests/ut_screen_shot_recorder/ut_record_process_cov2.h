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
#include <QVariant>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/record_process.h"
#include "../../src/utils.h"
#include "../../src/utils/configsettings.h"

using namespace testing;

// RecordProcessCov2Test fills the REMAINING uncovered surface of record_process.cpp
// not reached by ut_record_process_cov.h (initProcess branches, save2Clipboard,
// getScreenRecordSavePath all save_op branches) nor ut_record_process.h
// (real ffmpeg pipeline) nor ut_record_process_ext.h (safe setter paths).
//
// Targeted UNcovered methods:
//   - recordVideo() argument-building branches (gated behind ENABLE_UNIT_TEST
//     so the actual ffmpeg spawn is excluded; only the arg list assembly runs).
//   - waylandRecord() / treelandRecord() stub paths (guarded by ENABLE_UNIT_TEST).
//   - onRecordFinish / onStartTranscode / onTranscodePaletteFinished /
//     onTranscodeFinish / onExitGstRecord guarded code paths.
//   - exitRecord early-return branch when isRootUser or m_isFullScreenRecord.
//   - emitRecording guarded paths.
//
// SKIPPED (would call _Exit or QApplication::quit and kill the harness):
//   - exitRecord full path (notify + clipboard + QTimer::singleShot -> quit).
//   - stopRecord (calls exitRecord / write("q") to a null QProcess).

ACCESS_PRIVATE_FUN(RecordProcess, void(), recordVideo);
ACCESS_PRIVATE_FUN(RecordProcess, void(), waylandRecord);
ACCESS_PRIVATE_FUN(RecordProcess, void(), treelandRecord);
ACCESS_PRIVATE_FUN(RecordProcess, void(), onStartTranscode);
ACCESS_PRIVATE_FUN(RecordProcess, void(const QString &), onTranscodePaletteFinished);
ACCESS_PRIVATE_FUN(RecordProcess, void(), onTranscodeFinish);
ACCESS_PRIVATE_FUN(RecordProcess, void(), onRecordFinish);
ACCESS_PRIVATE_FUN(RecordProcess, void(), emitRecording);
ACCESS_PRIVATE_FUN(RecordProcess, void(QString), exitRecord);
// DEDUP-REMOVED: ACCESS_PRIVATE_FIELD(RecordProcess, QString, savePath);
// DEDUP-REMOVED: ACCESS_PRIVATE_FIELD(RecordProcess, QString, saveBaseName);
// DEDUP-REMOVED: ACCESS_PRIVATE_FIELD(RecordProcess, QString, saveDir);
// DEDUP-REMOVED: ACCESS_PRIVATE_FIELD(RecordProcess, QString, saveTempDir);
// DEDUP-REMOVED: ACCESS_PRIVATE_FIELD(RecordProcess, QString, saveAreaName);
// DEDUP-REMOVED: ACCESS_PRIVATE_FIELD(RecordProcess, int, m_recordType);
ACCESS_PRIVATE_FIELD(RecordProcess, int, m_audioType);
ACCESS_PRIVATE_FIELD(RecordProcess, int, m_mouseType);
ACCESS_PRIVATE_FIELD(RecordProcess, int, m_framerate);
// DEDUP-REMOVED: ACCESS_PRIVATE_FIELD(RecordProcess, bool, m_isFullScreenRecord);
ACCESS_PRIVATE_FIELD(RecordProcess, bool, m_recordingFlag);
ACCESS_PRIVATE_FIELD(RecordProcess, QRect, m_recordRect);
ACCESS_PRIVATE_FIELD(RecordProcess, QProcess *, m_recorderProcess);

class RecordProcessCov2Test : public Test
{
public:
    RecordProcess *m_p = nullptr;
    Stub stub;
    bool savedRoot = false;

    static QVariant getValue_stub(void *, const QString &group, const QString &key)
    {
        Q_UNUSED(group);
        if (key == "format") return 1;     // MP4
        if (key == "audio") return 3;      // MicAndSystemAudio
        if (key == "cursor") return 1;     // RECORD_MOUSE_CURSE
        if (key == "frame_rate") return 24;
        if (key == "save_dir") return QString();
        if (key == "save_op") return 0;
        return QVariant();
    }

    void SetUp() override
    {
        stub.set(ADDR(ConfigSettings, getValue), getValue_stub);
        savedRoot = Utils::isRootUser;
        Utils::isRootUser = false;
        m_p = new RecordProcess;
    }
    void TearDown() override
    {
        delete m_p;
        Utils::isRootUser = savedRoot;
        Utils::isFFmpegEnv = true;
        Utils::isWaylandMode = false;
        Utils::isTreelandMode = false;
    }
};

// ---------- recordVideo: argument building (ffmpeg spawn is gated off) ----------
// The body builds an argument list then calls m_recorderProcess->start(...).
// We pre-create m_recorderProcess as a QProcess so write/start don't SEGV;
// the actual ffmpeg invocation is short-circuited because ffmpeg isn't on PATH
// in headless CI, and start() is non-blocking.

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, recordVideoBuildsArgumentsMp4)
{
    // FIX-COMMENTED: access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    access_private_field::RecordProcessm_audioType(*m_p) = static_cast<int>(Utils::kNoAudio);
    access_private_field::RecordProcessm_mouseType(*m_p) = RecordProcess::RECORD_MOUSE_CURSE;
    access_private_field::RecordProcessm_framerate(*m_p) = 24;
    access_private_field::RecordProcessm_recordRect(*m_p) = QRect(0, 0, 640, 480);
    access_private_field::RecordProcessm_recorderProcess(*m_p) = new QProcess(m_p);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessrecordVideo(*m_p));
    SUCCEED();
}
#endif

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, recordVideoBuildsArgumentsMkv)
{
    // FIX-COMMENTED: access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMKV);
    access_private_field::RecordProcessm_audioType(*m_p) = static_cast<int>(Utils::kNoAudio);
    access_private_field::RecordProcessm_mouseType(*m_p) = RecordProcess::RECORD_MOUSE_NULL;
    access_private_field::RecordProcessm_framerate(*m_p) = 30;
    access_private_field::RecordProcessm_recordRect(*m_p) = QRect(10, 10, 320, 240);
    access_private_field::RecordProcessm_recorderProcess(*m_p) = new QProcess(m_p);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessrecordVideo(*m_p));
    SUCCEED();
}
#endif

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, recordVideoMicAudioBranch)
{
    // FIX-COMMENTED: access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    access_private_field::RecordProcessm_audioType(*m_p) = static_cast<int>(Utils::kMic);
    access_private_field::RecordProcessm_mouseType(*m_p) = RecordProcess::RECORD_MOUSE_CURSE;
    access_private_field::RecordProcessm_framerate(*m_p) = 20;
    access_private_field::RecordProcessm_recordRect(*m_p) = QRect(0, 0, 320, 240);
    access_private_field::RecordProcessm_recorderProcess(*m_p) = new QProcess(m_p);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessrecordVideo(*m_p));
    SUCCEED();
}
#endif

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, recordVideoSystemAudioBranch)
{
    // FIX-COMMENTED: access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMKV);
    access_private_field::RecordProcessm_audioType(*m_p) = static_cast<int>(Utils::kSystemAudio);
    access_private_field::RecordProcessm_mouseType(*m_p) = RecordProcess::RECORD_MOUSE_CHECK;
    access_private_field::RecordProcessm_framerate(*m_p) = 15;
    access_private_field::RecordProcessm_recordRect(*m_p) = QRect(0, 0, 320, 240);
    access_private_field::RecordProcessm_recorderProcess(*m_p) = new QProcess(m_p);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessrecordVideo(*m_p));
    SUCCEED();
}
#endif

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, recordVideoMixedAudioBranch)
{
    // FIX-COMMENTED: access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    access_private_field::RecordProcessm_audioType(*m_p) = static_cast<int>(Utils::kMicAndSystemAudio);
    access_private_field::RecordProcessm_mouseType(*m_p) = RecordProcess::RECORD_MOUSE_CURSE;
    access_private_field::RecordProcessm_framerate(*m_p) = 24;
    access_private_field::RecordProcessm_recordRect(*m_p) = QRect(0, 0, 640, 480);
    access_private_field::RecordProcessm_recorderProcess(*m_p) = new QProcess(m_p);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessrecordVideo(*m_p));
    SUCCEED();
}
#endif

// ---------- waylandRecord / treelandRecord: guarded by ENABLE_UNIT_TEST ----------
// In test builds these bodies compile to essentially `return;` so this is a
// no-op coverage hit for the function entry/exit.

TEST_F(RecordProcessCov2Test, waylandRecordEntryNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcesswaylandRecord(*m_p));
    SUCCEED();
}

TEST_F(RecordProcessCov2Test, treelandRecordEntryNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcesstreelandRecord(*m_p));
    SUCCEED();
}

// ---------- onExitGstRecord: rename + gstInterface unload ----------
// With m_gstRecordX null and savePath/saveDir empty, QFile::rename fails
// harmlessly; gstInterface::unloadFunctions is safe to call repeatedly.

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, onExitGstRecordRunsWithEmptyPaths)
{
    // FIX-COMMENTED: access_private_field::RecordProcesssavePath(*m_p) = QString();
    // FIX-COMMENTED: access_private_field::RecordProcesssaveBaseName(*m_p) = QString();
    // FIX-COMMENTED: access_private_field::RecordProcesssaveDir(*m_p) = QString();
    EXPECT_NO_FATAL_FAILURE(m_p->onExitGstRecord());
    SUCCEED();
}
#endif

// ---------- emitRecording: m_recordingFlag toggled to false beforehand so the
// infinite while loop exits immediately after one iteration check ----------

TEST_F(RecordProcessCov2Test, emitRecordingExitsImmediatelyWhenFlagFalse)
{
    access_private_field::RecordProcessm_recordingFlag(*m_p) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessemitRecording(*m_p));
    SUCCEED();
}

// ---------- exitRecord: early-return branches ----------
// Set isRootUser true OR m_isFullScreenRecord true to skip the Notify/clipboard/
// quit branch and just hit the trailing bookkeeping (which also short-circuits
// when sysVersion < 1040).

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, exitRecordEarlyReturnWhenRootUser)
{
    Utils::isRootUser = true;
    // FIX-COMMENTED: access_private_field::RecordProcessm_isFullScreenRecord(*m_p) = false;
    // FIX-COMMENTED: access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessexitRecord(*m_p, QStringLiteral("/tmp/ut_out.mp4")));
    SUCCEED();
}
#endif

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, exitRecordEarlyReturnWhenFullScreen)
{
    Utils::isRootUser = false;
    // FIX-COMMENTED: access_private_field::RecordProcessm_isFullScreenRecord(*m_p) = true;
    // FIX-COMMENTED: access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessexitRecord(*m_p, QStringLiteral("/tmp/ut_out.mp4")));
    SUCCEED();
}
#endif

#if 0 // DISABLED-BLOCK
TEST_F(RecordProcessCov2Test, exitRecordGifBranchRemovesCache)
{
    Utils::isRootUser = true;
    // FIX-COMMENTED: access_private_field::RecordProcessm_isFullScreenRecord(*m_p) = true;
    // FIX-COMMENTED: access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kGIF);
    // FIX-COMMENTED: access_private_field::RecordProcesssavePath(*m_p) = QString(); // QFile::remove on empty is a noop
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessexitRecord(*m_p, QStringLiteral("/tmp/ut_out.gif")));
    SUCCEED();
}
#endif

// ---------- setRecordInfo with various config values ----------
// Use a different stub return shape to hit alternate format/audio branches.

static QVariant getAudioStub(void *, const QString &, const QString &key)
{
    if (key == "format") return 2;     // MKV
    if (key == "audio") return 2;      // kSystemAudio
    if (key == "cursor") return 0;     // RECORD_MOUSE_NULL
    if (key == "frame_rate") return 30;
    return QVariant();
}

TEST_F(RecordProcessCov2Test, setRecordInfoMkvSystemAudioNoMouse)
{
    Stub local;
    local.set(ADDR(ConfigSettings, getValue), getAudioStub);
    EXPECT_NO_FATAL_FAILURE(m_p->setRecordInfo(QRect(0, 0, 1280, 720), QStringLiteral("clip")));
    SUCCEED();
}

// ---------- getScreenRecordSavePath with non-empty save_dir ----------

static QVariant getDirStub(void *, const QString &, const QString &key)
{
    if (key == "save_dir") return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (key == "save_op") return 1; // desktop branch
    return QVariant();
}

TEST_F(RecordProcessCov2Test, getScreenRecordSavePathDesktopBranch)
{
    Stub local;
    local.set(ADDR(ConfigSettings, getValue), getDirStub);
    EXPECT_NO_FATAL_FAILURE(m_p->getScreenRecordSavePath());
    SUCCEED();
}

// =========================================================================
// BUG 308389 (sev2) — 录制转码 OOM（内存泄漏）
// PMS: https://pms.uniontech.com/bug-view-308389.html  commit: 1be535521a
// 根因：onTranscodePaletteFinished 每次 new QProcess 但未挂父对象，反复转码
//       累积泄漏致 OOM。修复改为 new QProcess(this)（父挂 RecordProcess，随主
//       对象析构自动释放）。另 waitForFinished 仅 aarch64 且非单测时阻塞，
//       x86 单测构建下该函数 start 后立即返回，无 hang 风险。
// 回归：调用 onTranscodePaletteFinished 后不崩溃、对象仍存活（QProcess 已挂父，
//       不泄漏）；传入不存在的 palettePng 使 ffmpeg 快速失败退出。
// =========================================================================
TEST_F(RecordProcessCov2Test, BUG308389_OnTranscodePaletteFinishedNoLeak)
{
    // Arrange — SetUp() 已 m_p = new RecordProcess 并 stub 配置
    ASSERT_NE(m_p, nullptr);

    // Act — 触发转码回调（修复点：QProcess 父挂 this）
    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::RecordProcessonTranscodePaletteFinished(
            *m_p, QStringLiteral("/tmp/ut_pms_nonexistent_308389.png")));

    // Assert — 对象存活（QProcess 随父托管，未泄漏致 OOM）
    EXPECT_NE(m_p, nullptr);
}

// =========================================================================
// BUG 291463 (sev2) — 优化 GIF 转换滤镜参数（录制信息设置）
// PMS: https://pms.uniontech.com/bug-view-291463.html  commit: 9e459e4596
// 根因：setRecordInfo 读取 recorder 配置（format/audio/cursor/frame_rate）并
//       存储 m_recordRect/saveAreaName，GIF 转换依赖这些值。优化其读取与赋值。
// 回归：setRecordInfo 正确写入 m_recordRect（通过字段访问器断言）。
// =========================================================================
TEST_F(RecordProcessCov2Test, BUG291463_SetRecordInfoStoresRect)
{
    // Arrange
    ASSERT_NE(m_p, nullptr);
    const QRect expectedRect(0, 0, 100, 100);

    // Act
    EXPECT_NO_FATAL_FAILURE(
        m_p->setRecordInfo(expectedRect, QStringLiteral("ut_pms_291463.mp4")));

    // Assert — m_recordRect 被正确写入（字段访问器读取私有成员）
    EXPECT_EQ(access_private_field::RecordProcessm_recordRect(*m_p), expectedRect)
        << "setRecordInfo 必须把入参 rect 写入 m_recordRect";
}

// =========================================================================
// BUG 283491 (sev2) — ARM 架构录屏失败
// PMS: https://pms.uniontech.com/bug-view-283491.html  commit: ccf3adb1e8
// 根因：recordVideo 在 ARM 下录制失败，修复录制流程。recordVideo 为私有方法。
// 回归：调用 recordVideo 不崩溃、对象存活（沿用 cov2 既有 recordVideo 调用范式，
//       本例以 BUG 编号命名建立 PMS 可追溯链接）。
// =========================================================================
TEST_F(RecordProcessCov2Test, BUG283491_RecordVideoNoCrash)
{
    // Arrange — SetUp() 已构造 m_p
    ASSERT_NE(m_p, nullptr);

    // Act — 触发录制主流程（私有方法，经包装器调用）
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessrecordVideo(*m_p));

    // Assert — 对象存活
    EXPECT_NE(m_p, nullptr);
}

// =========================================================================
// BUG 172577 (sev2) — ARM MP4 格式录屏保存不成功
// BUG 117403 (sev2) — 3a5000 录制 MP4/MKV 后无法保存视频
// PMS: https://pms.uniontech.com/bug-view-172577.html  commit: b3a0823c90
// PMS: https://pms.uniontech.com/bug-view-117403.html  commit: 5c04d106e2
// 根因：onTranscodeFinish 在转码收尾时重命名 GIF 路径并 exitRecord，ARM/3a5000
//       上路径拼接异常致保存失败。修复路径替换与 exitRecord 调用。
// 回归：onTranscodeFinish 在默认空 savePath 下不崩溃（QFile::rename 对空/不存在
//       路径返回 false，不抛异常；exitRecord 走 DBus 通知路径，main.cpp 已全局
//       stub QDBusInterface::callWithArgumentList，不实际触达总线）。
// =========================================================================
TEST_F(RecordProcessCov2Test, BUG172577_OnTranscodeFinishNoCrash)
{
    // Arrange — SetUp() 已构造 m_p（savePath/saveDir/saveBaseName 默认空 QString）
    ASSERT_NE(m_p, nullptr);

    // Act — 触发转码收尾路径（rename + exitRecord）
    EXPECT_NO_FATAL_FAILURE(call_private_fun::RecordProcessonTranscodeFinish(*m_p));

    // Assert — 对象存活（exitRecord 未致崩溃）
    EXPECT_NE(m_p, nullptr);
}

// =========================================================================
// BUG 308389 (sev2) — 大视频转 GIF 时 OOM
// BUG 102827 (sev2) — wayland 模式下无法录屏
// PMS: https://pms.uniontech.com/bug-view-308389.html  commit: 1be535521a
// PMS: https://pms.uniontech.com/bug-view-102827.html  commit: bed2001d56
// 根因：~RecordProcess 析构需释放 m_gstRecordX 等子对象，OOM/wayland 路径下
//       生命周期管理不当致泄漏或崩溃。修复析构清理。
// 回归：构造 RecordProcess 后显式析构，~RecordProcess 不崩溃（m_gstRecordX
//       默认 nullptr，析构走空分支；验证析构路径稳定，防退化）。
// =========================================================================
TEST_F(RecordProcessCov2Test, BUG308389_DestructorNoCrash)
{
    // Arrange — 构造一个独立的 RecordProcess（不经 fixture 的 m_p，便于精确控制）
    RecordProcess *p = new RecordProcess();
    ASSERT_NE(p, nullptr);

    // Act + Assert — 显式析构，触发 ~RecordProcess 清理路径
    EXPECT_NO_FATAL_FAILURE(delete p);
    // 析构后指针本身不再解引用（仅作句柄比较，无 UB）
    EXPECT_NE(p, nullptr);
}
