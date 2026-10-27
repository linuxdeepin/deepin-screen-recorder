// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "ut_annotationcoordinator.h"
#include "../../../src/screenshot/session/screenshotsessioncontroller.h"
#include "../../../src/screenshot/ports/iscreenshotbackend.h"
#include "../../../src/screenshot/ports/iscreenshotoverlayhost.h"
#include "../../../src/screenshot/ports/iscreenshotresultsink.h"
#include "../../../src/screenshot/ports/isnaphintsource.h"
#include "../../../src/screenshot/ports/iwindowtagger.h"

using namespace Dsr::Screenshot;
using namespace testing;

class MockScreenshotBackend : public IScreenshotBackend
{
public:
    explicit MockScreenshotBackend(QObject *parent = nullptr)
        : IScreenshotBackend(parent) {}

    int prepareCallCount = 0;
    int cancelCallCount = 0;
    Request lastRequest;

    BackendCapabilities capabilities() const override
    {
        BackendCapabilities caps;
        caps.frozenFrame = true;
        return caps;
    }
    QList<OutputDescriptor> outputs() const override { return {}; }
    void prepare(const Request &request) override { ++prepareCallCount; lastRequest = request; }
    void cancel() override { ++cancelCallCount; }

    void emitPrepared(const FrozenFrameSet &frames) { emit prepared(frames); }
    void emitFailed(const QString &reason) { emit failed(reason); }
};

class MockOverlayHost : public IScreenshotOverlayHost
{
public:
    explicit MockOverlayHost(QObject *parent = nullptr)
        : IScreenshotOverlayHost(parent) {}

    bool createReturn = true;
    int createCallCount = 0;
    int showCallCount = 0;
    int closeCallCount = 0;
    QRect lastSelection;

    bool create(const QList<OutputDescriptor> &) override { ++createCallCount; return createReturn; }
    QList<QWindow *> maskWindows() const override { return {}; }
    void setFrozenFrames(const FrozenFrameSet &) override {}
    void show() override { ++showCallCount; }
    void setSelection(const QRect &globalSelection) override { lastSelection = globalSelection; }
    void setSnapHint(const QRect &) override {}
    void setSelectionFinalized(bool) override {}
    void setCursorIntent(CursorIntent) override {}
    void close() override { ++closeCallCount; }
};

class MockResultSink : public IScreenshotResultSink
{
public:
    explicit MockResultSink(QObject *parent = nullptr)
        : IScreenshotResultSink(parent) {}

    bool saveReturn = true;
    bool copyReturn = true;
    QString savePath = "/tmp/test_screenshot.png";

    bool save(const Result &, const QString &, QString *savedPath, QString *) override
    {
        if (saveReturn && savedPath)
            *savedPath = savePath;
        return saveReturn;
    }
    bool copy(const Result &, QString *) override { return copyReturn; }
};

class MockSnapHintSource : public ISnapHintSource
{
public:
    explicit MockSnapHintSource(QObject *parent = nullptr)
        : ISnapHintSource(parent) {}

    bool startReturn = true;
    int startCallCount = 0;
    int stopCallCount = 0;

    bool start(QString *) override { ++startCallCount; return startReturn; }
    void stop() override { ++stopCallCount; }
    QRect currentRegion() const override { return {}; }
};

class MockWindowTagger : public IWindowTagger
{
public:
    explicit MockWindowTagger(QObject *parent = nullptr)
        : IWindowTagger(parent) {}

    bool tagReturn = true;
    bool tagMaskWindow(QWindow *, QString *) override { return tagReturn; }
};

class ScreenshotSessionControllerTest : public Test
{
protected:
    MockScreenshotBackend *backend = nullptr;
    MockOverlayHost *overlay = nullptr;
    MockResultSink *resultSink = nullptr;
    MockSnapHintSource *snapHints = nullptr;
    MockWindowTagger *windowTagger = nullptr;
    MockAnnotationEditor *annotationEditor = nullptr;
    ScreenshotSessionController *controller = nullptr;

    void SetUp() override
    {
        backend = new MockScreenshotBackend;
        overlay = new MockOverlayHost;
        resultSink = new MockResultSink;
        snapHints = new MockSnapHintSource;
        windowTagger = new MockWindowTagger;
        annotationEditor = new MockAnnotationEditor;
        controller = new ScreenshotSessionController(
            backend, overlay, snapHints, windowTagger, resultSink, annotationEditor);
    }

    void TearDown() override
    {
        delete controller;
    }

    FrozenFrameSet makeValidFrames()
    {
        FrozenFrameSet frames;
        frames.generation = 1;
        frames.virtualDesktopRect = QRect(0, 0, 1920, 1080);
        OutputFrame frame;
        frame.output.outputId = QStringLiteral("output1");
        frame.output.logicalGeometry = QRect(0, 0, 1920, 1080);
        frame.image = QImage(1920, 1080, QImage::Format_ARGB32);
        frame.pixelScale = 1.0;
        frames.outputs.append(frame);
        return frames;
    }

    Request makeRequest(Mode mode = Mode::Region)
    {
        Request req;
        req.mode = mode;
        return req;
    }
};

TEST_F(ScreenshotSessionControllerTest, ConstructorSetsIdleState)
{
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Idle);
}

TEST_F(ScreenshotSessionControllerTest, StartTransitionsToPreparingFrame)
{
    Request req = makeRequest();
    EXPECT_TRUE(controller->start(req));
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::PreparingFrame);
    EXPECT_EQ(backend->prepareCallCount, 1);
    EXPECT_EQ(backend->lastRequest.mode, req.mode);
}

TEST_F(ScreenshotSessionControllerTest, StartReturnsFalseWhenAlreadyPreparing)
{
    Request req = makeRequest();
    ASSERT_TRUE(controller->start(req));
    EXPECT_FALSE(controller->start(req));
    EXPECT_EQ(backend->prepareCallCount, 1);
}

TEST_F(ScreenshotSessionControllerTest, CancelCallsBackendCancelAndSetsCancelled)
{
    Request req = makeRequest();
    ASSERT_TRUE(controller->start(req));
    controller->cancel();
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Cancelled);
    EXPECT_EQ(backend->cancelCallCount, 1);
}

TEST_F(ScreenshotSessionControllerTest, CancelFromIdleStillCancelsBackend)
{
    controller->cancel();
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Cancelled);
    EXPECT_EQ(backend->cancelCallCount, 1);
}

TEST_F(ScreenshotSessionControllerTest, BackendPreparedEmptyFramesFails)
{
    ASSERT_TRUE(controller->start(makeRequest()));
    FrozenFrameSet emptyFrames;
    backend->emitPrepared(emptyFrames);
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Failed);
    EXPECT_GE(backend->cancelCallCount, 1);
}

TEST_F(ScreenshotSessionControllerTest, BackendPreparedInvalidDesktopFails)
{
    ASSERT_TRUE(controller->start(makeRequest()));
    FrozenFrameSet frames;
    frames.virtualDesktopRect = QRect();
    OutputFrame frame;
    frame.image = QImage(100, 100, QImage::Format_ARGB32);
    frames.outputs.append(frame);
    backend->emitPrepared(frames);
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Failed);
}

TEST_F(ScreenshotSessionControllerTest, BackendPreparedValidFramesCreatesOverlay)
{
    ASSERT_TRUE(controller->start(makeRequest()));
    FrozenFrameSet frames = makeValidFrames();
    backend->emitPrepared(frames);
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::CreatingMasks);
    EXPECT_EQ(overlay->createCallCount, 1);
    EXPECT_EQ(overlay->showCallCount, 1);
}

TEST_F(ScreenshotSessionControllerTest, BackendPreparedFullScreenModeSetsSelection)
{
    ASSERT_TRUE(controller->start(makeRequest(Mode::FullScreen)));
    FrozenFrameSet frames = makeValidFrames();
    backend->emitPrepared(frames);
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::CreatingMasks);
    EXPECT_EQ(overlay->lastSelection, frames.virtualDesktopRect);
}

TEST_F(ScreenshotSessionControllerTest, BackendFailedTransitionsToFailed)
{
    ASSERT_TRUE(controller->start(makeRequest()));
    backend->emitFailed(QStringLiteral("backend error"));
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Failed);
    EXPECT_GE(backend->cancelCallCount, 1);
}

TEST_F(ScreenshotSessionControllerTest, FailedEmitsFailedSignal)
{
    QSignalSpy failedSpy(controller, &ScreenshotSessionController::failed);
    ASSERT_TRUE(controller->start(makeRequest()));
    backend->emitFailed(QStringLiteral("backend error"));
    EXPECT_EQ(failedSpy.count(), 1);
}

TEST_F(ScreenshotSessionControllerTest, CancelEmitsFinishedSignal)
{
    QSignalSpy finishedSpy(controller, &ScreenshotSessionController::finished);
    ASSERT_TRUE(controller->start(makeRequest()));
    controller->cancel();
    EXPECT_EQ(finishedSpy.count(), 1);
}

TEST_F(ScreenshotSessionControllerTest, BackendPreparedIgnoredWhenNotPreparing)
{
    FrozenFrameSet frames = makeValidFrames();
    backend->emitPrepared(frames);
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Idle);
    EXPECT_EQ(overlay->createCallCount, 0);
}

TEST_F(ScreenshotSessionControllerTest, BackendFailedIgnoredWhenNotPreparing)
{
    // onBackendFailed calls fail() unconditionally (only guards on m_finished),
    // so even from Idle state it transitions to Failed and emits the failed signal.
    backend->emitFailed(QStringLiteral("ignored"));
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Failed);
}

TEST_F(ScreenshotSessionControllerTest, StartAfterCancelSucceeds)
{
    ASSERT_TRUE(controller->start(makeRequest()));
    controller->cancel();
    EXPECT_TRUE(controller->start(makeRequest()));
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::PreparingFrame);
}

TEST_F(ScreenshotSessionControllerTest, OverlayCreateFailureTransitionsToFailed)
{
    ASSERT_TRUE(controller->start(makeRequest()));
    overlay->createReturn = false;
    backend->emitPrepared(makeValidFrames());
    EXPECT_EQ(controller->state(), ScreenshotSessionController::State::Failed);
}
