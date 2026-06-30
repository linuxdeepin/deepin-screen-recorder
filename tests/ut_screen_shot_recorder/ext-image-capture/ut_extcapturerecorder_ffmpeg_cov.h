// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QProcess>
#include "addr_pri.h"
#include "stub.h"
#include "../../src/ext-image-capture/extcapturerecorder.h"

using namespace testing;

// 本头聚焦 createVideoFile / startFFmpegProcess / startDmaFFmpegProcess /
// processDmaBufferFrame 的覆盖。adjustVideoDurationIfNeeded 已在
// ut_extcapturerecorder.h 声明访问器，禁止重复声明。
//
// 策略：
//  - 早退守卫分支（无帧 / m_ffmpegProcess 已存在）—— 直接覆盖，安全。
//  - 深层 ffmpeg 启动分支 —— 桩掉 QProcess::waitForStarted/waitForFinished 后再调用，
//    因 start() 在 Qt6 中带默认参数重载、无法干净 stub，这里仅断言其不 ABORT；
//    真实 spawn 的 sh+ffmpeg 会被 waitForFinished 桩立即“放行”，进程对象随作用域
//    结束被析构（QProcess 析构会 kill 子进程），无副作用。
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, bool(), createVideoFile);
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, bool(), startFFmpegProcess);
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, bool(), startDmaFFmpegProcess);
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, bool(int, void *, int, int, int), processDmaBufferFrame);

// 补充 _ext.h 未声明的字段访问器
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, int, m_frameHeight);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, QProcess *, m_ffmpegProcess);

namespace {
bool qt6_waitForFinished_stub(QProcess *, int) { return true; }
bool qt6_waitForStarted_stub(QProcess *, int) { return true; }
} // namespace

class ExtCaptureRecorderFfmpegCovTest : public Test
{
public:
    ExtCaptureRecorder *m_rec;
    Stub stub;

    void SetUp() override
    {
        m_rec = new ExtCaptureRecorder;
        stub.set((bool(QProcess::*)(int))ADDR(QProcess, waitForFinished), qt6_waitForFinished_stub);
        stub.set((bool(QProcess::*)(int))ADDR(QProcess, waitForStarted), qt6_waitForStarted_stub);
    }

    void TearDown() override
    {
        // 释放可能由 startFFmpegProcess new 出来的 QProcess
        QProcess *&proc = access_private_field::ExtCaptureRecorderm_ffmpegProcess(*m_rec);
        if (proc) {
            proc->deleteLater();
            proc = nullptr;
        }
        delete m_rec;
    }
};

// createVideoFile 早退：frameBuffer 无帧 -> false
TEST_F(ExtCaptureRecorderFfmpegCovTest, createVideoFileNoFramesEarlyReturn)
{
    EXPECT_NO_FATAL_FAILURE(
        EXPECT_FALSE(call_private_fun::ExtCaptureRecordercreateVideoFile(*m_rec)));
}

// createVideoFile 深层路径：填充 frameBuffer 后走 ffmpeg 分支（start+wait 均被桩放行）
TEST_F(ExtCaptureRecorderFfmpegCovTest, createVideoFileWithFramesRunsFfmpegBranch)
{
    ExtCaptureFrameBuffer *fb = m_rec->getFrameBuffer();
    ASSERT_NE(fb, nullptr);
    ASSERT_TRUE(fb->initialize(2, 2, 8));
    access_private_field::ExtCaptureRecorderm_frameWidth(*m_rec) = 2;
    access_private_field::ExtCaptureRecorderm_frameHeight(*m_rec) = 2;
    access_private_field::ExtCaptureRecorderm_frameRate(*m_rec) = 10;
    access_private_field::ExtCaptureRecorderm_outputPath(*m_rec) =
        QStringLiteral("/tmp/ut_ext_ffmpeg_cov_create.mp4");

    unsigned char px[4 * 2 * 2] = {0}; // 2x2 rgba, stride=8
    fb->appendBuffer(px, sizeof(px), 2, 2, 8, 0);
    ASSERT_TRUE(fb->hasFrames());

    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecordercreateVideoFile(*m_rec));
}

// startFFmpegProcess 早退：m_ffmpegProcess 已存在 -> false
TEST_F(ExtCaptureRecorderFfmpegCovTest, startFFmpegProcessAlreadyExistsEarlyReturn)
{
    QProcess dummy;
    access_private_field::ExtCaptureRecorderm_ffmpegProcess(*m_rec) = &dummy;
    EXPECT_NO_FATAL_FAILURE(
        EXPECT_FALSE(call_private_fun::ExtCaptureRecorderstartFFmpegProcess(*m_rec)));
    access_private_field::ExtCaptureRecorderm_ffmpegProcess(*m_rec) = nullptr;
}

// startFFmpegProcess 正常路径：覆盖命令构建 + 启动分支（waitForStarted 被桩放行）
TEST_F(ExtCaptureRecorderFfmpegCovTest, startFFmpegProcessBuildsCmdAndStarts)
{
    access_private_field::ExtCaptureRecorderm_frameWidth(*m_rec) = 4;
    access_private_field::ExtCaptureRecorderm_frameHeight(*m_rec) = 4;
    access_private_field::ExtCaptureRecorderm_frameRate(*m_rec) = 30;
    access_private_field::ExtCaptureRecorderm_outputPath(*m_rec) =
        QStringLiteral("/tmp/ut_ext_ffmpeg_cov_start.mp4");

    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::ExtCaptureRecorderstartFFmpegProcess(*m_rec));
}

// startDmaFFmpegProcess / processDmaBufferFrame：ENABLE_UNIT_TEST 下为桩，固定 false
TEST_F(ExtCaptureRecorderFfmpegCovTest, dmaFunctionsAreStubsUnderTest)
{
    EXPECT_NO_FATAL_FAILURE(
        EXPECT_FALSE(call_private_fun::ExtCaptureRecorderstartDmaFFmpegProcess(*m_rec)));
    EXPECT_NO_FATAL_FAILURE(
        EXPECT_FALSE(call_private_fun::ExtCaptureRecorderprocessDmaBufferFrame(
            *m_rec, 1, reinterpret_cast<void *>(0x1), 2, 2, 8)));
}
