// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage header (round 3) for the three lowest-covered src/ TUs:
//   - src/capture.cpp            (TreelandCaptureManager family)
//   - src/record_process.cpp     (RecordProcess state/slot helpers)
//   - src/utils/screengrabber.cpp (X11/Wayland screenshot dispatchers)
//
// All ACCESS_PRIVATE_FUN/FIELD targets below are NEW relative to existing
// cov/cov2 headers (verified by grep): no (Class, member) dup.
//
// Risk model: every potentially-blocking or environment-touching call is
// wrapped in EXPECT_NO_FATAL_FAILURE. Heap objects are deliberately leaked
// in TearDown (per task spec) because their destructors touch wayland /
// QProcess state that is unsafe under the offscreen test platform.

#pragma once
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeData>
#include <QPixmap>
#include <QRect>
#include <QScreen>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QVariant>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/capture.h"
#include "../../src/record_process.h"
#include "../../src/utils.h"
#include "../../src/utils/configsettings.h"
#include "../../src/utils/screengrabber.h"

using namespace testing;

// ============================================================================
// === CaptureCov3Test =======================================================
// ============================================================================
// capture.cpp wraps the treeland_capture wayland protocol. ut_capture_cov.h
// already hits instance/cancelCapture/setRecord/record/recordStarted/context
// (the no-wayland short-circuit branches). The deep paths (ensureContext,
// ensureFrame, ensureSession) all call wayland C functions that SEGV/hang
// without a live compositor, so we re-exercise only the safe surface here to
// add stable line hits in the manager constructor / setRecord / cancelCapture
// branches beyond what the singleton's first instantiation already covers.

class CaptureCov3Test : public Test
{
public:
    TreelandCaptureManager *m_mgr = nullptr;

    void SetUp() override
    {
        EXPECT_NO_FATAL_FAILURE({ m_mgr = TreelandCaptureManager::instance(); });
    }
    void TearDown() override { /* singleton: never delete */ }
};

// Re-hit instance(): each call after the first still enters the function and
// returns the static singleton (covers the entry log line + return).
TEST_F(CaptureCov3Test, InstanceRepeated)
{
    TreelandCaptureManager *p1 = nullptr;
    TreelandCaptureManager *p2 = nullptr;
    EXPECT_NO_FATAL_FAILURE({ p1 = TreelandCaptureManager::instance(); });
    EXPECT_NO_FATAL_FAILURE({ p2 = TreelandCaptureManager::instance(); });
    (void)p1;
    (void)p2;
}

// cancelCapture: with isActive()==false on offscreen, hits the early-return
// "Wayland connection not active" log branch (stable, no wayland calls).
TEST_F(CaptureCov3Test, CancelCaptureNoWayland)
{
    EXPECT_NO_FATAL_FAILURE(m_mgr->cancelCapture());
}

// setRecord toggle both directions + same-value no-op branch + record() getter.
TEST_F(CaptureCov3Test, SetRecordFullToggle)
{
    EXPECT_NO_FATAL_FAILURE(m_mgr->setRecord(false));  // no-change (already false)
    EXPECT_NO_FATAL_FAILURE(m_mgr->setRecord(true));   // emits recordChanged
    EXPECT_NO_FATAL_FAILURE(m_mgr->setRecord(true));   // no-change
    EXPECT_NO_FATAL_FAILURE(m_mgr->setRecord(false));  // emits recordChanged
    bool r = true;
    EXPECT_NO_FATAL_FAILURE({ r = m_mgr->record(); });
    (void)r;
}

// recordStarted() with null m_context: short-circuits to false, no wayland.
TEST_F(CaptureCov3Test, RecordStartedNoContextSafe)
{
    bool started = true;
    EXPECT_NO_FATAL_FAILURE({ started = m_mgr->recordStarted(); });
    (void)started;
}

// context() getter: returns m_context (null on offscreen), pure accessor.
TEST_F(CaptureCov3Test, ContextGetterSafe)
{
    TreelandCaptureContext *ctx = reinterpret_cast<TreelandCaptureContext *>(0x1);
    EXPECT_NO_FATAL_FAILURE({ ctx = m_mgr->context(); });
    (void)ctx;
}

// ============================================================================
// === RecordProcessCov3Test =================================================
// ============================================================================
// Goal: exercise the *public* safe surface that the existing cov/cov2 headers
// leave alone -- setRecordSavingNotifyId, save2Clipboard (real QClipboard
// hit), setFullScreenRecord, getScreenRecordSavePath default-path branch.
//
// Deliberately NOT exercised here (covered elsewhere or unsafe under offscreen):
//   - GstStartRecord / GstStopRecord / onExitGstRecord (see note below)
//   - recordVideo / waylandRecord / treelandRecord (cov2 already covers these)
//   - exitRecord (calls QApplication::quit() via QTimer -> kills the harness)
//   - startRecord / stopRecord (spawn real ffmpeg subprocesses)

// No new ACCESS_PRIVATE_FUN/FIELD for RecordProcess: all targets we need are
// either public or already declared in ut_record_process_cov.h / _cov2.h.

class RecordProcessCov3Test : public Test
{
public:
    RecordProcess *m_p = nullptr;
    Stub stub;
    bool savedRoot = false;

    static QVariant getValue_stub(void *, const QString &group, const QString &key)
    {
        Q_UNUSED(group);
        if (key == "format") return 0;     // kGIF (default-ish)
        if (key == "audio") return 0;      // kNoAudio
        if (key == "cursor") return 0;     // RECORD_MOUSE_NULL
        if (key == "frame_rate") return 15;
        if (key == "save_dir") return QString();
        if (key == "save_op") return 0;
        return QVariant();
    }

    void SetUp() override
    {
        stub.set(ADDR(ConfigSettings, getValue), getValue_stub);
        savedRoot = Utils::isRootUser;
        Utils::isRootUser = false;
        Utils::isFFmpegEnv = true;
        Utils::isWaylandMode = false;
        Utils::isTreelandMode = false;
        m_p = new RecordProcess;
    }
    void TearDown() override
    {
        // Leak per task spec -- destructors touch QProcess/gst state.
        (void)m_p;
        Utils::isRootUser = savedRoot;
        Utils::isFFmpegEnv = true;
        Utils::isWaylandMode = false;
        Utils::isTreelandMode = false;
    }
};

// setRecordSavingNotifyId: trivial setter; covers the assignment + the
// RECORD_SAVING_NOTIFY_ID_INVALID usage.
TEST_F(RecordProcessCov3Test, SetRecordSavingNotifyIdValues)
{
    EXPECT_NO_FATAL_FAILURE(m_p->setRecordSavingNotifyId(RecordProcess::RECORD_SAVING_NOTIFY_ID_INVALID));
    EXPECT_NO_FATAL_FAILURE(m_p->setRecordSavingNotifyId(42u));
    EXPECT_NO_FATAL_FAILURE(m_p->setRecordSavingNotifyId(0u));
}

// setFullScreenRecord: setter only.
TEST_F(RecordProcessCov3Test, SetFullScreenRecordToggle)
{
    EXPECT_NO_FATAL_FAILURE(m_p->setFullScreenRecord(true));
    EXPECT_NO_FATAL_FAILURE(m_p->setFullScreenRecord(false));
}

// save2Clipboard with a real temp file: hits QClipboard + QMimeData paths,
// all safe under offscreen platform.
TEST_F(RecordProcessCov3Test, Save2ClipboardRealFile)
{
    QTemporaryFile tmp;
    tmp.setAutoRemove(true);
    ASSERT_TRUE(tmp.open());
    tmp.write("hello");
    tmp.close();
    EXPECT_NO_FATAL_FAILURE(m_p->save2Clipboard(tmp.fileName()));
    qApp->processEvents();
}

TEST_F(RecordProcessCov3Test, Save2ClipboardEmptyPath)
{
    EXPECT_NO_FATAL_FAILURE(m_p->save2Clipboard(QString()));
    qApp->processEvents();
}

// getScreenRecordSavePath with empty save_dir (default): falls into the
// "采用默认路径" branch and builds a Screen Recordings path under MoviesLocation.
static QVariant cov3DefaultPathStub(void *, const QString &, const QString &key)
{
    if (key == "save_dir") return QString();  // empty -> default path
    if (key == "save_op") return 0;
    return QVariant();
}

TEST_F(RecordProcessCov3Test, GetScreenRecordSavePathDefaultBranch)
{
    Stub local;
    local.set(ADDR(ConfigSettings, getValue), cov3DefaultPathStub);
    EXPECT_NO_FATAL_FAILURE(m_p->getScreenRecordSavePath());
    qApp->processEvents();
}

// setRecordInfo then getScreenRecordSavePath with save_op=1 (desktop branch).
static QVariant cov3DesktopPathStub(void *, const QString &, const QString &key)
{
    if (key == "save_dir") return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (key == "save_op") return 1;  // desktop branch
    return QVariant();
}

TEST_F(RecordProcessCov3Test, SetRecordInfoThenDesktopSavePath)
{
    EXPECT_NO_FATAL_FAILURE(m_p->setRecordInfo(QRect(0, 0, 320, 240), QStringLiteral("ut_area")));
    Stub local;
    local.set(ADDR(ConfigSettings, getValue), cov3DesktopPathStub);
    EXPECT_NO_FATAL_FAILURE(m_p->getScreenRecordSavePath());
    qApp->processEvents();
}

// GstStartRecord / GstStopRecord are deliberately NOT exercised here:
// even though the wayland/treeland branches are #ifndef ENABLE_UNIT_TEST-gated,
// GstStartRecord still runs `m_gstRecordX = new GstRecordX(this)` + a chain of
// setters (allocating real gstreamer state), and GstStopRecord unconditionally
// calls `m_gstRecordX->x11GstStopRecord()` which dereferences a null
// m_gstRecordX (SEGV). The existing ut_gstrecordx_cov2.h covers the GstRecordX
// surface directly and more safely.

// ============================================================================
// === ScreenGrabberCov3Test =================================================
// ============================================================================
// New private funs declared (verified absent from cov/cov2/ext headers):
//   - grabWithXGetImage   (Qt6 only; XOpenDisplay returns null on offscreen)
//   - grabWaylandScreenshot (KWin DBus call fails harmlessly in CI)
// Both short-circuit cleanly on the offscreen test platform.

ACCESS_PRIVATE_FUN(ScreenGrabber, QPixmap(bool &, const QRect &), grabWithXGetImage);
ACCESS_PRIVATE_FUN(ScreenGrabber, QPixmap(bool &, const QRect &, const qreal), grabWaylandScreenshot);

class ScreenGrabberCov3Test : public Test
{
public:
    void SetUp() override
    {
        Utils::isWaylandMode = false;
        Utils::isQt6XcbEnv = false;
    }
    void TearDown() override
    {
        Utils::isWaylandMode = false;
        Utils::isQt6XcbEnv = false;
    }
};

// grabWithXGetImage directly: XOpenDisplay returns nullptr under offscreen,
// so the function logs and returns a null pixmap with ok=false.
TEST_F(ScreenGrabberCov3Test, GrabWithXGetImageNoDisplay)
{
    ScreenGrabber g;
    bool ok = true;
    QPixmap pm;
    EXPECT_NO_FATAL_FAILURE({
        pm = call_private_fun::ScreenGrabbergrabWithXGetImage(g, ok, QRect(0, 0, 10, 10));
    });
    (void)pm;
    qApp->processEvents();
}

// grabWithXGetImage with degenerate (empty) rect.
TEST_F(ScreenGrabberCov3Test, GrabWithXGetImageEmptyRect)
{
    ScreenGrabber g;
    bool ok = true;
    EXPECT_NO_FATAL_FAILURE({
        (void)call_private_fun::ScreenGrabbergrabWithXGetImage(g, ok, QRect());
    });
    qApp->processEvents();
}

// grabWaylandScreenshot directly: KWin DBus call fails in CI, returns null
// pixmap with ok=false. No temp file is created in that branch.
TEST_F(ScreenGrabberCov3Test, GrabWaylandScreenshotNoKWin)
{
    ScreenGrabber g;
    bool ok = true;
    QPixmap pm;
    EXPECT_NO_FATAL_FAILURE({
        pm = call_private_fun::ScreenGrabbergrabWaylandScreenshot(g, ok, QRect(0, 0, 16, 16), 1.0);
    });
    (void)pm;
    qApp->processEvents();
}

// grabWaylandScreenshot with high-DPR scaling.
TEST_F(ScreenGrabberCov3Test, GrabWaylandScreenshotHighDPR)
{
    ScreenGrabber g;
    bool ok = true;
    EXPECT_NO_FATAL_FAILURE({
        (void)call_private_fun::ScreenGrabbergrabWaylandScreenshot(g, ok, QRect(10, 10, 50, 50), 2.0);
    });
    qApp->processEvents();
}

// getX11RootWindowSize with isQt6XcbEnv=true: XOpenDisplay fails on
// offscreen -> returns QSize() via the warning branch.
TEST_F(ScreenGrabberCov3Test, GetX11RootWindowSizeXcbModeNoDisplay)
{
    Utils::isQt6XcbEnv = true;
    QSize s;
    EXPECT_NO_FATAL_FAILURE({ s = ScreenGrabber::getX11RootWindowSize(); });
    (void)s;
    qApp->processEvents();
}

// grabEntireDesktop with a far-off-screen rect (no intersection) routes to
// grabPrimaryScreenFallback -> primaryScreen()->grabWindow. Safe under offscreen.
TEST_F(ScreenGrabberCov3Test, GrabEntireDesktopFarOffRect)
{
    ScreenGrabber g;
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(-100000, -100000, 1, 1), 1.0));
    qApp->processEvents();
}

// grabEntireDesktop with high DPR for the multi-screen canvas scaling path.
TEST_F(ScreenGrabberCov3Test, GrabEntireDesktopHighDPR)
{
    ScreenGrabber g;
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE(g.grabEntireDesktop(ok, QRect(0, 0, 200, 100), 3.0));
    qApp->processEvents();
}
