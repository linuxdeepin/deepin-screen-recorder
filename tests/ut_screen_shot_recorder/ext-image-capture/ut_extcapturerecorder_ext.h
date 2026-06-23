// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QGuiApplication>
#include <QScreen>
#include <QMetaObject>
#include <QElapsedTimer>
#include "addr_pri.h"
#include "../../src/ext-image-capture/extcapturerecorder.h"

using namespace testing;

// 仅暴露 ut_extcapturerecorder.h 中未声明的新增私有函数（避免同一 TU 内宏重定义）
ACCESS_PRIVATE_FUN(ExtCaptureRecorder, void(), finalizeRecording);

// 字段访问器：用于驱动 adjustVideoDurationIfNeeded 走不同早退分支
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, int, m_frameCount);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, qint64, m_firstFrameTimestampNs);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, qint64, m_lastFrameTimestampNs);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, int, m_frameRate);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, int, m_frameWidth);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, QString, m_outputPath);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, ExtCaptureRecorder::RecordState, m_state);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, bool, m_streamingMode);
ACCESS_PRIVATE_FIELD(ExtCaptureRecorder, bool, m_ffmpegStarted);

class ExtCaptureRecorderExtTest : public Test
{
public:
    ExtCaptureRecorder *m_rec;
    void SetUp() override { m_rec = new ExtCaptureRecorder; }
    void TearDown() override { delete m_rec; }
};

// generateDefaultOutputPath：路径始终指向 Documents（或 home 兜底），文件名带时间戳
TEST_F(ExtCaptureRecorderExtTest, generateDefaultOutputPathStableFormat)
{
    QString p = call_private_fun::ExtCaptureRecordergenerateDefaultOutputPath(*m_rec);
    ASSERT_FALSE(p.isEmpty());
    EXPECT_TRUE(p.endsWith(QStringLiteral(".mp4")));
    EXPECT_TRUE(p.contains(QStringLiteral("ext-capture-recording")));
    // 重复调用产生不同时间戳文件名（边界情形，秒级可能相同——只断言格式稳定）
    QString p2 = call_private_fun::ExtCaptureRecordergenerateDefaultOutputPath(*m_rec);
    EXPECT_TRUE(p2.endsWith(QStringLiteral(".mp4")));
}

// updateFrameTimestamps：第一帧设置 first/last；后续帧只更新 last
TEST_F(ExtCaptureRecorderExtTest, updateFrameTimestampsFirstAndSubsequent)
{
    // 负/零时间戳走 wallClock 兜底分支
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, 0));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, -100));
    // 正时间戳：首帧写入 first+last
    call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, 5000);
    EXPECT_EQ(access_private_field::ExtCaptureRecorderm_firstFrameTimestampNs(*m_rec), 5000);
    EXPECT_EQ(access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec), 5000);
    // 第二帧只更新 last
    call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, 9000);
    EXPECT_EQ(access_private_field::ExtCaptureRecorderm_firstFrameTimestampNs(*m_rec), 5000);
    EXPECT_EQ(access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec), 9000);
}

// adjustVideoDurationIfNeeded 早退路径 1：frameCount<=1
TEST_F(ExtCaptureRecorderExtTest, adjustDurationSkipWhenFrameCountLeOne)
{
    access_private_field::ExtCaptureRecorderm_frameCount(*m_rec) = 0;
    access_private_field::ExtCaptureRecorderm_firstFrameTimestampNs(*m_rec) = 1000;
    access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec) = 2000;
    EXPECT_TRUE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));

    access_private_field::ExtCaptureRecorderm_frameCount(*m_rec) = 1;
    EXPECT_TRUE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));
}

// adjustVideoDurationIfNeeded 早退路径 2：first<0
TEST_F(ExtCaptureRecorderExtTest, adjustDurationSkipWhenFirstTimestampNegative)
{
    access_private_field::ExtCaptureRecorderm_frameCount(*m_rec) = 10;
    access_private_field::ExtCaptureRecorderm_firstFrameTimestampNs(*m_rec) = -1;
    access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec) = 1000;
    EXPECT_TRUE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));
}

// adjustVideoDurationIfNeeded 早退路径 3：last<=first（含相等与倒退）
TEST_F(ExtCaptureRecorderExtTest, adjustDurationSkipWhenLastLeFirst)
{
    access_private_field::ExtCaptureRecorderm_frameCount(*m_rec) = 10;
    access_private_field::ExtCaptureRecorderm_firstFrameTimestampNs(*m_rec) = 1000;
    access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec) = 1000; // 相等
    EXPECT_TRUE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));

    access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec) = 500; // 倒退
    EXPECT_TRUE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));
}

// adjustVideoDurationIfNeeded 早退路径 4：intervalNs<=0（上面相等已覆盖，这里再断一次独立语义）
TEST_F(ExtCaptureRecorderExtTest, adjustDurationSkipWhenIntervalNonPositive)
{
    access_private_field::ExtCaptureRecorderm_frameCount(*m_rec) = 5;
    access_private_field::ExtCaptureRecorderm_firstFrameTimestampNs(*m_rec) = 0;
    access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec) = 0;
    EXPECT_TRUE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));
}

// adjustVideoDurationIfNeeded 进入校正分支（deviation>=3%）
// 注意：该校正分支会 spawn ffmpeg 进程——我们故意只断言其返回 false（ffmpeg 不存在/失败），
// 不验证文件副作用。这是安全的：QProcess::start 对不存在的 ffmpeg 会失败快速返回。
TEST_F(ExtCaptureRecorderExtTest, adjustDurationEntersCorrectionBranchAndFails)
{
    access_private_field::ExtCaptureRecorderm_frameCount(*m_rec) = 100;
    access_private_field::ExtCaptureRecorderm_frameRate(*m_rec) = 30;
    // 实际时长远大于编码时长 -> stretch factor 偏差巨大
    access_private_field::ExtCaptureRecorderm_firstFrameTimestampNs(*m_rec) = 0;
    // 10 秒的纳秒数
    access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec) =
        static_cast<qint64>(10) * 1000 * 1000 * 1000;
    access_private_field::ExtCaptureRecorderm_outputPath(*m_rec) =
        QStringLiteral("/tmp/ut_ext_rec_nonexist.mp4");
    // 进入 ffmpeg 校正分支；ffmpeg 缺失或失败 -> 返回 false，不抛异常
    EXPECT_FALSE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));
}

// adjustVideoDurationIfNeeded：偏差 < 3% 时跳过校正（构造接近理论编码时长的数据）
TEST_F(ExtCaptureRecorderExtTest, adjustDurationSkipWhenDeviationSmall)
{
    access_private_field::ExtCaptureRecorderm_frameCount(*m_rec) = 30;
    access_private_field::ExtCaptureRecorderm_frameRate(*m_rec) = 30;
    // 理论编码时长 = 30/30 = 1s；构造实际总时长 ~1s（偏差 <3%）
    // intervalNs = last-first；选 last 使 (interval + avgInterval) 接近 1s
    access_private_field::ExtCaptureRecorderm_firstFrameTimestampNs(*m_rec) = 0;
    // 29 帧间隔 * (1/29)s ≈ 0.966s；avgInterval ≈ 0.0333s；total ≈ 0.9994s ~ 1s
    qint64 nsPerFrame = static_cast<qint64>(1e9 / 29.0);
    access_private_field::ExtCaptureRecorderm_lastFrameTimestampNs(*m_rec) = nsPerFrame * 29;
    EXPECT_TRUE(call_private_fun::ExtCaptureRecorderadjustVideoDurationIfNeeded(*m_rec));
}

// processFrameQueue：仅日志，无副作用，反复调用安全
TEST_F(ExtCaptureRecorderExtTest, processFrameQueueIsSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderprocessFrameQueue(*m_rec));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderprocessFrameQueue(*m_rec));
}

// initializeCapture：仅日志，依赖 m_outputPath（空与非空两种）
TEST_F(ExtCaptureRecorderExtTest, initializeCaptureWithEmptyAndSetPath)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderinitializeCapture(*m_rec));
    access_private_field::ExtCaptureRecorderm_outputPath(*m_rec) =
        QStringLiteral("/tmp/ut_ext_rec_init.mp4");
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderinitializeCapture(*m_rec));
}

// finalizeRecording：缓冲模式无帧 + 流式模式未启动 ffmpeg，均走“无操作”收尾，安全
TEST_F(ExtCaptureRecorderExtTest, finalizeRecordingNoOpWhenIdle)
{
    // 默认 m_streamingMode=true 且 m_ffmpegStarted=false -> 跳过两个分支
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderfinalizeRecording(*m_rec));
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopped);
    EXPECT_EQ(m_rec->frameCount(), 0);
    EXPECT_EQ(access_private_field::ExtCaptureRecorderm_frameWidth(*m_rec), 0);
}

// finalizeRecording：缓冲模式但 frameBuffer 无帧 -> createVideoFile 分支被跳过（hasFrames false）
TEST_F(ExtCaptureRecorderExtTest, finalizeRecordingBufferModeNoFrames)
{
    access_private_field::ExtCaptureRecorderm_streamingMode(*m_rec) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderfinalizeRecording(*m_rec));
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopped);
}

// onExtCaptureUnavailable：非录制态 -> 仅日志，不触发 stopRecording/error
TEST_F(ExtCaptureRecorderExtTest, onExtCaptureUnavailableWhileStopped)
{
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_rec, "onExtCaptureUnavailable", Qt::DirectConnection));
    EXPECT_EQ(errSpy.count(), 0);
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Stopped);
}

// onExtCaptureError：传入空串与非空串两种
TEST_F(ExtCaptureRecorderExtTest, onExtCaptureErrorVariants)
{
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    QMetaObject::invokeMethod(m_rec, "onExtCaptureError", Qt::DirectConnection,
                              Q_ARG(QString, QString()));
    QMetaObject::invokeMethod(m_rec, "onExtCaptureError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("disk full")));
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Error);
    EXPECT_GE(errSpy.count(), 2);
}

// onCaptureTimer：m_extCapture 非空但 state!=Recording -> 早退
TEST_F(ExtCaptureRecorderExtTest, onCaptureTimerGuards)
{
    // Stopped -> 早退
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_rec, "onCaptureTimer", Qt::DirectConnection));
    // Error 态同样早退
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Error;
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_rec, "onCaptureTimer", Qt::DirectConnection));
}

// onFrameReady：state==Recording 但 data 非空、frameCount==0 时，frameBuffer 初始化失败路径
// （首帧分支：initialize 失败 -> emit error，不启动 ffmpeg）
TEST_F(ExtCaptureRecorderExtTest, onFrameReadyRecordingBufferInitFailure)
{
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Recording;
    access_private_field::ExtCaptureRecorderm_streamingMode(*m_rec) = false; // 走缓冲分支
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);

    unsigned char data[16] = {0};
    // width=0 -> frameBuffer->initialize 必然失败 -> emit error 并 return
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(
        m_rec, "onFrameReady", Qt::DirectConnection,
        Q_ARG(const void *, (const void *)data), Q_ARG(size_t, 16),
        Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(uint64_t, 100)));
    EXPECT_GE(errSpy.count(), 1);
    EXPECT_EQ(m_rec->frameCount(), 0);
}

// onDmaFrameReady：state==Recording 但 dmaBufferFd<0 或 gbmBo=null -> 早退
TEST_F(ExtCaptureRecorderExtTest, onDmaFrameReadyRecordingGuards)
{
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Recording;
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(
        m_rec, "onDmaFrameReady", Qt::DirectConnection,
        Q_ARG(int, -1), Q_ARG(void *, nullptr), Q_ARG(size_t, 0),
        Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(uint64_t, 0)));
    // fd 有效但 gbmBo 为空同样早退（用伪造的正 fd，不进入 mmap，因为 gbmBo 守卫先命中）
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(
        m_rec, "onDmaFrameReady", Qt::DirectConnection,
        Q_ARG(int, 42), Q_ARG(void *, nullptr), Q_ARG(size_t, 0),
        Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(uint64_t, 0)));
    EXPECT_EQ(m_rec->frameCount(), 0);
}

// setOutputPath：仅在 Stopped 写入；Error 态应被忽略
TEST_F(ExtCaptureRecorderExtTest, setOutputPathIgnoredWhenNotStopped)
{
    m_rec->setOutputPath(QStringLiteral("/tmp/a.mp4"));
    EXPECT_EQ(m_rec->outputPath(), QStringLiteral("/tmp/a.mp4"));
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Error;
    m_rec->setOutputPath(QStringLiteral("/tmp/b.mp4"));
    // Error 态不应覆盖
    EXPECT_EQ(m_rec->outputPath(), QStringLiteral("/tmp/a.mp4"));
}

// recordingDuration：m_startTime==0 -> 0；Recording 态 -> 当前差值
TEST_F(ExtCaptureRecorderExtTest, recordingDurationBranches)
{
    EXPECT_EQ(m_rec->recordingDuration(), 0); // m_startTime==0
    // 直接驱动 onRecordingStarted 设置 m_startTime 并进入 Recording
    QMetaObject::invokeMethod(m_rec, "onRecordingStarted", Qt::DirectConnection);
    EXPECT_GE(m_rec->recordingDuration(), 0);
    // 停止后回到非 Recording：recordingDuration 返回 m_startTime 差值（固定）
    QMetaObject::invokeMethod(m_rec, "onRecordingStopped", Qt::DirectConnection);
    EXPECT_GE(m_rec->recordingDuration(), 0);
}

// startRecording 状态守卫：非 Stopped 直接拒绝
TEST_F(ExtCaptureRecorderExtTest, startRecordingRejectedWhenNotStopped)
{
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    access_private_field::ExtCaptureRecorderm_state(*m_rec) = ExtCaptureRecorder::Recording;
    EXPECT_FALSE(m_rec->startRecording(nullptr));
    EXPECT_EQ(errSpy.count(), 0); // 状态守卫不发 error
}
