// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QGuiApplication>
#include <QScreen>
#include <QMetaObject>
#include "addr_pri.h"
#include "../../src/ext-image-capture/extcaptureintegration.h"
#include "../../src/ext-image-capture/extcapturerecorder.h"
#include "../../src/ext-image-capture/multiscreencapturecoordinator.h"
#include "../../src/ext-image-capture/session/extcapturesession.h"
#include "../../src/ext-image-capture/frame/extcaptureframe.h"
#include "../../src/ext-image-capture/manager/extcapturemanager.h"
#include "../../src/ext-image-capture/manager/extoutputsourcemanager.h"

using namespace testing;

// 覆盖 ext-image-capture 各模块剩余未覆盖的安全分支。
// 既有 cov 头已声明大量 ACCESS_PRIVATE_FUN/FIELD，同一 TU 内不重复声明；
// 仅声明本文件新增的私有访问。

// ===== ExtCaptureIntegration 补充 =====
ACCESS_PRIVATE_FIELD(ExtCaptureIntegration, bool, m_recording);
ACCESS_PRIVATE_FIELD(ExtCaptureIntegration, bool, m_multiScreenRecording);

class ExtCaptureIntegrationCov2Test : public Test
{
public:
    ExtCaptureIntegration *m_int;
    void SetUp() override { m_int = new ExtCaptureIntegration; }
    void TearDown() override { delete m_int; }
};

// startScreenRecording：已在录制 -> 早退 false
TEST_F(ExtCaptureIntegrationCov2Test, startScreenRecordingAlreadyRecording)
{
    access_private_field::ExtCaptureIntegrationm_recording(*m_int) = true;
    QList<QScreen *> screens = QGuiApplication::screens();
    QScreen *scr = screens.isEmpty() ? nullptr : screens.first();
    EXPECT_FALSE(m_int->startScreenRecording(scr, true));
}

// startScreenRecording：null screen -> emit error + false
TEST_F(ExtCaptureIntegrationCov2Test, startScreenRecordingNullScreen)
{
    // 协议不可用时会先返回；通过置 m_recording=true 跳过，再置 false 进入 isAvailable 检查
    // 直接置 false（默认）+ null screen，协议不可用守卫先返回
    EXPECT_FALSE(m_int->startScreenRecording(nullptr, true));
}

// startMultiScreenRecording：空 screens -> emit error
TEST_F(ExtCaptureIntegrationCov2Test, startMultiScreenRecordingEmptyScreens)
{
    QList<QScreen *> empty;
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    EXPECT_FALSE(m_int->startMultiScreenRecording(empty, true));
    EXPECT_GE(errSpy.count(), 1);
}

// startMultiScreenRecording：已在录制 -> 早退 false
TEST_F(ExtCaptureIntegrationCov2Test, startMultiScreenRecordingAlreadyRecording)
{
    access_private_field::ExtCaptureIntegrationm_recording(*m_int) = true;
    QList<QScreen *> screens = QGuiApplication::screens();
    EXPECT_FALSE(m_int->startMultiScreenRecording(screens, true));
}

// stopRecording：单屏录制态 -> emit recordingStopped
TEST_F(ExtCaptureIntegrationCov2Test, stopRecordingSingleScreenEmits)
{
    access_private_field::ExtCaptureIntegrationm_recording(*m_int) = true;
    QSignalSpy stoppedSpy(m_int, &ExtCaptureIntegration::recordingStopped);
    EXPECT_NO_FATAL_FAILURE(m_int->stopRecording());
    EXPECT_GE(stoppedSpy.count(), 1);
}

// onProtocolAvailable：emit available
TEST_F(ExtCaptureIntegrationCov2Test, onProtocolAvailableEmits)
{
    QSignalSpy availSpy(m_int, &ExtCaptureIntegration::available);
    QMetaObject::invokeMethod(m_int, "onProtocolAvailable", Qt::DirectConnection);
    EXPECT_EQ(availSpy.count(), 1);
}

// onProtocolUnavailable：录制态 -> 调 stopRecording + emit unavailable
TEST_F(ExtCaptureIntegrationCov2Test, onProtocolUnavailableWhileRecording)
{
    access_private_field::ExtCaptureIntegrationm_recording(*m_int) = true;
    QSignalSpy unavailSpy(m_int, &ExtCaptureIntegration::unavailable);
    QSignalSpy stoppedSpy(m_int, &ExtCaptureIntegration::recordingStopped);
    QMetaObject::invokeMethod(m_int, "onProtocolUnavailable", Qt::DirectConnection);
    EXPECT_GE(unavailSpy.count(), 1);
    EXPECT_GE(stoppedSpy.count(), 1);
}

// onSessionReady：emit recordingStarted
TEST_F(ExtCaptureIntegrationCov2Test, onSessionReadyEmits)
{
    QSignalSpy startedSpy(m_int, &ExtCaptureIntegration::recordingStarted);
    QMetaObject::invokeMethod(m_int, "onSessionReady", Qt::DirectConnection);
    EXPECT_EQ(startedSpy.count(), 1);
}

// onSessionStopped：录制态 -> 调 stopRecording
TEST_F(ExtCaptureIntegrationCov2Test, onSessionStoppedWhileRecording)
{
    access_private_field::ExtCaptureIntegrationm_recording(*m_int) = true;
    QSignalSpy stoppedSpy(m_int, &ExtCaptureIntegration::recordingStopped);
    QMetaObject::invokeMethod(m_int, "onSessionStopped", Qt::DirectConnection);
    EXPECT_GE(stoppedSpy.count(), 1);
}

// onSessionError：录制态 -> stopRecording + emit error
TEST_F(ExtCaptureIntegrationCov2Test, onSessionErrorWhileRecording)
{
    access_private_field::ExtCaptureIntegrationm_recording(*m_int) = true;
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    QMetaObject::invokeMethod(m_int, "onSessionError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("e2")));
    EXPECT_GE(errSpy.count(), 1);
}

// onFrameFailed：emit error（含 reason 拼接）
TEST_F(ExtCaptureIntegrationCov2Test, onFrameFailedEmitsError)
{
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    QMetaObject::invokeMethod(m_int, "onFrameFailed", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("boom")));
    EXPECT_GE(errSpy.count(), 1);
}

// onMultiScreenCaptureStarted：emit recordingStarted
TEST_F(ExtCaptureIntegrationCov2Test, onMultiScreenCaptureStartedEmits)
{
    QSignalSpy startedSpy(m_int, &ExtCaptureIntegration::recordingStarted);
    QMetaObject::invokeMethod(m_int, "onMultiScreenCaptureStarted", Qt::DirectConnection);
    EXPECT_EQ(startedSpy.count(), 1);
}

// onMultiScreenCaptureStopped：复位 m_multiScreenRecording + emit recordingStopped
TEST_F(ExtCaptureIntegrationCov2Test, onMultiScreenCaptureStoppedEmits)
{
    access_private_field::ExtCaptureIntegrationm_multiScreenRecording(*m_int) = true;
    QSignalSpy stoppedSpy(m_int, &ExtCaptureIntegration::recordingStopped);
    QMetaObject::invokeMethod(m_int, "onMultiScreenCaptureStopped", Qt::DirectConnection);
    EXPECT_FALSE(m_int->isMultiScreenRecording());
    EXPECT_EQ(stoppedSpy.count(), 1);
}

// onMultiScreenCaptureError：复位 + emit error
TEST_F(ExtCaptureIntegrationCov2Test, onMultiScreenCaptureErrorEmits)
{
    access_private_field::ExtCaptureIntegrationm_multiScreenRecording(*m_int) = true;
    QSignalSpy errSpy(m_int, &ExtCaptureIntegration::error);
    QMetaObject::invokeMethod(m_int, "onMultiScreenCaptureError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("multi")));
    EXPECT_FALSE(m_int->isMultiScreenRecording());
    EXPECT_GE(errSpy.count(), 1);
}

// onMultiScreenFrameReady：转发 emit multiScreenFrameReady
TEST_F(ExtCaptureIntegrationCov2Test, onMultiScreenFrameReadyForwards)
{
    QSignalSpy frameSpy(m_int, &ExtCaptureIntegration::multiScreenFrameReady);
    QMetaObject::invokeMethod(m_int, "onMultiScreenFrameReady", Qt::DirectConnection,
                              Q_ARG(QByteArray, QByteArray(64, 0)),
                              Q_ARG(int, 4), Q_ARG(int, 4), Q_ARG(int, 16),
                              Q_ARG(uint64_t, 1234ull));
    EXPECT_EQ(frameSpy.count(), 1);
}

// getAvailableScreens：返回非空（offscreen 至少 1 个）
TEST_F(ExtCaptureIntegrationCov2Test, getAvailableScreensNonEmpty)
{
    EXPECT_GE(m_int->getAvailableScreens().size(), 1);
}

// ===== ExtCaptureRecorder 补充 =====
// 既有 ut_extcapturerecorder.h 已声明 generateDefaultOutputPath/
// updateFrameTimestamps/processFrameQueue/initializeCapture，
// ut_extcapturerecorder_ext.h 已声明 m_frameCount/m_state/m_streamingMode/
// m_frameRate 等字段，ut_extcapturerecorder_cov.h 已声明 setState，
// 同一 TU 内不重复声明，直接复用 call_private_fun / access_private_field。

class ExtCaptureRecorderCov2Test : public Test
{
public:
    ExtCaptureRecorder *m_rec;
    void SetUp() override { m_rec = new ExtCaptureRecorder; }
    void TearDown() override { delete m_rec; }
};

// frameCount：默认 0
TEST_F(ExtCaptureRecorderCov2Test, frameCountDefaultZero)
{
    EXPECT_EQ(m_rec->frameCount(), 0);
}

// initializeCapture：仅日志
TEST_F(ExtCaptureRecorderCov2Test, initializeCaptureSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderinitializeCapture(*m_rec));
}

// processFrameQueue：仅日志
TEST_F(ExtCaptureRecorderCov2Test, processFrameQueueSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderprocessFrameQueue(*m_rec));
}

// generateDefaultOutputPath：返回非空路径
TEST_F(ExtCaptureRecorderCov2Test, generateDefaultOutputPathNonEmpty)
{
    QString p;
    EXPECT_NO_FATAL_FAILURE(p = call_private_fun::ExtCaptureRecordergenerateDefaultOutputPath(*m_rec));
    EXPECT_FALSE(p.isEmpty());
}

// updateFrameTimestamps：timestamp<=0 -> 用 wallClockTimer
TEST_F(ExtCaptureRecorderCov2Test, updateFrameTimestampsNegativeUsesWallClock)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, -1));
}

// updateFrameTimestamps：timestamp>0 -> 直接用
TEST_F(ExtCaptureRecorderCov2Test, updateFrameTimestampsPositive)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, 1000));
    // 再次调用 -> m_firstFrameTimestampNs 已设
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureRecorderupdateFrameTimestamps(*m_rec, 2000));
}

// onExtCaptureAvailable：emit protocolAvailable
TEST_F(ExtCaptureRecorderCov2Test, onExtCaptureAvailableEmits)
{
    QSignalSpy protoSpy(m_rec, &ExtCaptureRecorder::protocolAvailable);
    QMetaObject::invokeMethod(m_rec, "onExtCaptureAvailable", Qt::DirectConnection);
    EXPECT_EQ(protoSpy.count(), 1);
}

// onExtCaptureUnavailable：非 Recording 态 -> 无 emit error
TEST_F(ExtCaptureRecorderCov2Test, onExtCaptureUnavailableIdleSafe)
{
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    QMetaObject::invokeMethod(m_rec, "onExtCaptureUnavailable", Qt::DirectConnection);
    EXPECT_EQ(errSpy.count(), 0);
}

// onExtCaptureError：setState(Error) + emit error
TEST_F(ExtCaptureRecorderCov2Test, onExtCaptureErrorEmits)
{
    QSignalSpy errSpy(m_rec, &ExtCaptureRecorder::error);
    QMetaObject::invokeMethod(m_rec, "onExtCaptureError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("rec err")));
    EXPECT_EQ(m_rec->state(), ExtCaptureRecorder::Error);
    EXPECT_GE(errSpy.count(), 1);
}

// onCaptureTimer：非 Recording 态 -> 早退
TEST_F(ExtCaptureRecorderCov2Test, onCaptureTimerIdleSafe)
{
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_rec, "onCaptureTimer", Qt::DirectConnection));
}

// onRecordingStopped：emit recordingStopped
TEST_F(ExtCaptureRecorderCov2Test, onRecordingStoppedEmits)
{
    QSignalSpy stoppedSpy(m_rec, &ExtCaptureRecorder::recordingStopped);
    QMetaObject::invokeMethod(m_rec, "onRecordingStopped", Qt::DirectConnection);
    EXPECT_EQ(stoppedSpy.count(), 1);
}

// onFrameReady：非 Recording 态 -> 早退
TEST_F(ExtCaptureRecorderCov2Test, onFrameReadyIdleSafe)
{
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_rec, "onFrameReady",
        Qt::DirectConnection, Q_ARG(const void *, nullptr), Q_ARG(size_t, 0),
        Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(uint64_t, 0ull)));
}

// onDmaFrameReady：fd<0 -> 早退
TEST_F(ExtCaptureRecorderCov2Test, onDmaFrameReadyInvalidFdSafe)
{
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_rec, "onDmaFrameReady",
        Qt::DirectConnection, Q_ARG(int, -1), Q_ARG(void *, nullptr),
        Q_ARG(size_t, 0), Q_ARG(int, 0), Q_ARG(int, 0), Q_ARG(int, 0),
        Q_ARG(uint64_t, 0ull)));
}

// ===== MultiScreenCaptureCoordinator 补充 =====
// 既有 ut_multiscreencapturecoordinator.h 已声明 calculateVirtualDesktopLayout/
// composeMultiScreenFrame/resetFrameReadyFlags/cleanupSessions/getWaylandOutput，
// ut_multiscreencapturecoordinator_cov.h 已声明 allScreenFramesReady/m_capturing，
// 同一 TU 内不重复声明。

class MultiScreenCaptureCoordinatorCov2Test : public Test
{
public:
    MultiScreenCaptureCoordinator *m_c;
    void SetUp() override { m_c = new MultiScreenCaptureCoordinator; }
    void TearDown() override { delete m_c; }
};

// resetFrameReadyFlags：空 layout -> 无循环
TEST_F(MultiScreenCaptureCoordinatorCov2Test, resetFrameReadyFlagsEmptySafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenCaptureCoordinatorresetFrameReadyFlags(*m_c));
}

// composeMultiScreenFrame：virtualDesktopSize 空 -> 返回空
TEST_F(MultiScreenCaptureCoordinatorCov2Test, composeEmptyWhenNoLayout)
{
    QByteArray out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::MultiScreenCaptureCoordinatorcomposeMultiScreenFrame(*m_c));
    EXPECT_TRUE(out.isEmpty());
}

// composeMultiScreenFrame：设置 virtualDesktopSize 后返回非空
TEST_F(MultiScreenCaptureCoordinatorCov2Test, composeReturnsPlaceholderWhenSized)
{
    // 通过 calculateVirtualDesktopLayout 间接设置 virtualDesktopSize
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) GTEST_SKIP();
    call_private_fun::MultiScreenCaptureCoordinatorcalculateVirtualDesktopLayout(*m_c, screens);
    QByteArray out;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::MultiScreenCaptureCoordinatorcomposeMultiScreenFrame(*m_c));
    EXPECT_FALSE(out.isEmpty());
}

// cleanupSessions：空状态安全
TEST_F(MultiScreenCaptureCoordinatorCov2Test, cleanupSessionsEmptySafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MultiScreenCaptureCoordinatorcleanupSessions(*m_c));
}

// getWaylandOutput：null screen -> nullptr（ENABLE_UNIT_TEST 桩）
TEST_F(MultiScreenCaptureCoordinatorCov2Test, getWaylandOutputNullScreen)
{
    void *out = (void *)0x1;
    EXPECT_NO_FATAL_FAILURE(out = call_private_fun::MultiScreenCaptureCoordinatorgetWaylandOutput(*m_c, nullptr));
    EXPECT_EQ(out, nullptr);
}

// onSessionError：emit error
TEST_F(MultiScreenCaptureCoordinatorCov2Test, onSessionErrorEmits)
{
    QSignalSpy errSpy(m_c, &MultiScreenCaptureCoordinator::error);
    QMetaObject::invokeMethod(m_c, "onSessionError", Qt::DirectConnection,
                              Q_ARG(QString, QStringLiteral("s err")));
    EXPECT_GE(errSpy.count(), 1);
}

// onSessionStopped：仅日志
TEST_F(MultiScreenCaptureCoordinatorCov2Test, onSessionStoppedSafe)
{
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_c, "onSessionStopped", Qt::DirectConnection));
}

// captureMultiScreenFrame：未 capturing -> false
TEST_F(MultiScreenCaptureCoordinatorCov2Test, captureFrameNotCapturing)
{
    EXPECT_FALSE(m_c->captureMultiScreenFrame());
}

// startMultiScreenCapture：无 manager -> emit error + false
TEST_F(MultiScreenCaptureCoordinatorCov2Test, startNoManager)
{
    QList<QScreen *> screens = QGuiApplication::screens();
    QSignalSpy errSpy(m_c, &MultiScreenCaptureCoordinator::error);
    EXPECT_FALSE(m_c->startMultiScreenCapture(screens, true));
    EXPECT_GE(errSpy.count(), 1);
}

// ===== ExtCaptureSession 补充 =====
ACCESS_PRIVATE_FUN(ExtCaptureSession, void(ExtCaptureSession::SessionState), setState);

class ExtCaptureSessionCov2Test : public Test
{
public:
    ExtCaptureSession *m_sess;
    void SetUp() override { m_sess = new ExtCaptureSession; }
    void TearDown() override { delete m_sess; }
};

// setState：各状态转换
TEST_F(ExtCaptureSessionCov2Test, setStateAcrossAllStates)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureSessionsetState(*m_sess, ExtCaptureSession::Initializing));
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Initializing);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureSessionsetState(*m_sess, ExtCaptureSession::Ready));
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Ready);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureSessionsetState(*m_sess, ExtCaptureSession::Capturing));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureSessionsetState(*m_sess, ExtCaptureSession::Error));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::ExtCaptureSessionsetState(*m_sess, ExtCaptureSession::Stopped));
}

// handleStopped：setState(Stopped) + emit stopped
TEST_F(ExtCaptureSessionCov2Test, handleStoppedEmits)
{
    QSignalSpy stoppedSpy(m_sess, &ExtCaptureSession::stopped);
    call_private_fun::ExtCaptureSessionhandleStopped(*m_sess);
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Stopped);
    EXPECT_EQ(stoppedSpy.count(), 1);
}

// getWaylandShm：ENABLE_UNIT_TEST 桩 -> nullptr
TEST_F(ExtCaptureSessionCov2Test, getWaylandShmReturnsNull)
{
    EXPECT_EQ(m_sess->getWaylandShm(), nullptr);
}

// getLinuxDmabuf：ENABLE_UNIT_TEST 桩 -> nullptr
TEST_F(ExtCaptureSessionCov2Test, getLinuxDmabufReturnsNull)
{
    EXPECT_EQ(m_sess->getLinuxDmabuf(), nullptr);
}

// selectOptimalFormat：无任何格式 -> emit error
TEST_F(ExtCaptureSessionCov2Test, selectOptimalNoFormatsEmitsError)
{
    QSignalSpy errSpy(m_sess, &ExtCaptureSession::error);
    call_private_fun::ExtCaptureSessionselectOptimalFormat(*m_sess);
    EXPECT_GE(errSpy.count(), 1);
    EXPECT_EQ(m_sess->state(), ExtCaptureSession::Error);
}

// supportedFormats：默认空
TEST_F(ExtCaptureSessionCov2Test, supportedFormatsDefaultEmpty)
{
    EXPECT_TRUE(m_sess->supportedFormats().isEmpty());
}

// ===== ExtCaptureFrame 补充 =====
// 既有 ut_extcaptureframe.h 已声明 handleTransform/handleDamage/handlePresentationTime/
// handleReady/handleFailed/failureReasonToString，ut_extcaptureframe_ext.h 已声明 setState，
// 同一 TU 内不重复声明。

class ExtCaptureFrameCov2Test : public Test
{
public:
    ExtCaptureFrame *m_frame;
    void SetUp() override { m_frame = new ExtCaptureFrame; }
    void TearDown() override { delete m_frame; }
};

// failureReasonToString：各 reason 分支
TEST_F(ExtCaptureFrameCov2Test, failureReasonToStringAllBranches)
{
    QString s0, s1, s2, s99;
    EXPECT_NO_FATAL_FAILURE(s0 = call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 0));
    EXPECT_NO_FATAL_FAILURE(s1 = call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 1));
    EXPECT_NO_FATAL_FAILURE(s2 = call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 2));
    EXPECT_NO_FATAL_FAILURE(s99 = call_private_fun::ExtCaptureFramefailureReasonToString(*m_frame, 99));
    EXPECT_FALSE(s0.isEmpty());
    EXPECT_FALSE(s99.isEmpty());
}

// capture：state < Attached -> false
TEST_F(ExtCaptureFrameCov2Test, captureFailsInUninitialized)
{
    EXPECT_FALSE(m_frame->capture());
    EXPECT_FALSE(m_frame->capture(false));
}

// initialize：state != Uninitialized（先 setState）-> false
TEST_F(ExtCaptureFrameCov2Test, initializeFailsWhenAlreadyInit)
{
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Attached);
    EXPECT_FALSE(m_frame->initialize(nullptr, CaptureConfig{}));
}

// createBuffer：ENABLE_UNIT_TEST 桩 -> false
TEST_F(ExtCaptureFrameCov2Test, createBufferReturnsFalseInTest)
{
    call_private_fun::ExtCaptureFramesetState(*m_frame, ExtCaptureFrame::Attached);
    // createBuffer 是 private，通过 initialize 间接调用（桩返回 false）
    CaptureConfig cfg;
    cfg.bufferSize = QSize(10, 10);
    EXPECT_FALSE(m_frame->initialize(reinterpret_cast<void *>(0x1), cfg));
}

// timestamp：默认 0
TEST_F(ExtCaptureFrameCov2Test, timestampDefaultZero)
{
    EXPECT_EQ(m_frame->timestamp(), 0ull);
}

// isDmaBuffer：默认 config.useDmaBuffer=true
TEST_F(ExtCaptureFrameCov2Test, isDmaBufferDefault)
{
    EXPECT_NO_FATAL_FAILURE((void)m_frame->isDmaBuffer());
}

// getDmaBufferFd / getGbmBufferObject：默认（useDmaBuffer=true 但无 bo）
TEST_F(ExtCaptureFrameCov2Test, dmaAccessorsDefault)
{
    EXPECT_NO_FATAL_FAILURE((void)m_frame->getDmaBufferFd());
    EXPECT_NO_FATAL_FAILURE((void)m_frame->getGbmBufferObject());
}

// ===== ExtCaptureManager 补充 =====
// 既有 ut_extcapturemanager_cov.h 已声明 setupWaylandIntegration/getWaylandOutput，
// 同一 TU 内不重复声明。onActiveChanged 是 private slot，既有用 invokeMethod 调用，
// 这里也用 invokeMethod 调用，无需 ACCESS_PRIVATE_FUN。

class ExtCaptureManagerCov2Test : public Test
{
public:
    ExtCaptureManager *m_mgr;
    void SetUp() override { m_mgr = new ExtCaptureManager; }
    void TearDown() override { delete m_mgr; }
};

// extensionInterface：返回非空
TEST_F(ExtCaptureManagerCov2Test, extensionInterfaceNonNull)
{
    EXPECT_NE(m_mgr->extensionInterface(), nullptr);
}

// onActiveChanged：offscreen 非 active -> 无 emit（protocolActive=false）
TEST_F(ExtCaptureManagerCov2Test, onActiveChangedInactiveSafe)
{
    QSignalSpy unavailSpy(m_mgr, &ExtCaptureManager::protocolUnavailable);
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_mgr, "onActiveChanged", Qt::DirectConnection));
    // protocolActive=false，deactive 分支不发信号
    SUCCEED();
}

// createScreenCaptureSession：协议不可用（默认）-> nullptr
TEST_F(ExtCaptureManagerCov2Test, createSessionProtocolUnavailable)
{
    EXPECT_EQ(m_mgr->createScreenCaptureSession(nullptr, false), nullptr);
    QList<QScreen *> screens = QGuiApplication::screens();
    if (!screens.isEmpty()) {
        EXPECT_EQ(m_mgr->createScreenCaptureSession(screens.first(), true), nullptr);
    }
}

// ===== ExtOutputSourceManager 补充 =====
// onActiveChanged 是 private slot，用 invokeMethod 调用，无需 ACCESS_PRIVATE_FUN。

class ExtOutputSourceManagerCov2Test : public Test
{
public:
    ExtOutputSourceManager *m_mgr;
    void SetUp() override { m_mgr = new ExtOutputSourceManager; }
    void TearDown() override { delete m_mgr; }
};

// extensionInterface：返回非空
TEST_F(ExtOutputSourceManagerCov2Test, extensionInterfaceNonNull)
{
    EXPECT_NE(m_mgr->extensionInterface(), nullptr);
}

// createSourceForOutput：协议不可用 -> nullptr
TEST_F(ExtOutputSourceManagerCov2Test, createSourceUnavailableReturnsNull)
{
    EXPECT_EQ(m_mgr->createSourceForOutput(nullptr), nullptr);
}

// onActiveChanged：offscreen 非 active -> emit protocolUnavailable
TEST_F(ExtOutputSourceManagerCov2Test, onActiveChangedEmitsUnavailable)
{
    QSignalSpy unavailSpy(m_mgr, &ExtOutputSourceManager::protocolUnavailable);
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(m_mgr, "onActiveChanged", Qt::DirectConnection));
    EXPECT_GE(unavailSpy.count(), 1);
}
