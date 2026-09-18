// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "addr_pri.h"
#include "../../src/ext-image-capture/session/extcapturesession.h"

using namespace testing;

// protected 协议事件处理 + selectOptimalFormat：安全纯逻辑
ACCESS_PRIVATE_FUN(ExtCaptureSession, void(uint32_t, uint32_t), handleBufferSize);
ACCESS_PRIVATE_FUN(ExtCaptureSession, void(uint32_t), handleShmFormat);
ACCESS_PRIVATE_FUN(ExtCaptureSession, void(const QByteArray &), handleDmabufDevice);
ACCESS_PRIVATE_FUN(ExtCaptureSession, void(uint32_t, const QList<uint64_t> &), handleDmabufFormat);
ACCESS_PRIVATE_FUN(ExtCaptureSession, void(), handleDone);
ACCESS_PRIVATE_FUN(ExtCaptureSession, void(), handleStopped);
ACCESS_PRIVATE_FUN(ExtCaptureSession, void(), selectOptimalFormat);

class ExtCaptureSessionTest : public Test
{
public:
    ExtCaptureSession *m_sess;
    void SetUp() override { m_sess = new ExtCaptureSession; }
    void TearDown() override { delete m_sess; }
};

TEST_F(ExtCaptureSessionTest, defaults)
{
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Uninitialized);
    EXPECT_TRUE(m_sess->supportedFormats().isEmpty());
    EXPECT_EQ(m_sess->getWaylandShm(), nullptr);
    EXPECT_EQ(m_sess->getLinuxDmabuf(), nullptr);
    EXPECT_EQ(m_sess->createFrame(), nullptr); // state != Ready/Capturing
}

// 注意：initialize() 会触发 initializeWaylandGlobals()，在 x11 上对 X11 Display*
// 调用 wl_display_get_registry 会 SEGV，因此不在单测中直接调用 initialize()。

TEST_F(ExtCaptureSessionTest, stopGuard)
{
    EXPECT_NO_FATAL_FAILURE(m_sess->stop()); // Uninitialized -> 直接返回
}

TEST_F(ExtCaptureSessionTest, protectedHandlers)
{
    call_private_fun::ExtCaptureSessionhandleBufferSize(*m_sess, 100, 50);
    EXPECT_EQ(m_sess->config().bufferSize, QSize(100, 50));

    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, 875713112); // WL_SHM_FORMAT_XBGR8888
    EXPECT_FALSE(m_sess->supportedFormats().isEmpty());

    call_private_fun::ExtCaptureSessionhandleDmabufDevice(*m_sess, QByteArray("/dev/dri/renderD128"));
    call_private_fun::ExtCaptureSessionhandleDmabufFormat(*m_sess, 875713112, QList<uint64_t>{0});

    // selectOptimalFormat：已有 shm 格式 -> 选中
    call_private_fun::ExtCaptureSessionselectOptimalFormat(*m_sess);
    EXPECT_NE(m_sess->config().format, 0u);
}

TEST_F(ExtCaptureSessionTest, handleDoneAndStopped)
{
    QSignalSpy readySpy(m_sess, &ExtCaptureSession::ready);
    call_private_fun::ExtCaptureSessionhandleShmFormat(*m_sess, 875713112);
    call_private_fun::ExtCaptureSessionhandleDone(*m_sess);
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Ready);
    EXPECT_EQ(readySpy.count(), 1);

    QSignalSpy stoppedSpy(m_sess, &ExtCaptureSession::stopped);
    call_private_fun::ExtCaptureSessionhandleStopped(*m_sess);
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Stopped);
    EXPECT_EQ(stoppedSpy.count(), 1);
}

TEST_F(ExtCaptureSessionTest, selectOptimalFormatNoFormats)
{
    // 无任何格式 -> error + Error 状态
    QSignalSpy errSpy(m_sess, &ExtCaptureSession::error);
    call_private_fun::ExtCaptureSessionselectOptimalFormat(*m_sess);
    EXPECT_GE(errSpy.count(), 1);
}
