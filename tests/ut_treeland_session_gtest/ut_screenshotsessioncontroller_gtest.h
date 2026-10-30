// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "screenshot/session/screenshotsessioncontroller.h"
#include "screenshot/core/screenshottypes.h"

#include "ut_testfakes.h"

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCoreApplication>

using namespace Dsr::Screenshot;

// ---------------------------------------------------------------------------
// Helper to build a controller with fakes and bring it to Selecting state
// ---------------------------------------------------------------------------

struct ControllerHarness
{
    FakeBackend *backend = nullptr;
    FakeOverlay *overlay = nullptr;
    FakeResultSink *sink = nullptr;
    FakeSnapHints *snapHints = nullptr;
    FakeWindowTagger *tagger = nullptr;
    FakeAnnotationEditor *editor = nullptr;
    std::unique_ptr<ScreenshotSessionController> controller;

    void setup(const QRect &desktopRect = QRect(0, 0, 100, 100))
    {
        backend = new FakeBackend;
        overlay = new FakeOverlay;
        sink = new FakeResultSink;
        snapHints = nullptr;       // disable window snap for simplicity
        tagger = nullptr;
        editor = new FakeAnnotationEditor;

        FrozenFrameSet frames;
        frames.virtualDesktopRect = desktopRect;
        frames.outputs.append(makeFrame(desktopRect, Qt::blue, 1.0));
        backend->frames = frames;

        controller = std::make_unique<ScreenshotSessionController>(
            backend, overlay, snapHints, tagger, sink, editor);
    }

    bool startAndReachSelecting()
    {
        Request req;
        req.mode = Mode::FullScreen;
        req.enableWindowSnap = false;
        if (!controller->start(req))
            return false;
        if (controller->state() != ScreenshotSessionController::State::Selecting)
            return false;
        return true;
    }
};

// ---------------------------------------------------------------------------
// start()
// ---------------------------------------------------------------------------

TEST(ScreenshotSessionControllerTest, Start_FromIdle_TransitionsToSelecting)
{
    ControllerHarness h;
    h.setup();
    Request req;
    req.enableWindowSnap = false;

    EXPECT_TRUE(h.controller->start(req));
    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Selecting);
    EXPECT_EQ(h.backend->prepareCount, 1);
}

TEST(ScreenshotSessionControllerTest, Start_FromSelecting_ReturnsFalse)
{
    ControllerHarness h;
    h.setup();
    Request req;
    req.enableWindowSnap = false;

    ASSERT_TRUE(h.controller->start(req));
    EXPECT_FALSE(h.controller->start(req));
    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Selecting);
}

TEST(ScreenshotSessionControllerTest, Start_FromCompleted_SucceedsAgain)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    h.sink->saveResult = true;
    h.overlay->confirm();
    // Process events so queued signals are delivered
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    ASSERT_EQ(h.controller->state(), ScreenshotSessionController::State::Completed);

    Request req;
    req.enableWindowSnap = false;
    EXPECT_TRUE(h.controller->start(req));
    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Selecting);
}

// ---------------------------------------------------------------------------
// onBackendPrepared — full happy path
// ---------------------------------------------------------------------------

TEST(ScreenshotSessionControllerTest, BackendPrepared_ReachesSelectingState)
{
    ControllerHarness h;
    h.setup();
    EXPECT_TRUE(h.startAndReachSelecting());
    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Selecting);
}

TEST(ScreenshotSessionControllerTest, BackendPrepared_EmptyFrames_Fails)
{
    ControllerHarness h;
    h.setup();
    h.backend->frames = FrozenFrameSet();   // no outputs, invalid rect

    Request req;
    req.enableWindowSnap = false;
    ASSERT_TRUE(h.controller->start(req));

    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Failed);
}

TEST(ScreenshotSessionControllerTest, BackendPrepared_InvalidVirtualDesktop_Fails)
{
    ControllerHarness h;
    h.setup();

    FrozenFrameSet frames;
    frames.outputs.append(makeFrame(QRect(0, 0, 100, 100), Qt::blue));
    frames.virtualDesktopRect = QRect();   // invalid
    h.backend->frames = frames;

    Request req;
    req.enableWindowSnap = false;
    ASSERT_TRUE(h.controller->start(req));

    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Failed);
}

TEST(ScreenshotSessionControllerTest, BackendPrepared_OverlayCreateFails_Fails)
{
    ControllerHarness h;
    h.setup();

    // Make overlay->create() fail by making outputs empty in the backend
    h.backend->frames.outputs.clear();

    Request req;
    req.enableWindowSnap = false;
    ASSERT_TRUE(h.controller->start(req));

    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Failed);
}

TEST(ScreenshotSessionControllerTest, BackendPrepared_OverlayPrepareFails_Fails)
{
    ControllerHarness h;
    h.setup();
    h.overlay->prepareResult = false;

    Request req;
    req.enableWindowSnap = false;
    ASSERT_TRUE(h.controller->start(req));

    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Failed);
}

// ---------------------------------------------------------------------------
// onConfirmRequested — save flow
// ---------------------------------------------------------------------------

TEST(ScreenshotSessionControllerTest, Confirm_SaveSuccess_CompletedAndSavedSignal)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    QSignalSpy savedSpy(h.controller.get(), &ScreenshotSessionController::saved);
    h.sink->saveResult = true;
    h.overlay->confirm();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Completed);
    EXPECT_EQ(h.sink->saveCount, 1);
    EXPECT_EQ(savedSpy.count(), 1);
}

TEST(ScreenshotSessionControllerTest, Confirm_SaveCancelled_TransitionsToCancelled)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    h.sink->saveCancelled = true;
    h.overlay->confirm();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Cancelled);
}

TEST(ScreenshotSessionControllerTest, Confirm_SaveFailure_TransitionsToFailed)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    h.sink->saveResult = false;
    h.sink->saveError = QStringLiteral("disk full");
    h.overlay->confirm();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Failed);
}

// ---------------------------------------------------------------------------
// onCopyRequested
// ---------------------------------------------------------------------------

TEST(ScreenshotSessionControllerTest, Copy_Success_CompletedAndCopiedSignal)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    QSignalSpy copiedSpy(h.controller.get(), &ScreenshotSessionController::copied);
    h.sink->copyResult = true;
    h.overlay->copy();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Completed);
    EXPECT_EQ(h.sink->copyCount, 1);
    EXPECT_EQ(copiedSpy.count(), 1);
}

TEST(ScreenshotSessionControllerTest, Copy_Failure_TransitionsToFailed)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    h.sink->copyResult = false;
    h.sink->copyError = QStringLiteral("clipboard error");
    h.overlay->copy();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Failed);
}

// ---------------------------------------------------------------------------
// onPinRequested
// ---------------------------------------------------------------------------

TEST(ScreenshotSessionControllerTest, Pin_Success_Completed)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    h.sink->pinResult = true;
    h.overlay->pin();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Completed);
    EXPECT_EQ(h.sink->pinCount, 1);
}

TEST(ScreenshotSessionControllerTest, Pin_Failure_TransitionsToFailed)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    h.sink->pinResult = false;
    h.sink->pinError = QStringLiteral("pin error");
    h.overlay->pin();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Failed);
}

// ---------------------------------------------------------------------------
// onCancelRequested
// ---------------------------------------------------------------------------

TEST(ScreenshotSessionControllerTest, Cancel_FromSelecting_TransitionsToCancelled)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    int cancelBefore = h.backend->cancelCount;
    h.overlay->cancel();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Cancelled);
    EXPECT_GT(h.backend->cancelCount, cancelBefore);
}

TEST(ScreenshotSessionControllerTest, Cancel_ViaControllerMethod_TransitionsToCancelled)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    h.controller->cancel();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Cancelled);
}

TEST(ScreenshotSessionControllerTest, Cancel_AlreadyFinished_NoOp)
{
    ControllerHarness h;
    h.setup();
    ASSERT_TRUE(h.startAndReachSelecting());

    h.overlay->cancel();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    ASSERT_EQ(h.controller->state(), ScreenshotSessionController::State::Cancelled);

    int cancelCount = h.backend->cancelCount;
    h.overlay->cancel();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_EQ(h.controller->state(), ScreenshotSessionController::State::Cancelled);
    EXPECT_EQ(h.backend->cancelCount, cancelCount);
}
