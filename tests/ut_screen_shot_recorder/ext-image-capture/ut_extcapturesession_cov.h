// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <drm/drm_fourcc.h>
#include <wayland-client.h>
#include "addr_pri.h"
#include "../../src/ext-image-capture/session/extcapturesession.h"

using namespace testing;

// 覆盖 extcapturesession.cpp 未触及的安全分支（注意：initialize/createFrame
// 依赖 Wayland 协议绑定，offscreen 下会 SEGV，故不直接调用）：
//  - handleDone 重复去重格式（handleShmFormat 已含则不重复 append）
//  - selectCapturePlan：优先尝试 DMA-BUF，失败后回退 SHM
//  - stop()：从 Ready 态停止（emit stopped，d->isInitialized()==false 分支）
//  - handleDmabufFormat：重复格式不重复 append
// 所有私有函数（handleBufferSize/handleShmFormat/handleDmabufDevice/handleDmabufFormat/
// handleDone/handleStopped/selectCapturePlan）已在 ut_extcapturesession.h 声明，
// 同一 TU 内可直接调用 call_private_fun，不重复声明。

class ExtCaptureSessionCovTest : public Test
{
public:
    ExtCaptureSession *m_sess;
    void SetUp() override { m_sess = new ExtCaptureSession; }
    void TearDown() override { delete m_sess; }
};

// handleShmFormat：同一格式重复传入 -> 仅 append 一次
TEST_F(ExtCaptureSessionCovTest, handleShmFormatDeduplicates)
{
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_XRGB8888);
    int before = m_sess->supportedFormats().size();
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_XRGB8888);
    EXPECT_EQ(m_sess->supportedFormats().size(), before);
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_ARGB8888);
    EXPECT_GT(m_sess->supportedFormats().size(), before);
}

// handleDmabufFormat：重复格式不重复 append
TEST_F(ExtCaptureSessionCovTest, handleDmabufFormatDeduplicates)
{
    call_private_fun::ExtCaptureSessionhandleDmabufFormat(*m_sess, 875713112, QList<uint64_t>{0});
    int before = m_sess->supportedFormats().size();
    call_private_fun::ExtCaptureSessionhandleDmabufFormat(*m_sess, 875713112, QList<uint64_t>{0});
    EXPECT_EQ(m_sess->supportedFormats().size(), before);
}

// selectCapturePlan：仅广告 DMA 格式但尚未协商 bufferSize 时，探测失败并回退 SHM
TEST_F(ExtCaptureSessionCovTest, selectCapturePlanFallsBackToShm)
{
    call_private_fun::ExtCaptureSessionhandleDmabufFormat(*m_sess, 875713112, QList<uint64_t>{0});
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_XRGB8888);
    call_private_fun::ExtCaptureSessionselectCapturePlan(*m_sess);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
    EXPECT_EQ(m_sess->config().format, WL_SHM_FORMAT_XRGB8888);
}

// selectCapturePlan：仅 shm -> useDmaBuffer=false
TEST_F(ExtCaptureSessionCovTest, selectCapturePlanShmOnlyWhenNoDma)
{
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_BGR888);
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_ARGB8888);
    call_private_fun::ExtCaptureSessionselectCapturePlan(*m_sess);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
    EXPECT_EQ(m_sess->config().format, WL_SHM_FORMAT_ARGB8888);
}

TEST_F(ExtCaptureSessionCovTest, selectsBg24AfterUnknownShmFormat)
{
    call_private_fun::ExtCaptureSessionhandleBufferSize(*m_sess, 1366, 768);
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_RGB565);
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_BGR888);
    call_private_fun::ExtCaptureSessionhandleDone(*m_sess);
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Ready);
    EXPECT_FALSE(m_sess->config().useDmaBuffer);
    EXPECT_EQ(m_sess->config().format, WL_SHM_FORMAT_BGR888);
}

TEST_F(ExtCaptureSessionCovTest, rejectsUnsupportedShmFormatsBeforeReady)
{
    QSignalSpy readySpy(m_sess, &ExtCaptureSession::ready);
    QSignalSpy errorSpy(m_sess, &ExtCaptureSession::error);
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, DRM_FORMAT_XRGB8888);
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, DRM_FORMAT_ARGB8888);
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_RGB565);
    call_private_fun::ExtCaptureSessionhandleDone(*m_sess);
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Error);
    EXPECT_EQ(readySpy.count(), 0);
    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_TRUE(errorSpy.first().first().toString().contains(QStringLiteral("RG16")));
}

// handleDone：设置 constraintsReceived 并转 Ready，可重复调用
TEST_F(ExtCaptureSessionCovTest, handleDoneTransitionsToReady)
{
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_XRGB8888);
    QSignalSpy readySpy(m_sess, &ExtCaptureSession::ready);
    call_private_fun::ExtCaptureSessionhandleDone(*m_sess);
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Ready);
    EXPECT_EQ(readySpy.count(), 1);
    // 再次 handleDone 仍 emit（无去重）
    call_private_fun::ExtCaptureSessionhandleDone(*m_sess);
    EXPECT_EQ(readySpy.count(), 2);
}

// stop()：从 Ready 态停止 -> emit stopped（d->isInitialized()==false 走跳过分支）
TEST_F(ExtCaptureSessionCovTest, stopFromReadyEmitsStopped)
{
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, WL_SHM_FORMAT_XRGB8888);
    call_private_fun::ExtCaptureSessionhandleDone(*m_sess);
    ASSERT_EQ(m_sess->state(), ExtCaptureSession::Ready);

    QSignalSpy stoppedSpy(m_sess, &ExtCaptureSession::stopped);
    EXPECT_NO_FATAL_FAILURE(m_sess->stop());
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Stopped);
    EXPECT_EQ(stoppedSpy.count(), 1);
    // Stopped 态再次 stop -> 早退无 emit
    EXPECT_NO_FATAL_FAILURE(m_sess->stop());
    EXPECT_EQ(stoppedSpy.count(), 1);
}

// createFrame：state != Ready/Capturing（如 Uninitialized）-> nullptr
TEST_F(ExtCaptureSessionCovTest, createFrameFailsInUninitialized)
{
    EXPECT_EQ(m_sess->createFrame(), nullptr);
}

// handleBufferSize：覆盖 config.bufferSize 写入
TEST_F(ExtCaptureSessionCovTest, handleBufferSizeSetsConfig)
{
    call_private_fun::ExtCaptureSessionhandleBufferSize(*m_sess, 320, 240);
    EXPECT_EQ(m_sess->config().bufferSize, QSize(320, 240));
}

// handleDmabufDevice：覆盖 device 写入
TEST_F(ExtCaptureSessionCovTest, handleDmabufDeviceSetsDevice)
{
    call_private_fun::ExtCaptureSessionhandleDmabufDevice(*m_sess, QByteArray("/dev/dri/renderD129"));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureSessionhandleDmabufDevice(*m_sess, QByteArray()));
}

// config()：默认 CaptureConfig 字段
TEST_F(ExtCaptureSessionCovTest, defaultConfigValues)
{
    EXPECT_TRUE(m_sess->config().bufferSize.isEmpty());
    EXPECT_EQ(m_sess->config().format, 0u);
    EXPECT_TRUE(m_sess->config().useDmaBuffer); // 默认 true
    EXPECT_FALSE(m_sess->config().paintCursors);
}
