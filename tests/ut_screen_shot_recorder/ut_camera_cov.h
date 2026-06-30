// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Coverage tests for the camera code paths that are compiled into the test
// build.
//
// Targets:
//   - src/camera/devnummonitor.cpp   (was 0%)
//   - src/utils/camerawatcher.cpp    (was 36%)
//   - src/widgets/camerawidget.cpp   (was 38%)
//
// v4l2 init mechanism (wired up below):
//   The libcam v4l2 device subsystem is NOT initialized by any global ctor.
//   `get_device_list()` returns a pointer to a static `v4l2_device_list_t`
//   whose `num_devices` stays 0 until `v4l2core_init_device_list()` runs.
//   That function creates the udev context, sets up the udev monitor, and
//   calls `enum_v4l2_devices()` which scans /dev/video* (via getUdev()) and,
//   for each node, calls `getV4l2()->m_v4l2_open/ioctl/...` to query caps
//   before adding it to the list. Likewise `get_v4l2_device_handler()` returns
//   the static `my_vd` which stays NULL until `camInit(device)` ->
//   `create_v4l2_device_handler` sets it.
//
//   `getV4l2()`/`getUdev()`/... are lazy singletons that dlopen() the .so
//   whose path is stored in the static `g_ldnames` table. In production
//   `MainWindow::initDynamicLibPath()` populates that table via `setLibNames`
//   (resolving each lib under QLibraryInfo::LibrariesPath). The unit-test
//   build `#ifdef ENABLE_UNIT_TEST`-stubs that function, so by default
//   `g_ldnames` is empty -> dlopen fails -> every `m_v4l2_*` pointer is NULL
//   -> any v4l2 call SEGVs.
//
//   Fix: `CameraV4l2Env::ensureV4l2Ready()` (below) is called once from the
//   first fixture's SetUp. It replicates `MainWindow::libPath`+`setLibNames`
//   (all 10 entries -- `setLibNames` does strlen/strcpy on every field, so all
//   must be non-null) and then calls `v4l2core_init()`. After that
//   `getV4l2()->m_v4l2_open` is non-null, the device list is populated from
//   /dev/video* and `camInit()`/`timeOutSlot()` run against the real camera.

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QMetaObject>
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QDir>
#include <QByteArray>
#include <QLibraryInfo>

extern "C" {
#include "v4l2_devices.h"
#include "v4l2_core.h"
#include "load_libs.h"
}

#include "../../src/camera/devnummonitor.h"
#include "../../src/utils/camerawatcher.h"
#include "../../src/widgets/camerawidget.h"

// One-shot wiring of the libcam v4l2 stack for the whole test process.
// Mirrors MainWindow::initDynamicLibPath() (src/main_window.cpp:982), which is
// #ifdef ENABLE_UNIT_TEST-stubbed and therefore never runs in this binary.
class CameraV4l2Env
{
public:
    // Returns true iff v4l2 was successfully initialized AND at least one
    // /dev/video* device was enumerated (i.e. the deep tests are safe).
    static bool ensureV4l2Ready()
    {
        static int state = -1;  // -1 = not run, 0 = fail, 1 = ok
        if (state != -1) return state == 1;
        state = initOnce() ? 1 : 0;
        return state == 1;
    }

private:
    // Replicates MainWindow::libPath(): glob "<so>*" under the system library
    // path, sort ascending, return the last entry (the real .so, not the bare
    // soname symlink). The QByteArray is the absolute path.
    static QByteArray resolveLib(const char *so)
    {
        QDir dir(QLibraryInfo::location(QLibraryInfo::LibrariesPath));
        QStringList list = dir.entryList(QStringList() << (QString(so) + "*"),
                                         QDir::NoDotAndDotDot | QDir::Files);
        if (list.isEmpty()) return QByteArray();
        list.sort();
        return dir.absoluteFilePath(list.last()).toLocal8Bit();
    }

    static bool initOnce()
    {
        // All 10 fields are mandatory: setLibNames() strlens/copies each one.
        m_avcodec = resolveLib("libavcodec.so");
        m_ff      = resolveLib("libffmpegthumbnailer.so");
        m_swr     = resolveLib("libswresample.so");
        m_sws     = resolveLib("libswscale.so");
        m_avf     = resolveLib("libavformat.so");
        m_avu     = resolveLib("libavutil.so");
        m_udev    = resolveLib("libudev.so");
        m_usb     = resolveLib("libusb-1.0.so");
        m_pa      = resolveLib("libportaudio.so");
        m_v4l2    = resolveLib("libv4l2.so");

        // chV4l2 + chUdev are the load-bearing ones for device enumeration,
        // but every field must be non-null or setLibNames() SEGVs on strlen.
        if (m_v4l2.isEmpty() || m_udev.isEmpty()) return false;

        LoadLibNames names;
        names.chAvcodec = m_avcodec.data();
        names.chFfmpegthumbnailer = m_ff.data();
        names.chSwresample = m_swr.data();
        names.chSwscale = m_sws.data();
        names.chAvformat = m_avf.data();
        names.chAvutil = m_avu.data();
        names.chUdev = m_udev.data();
        names.chUsb = m_usb.data();
        names.chPortaudio = m_pa.data();
        names.chV4l2 = m_v4l2.data();
        setLibNames(names);

        // Populate the static device list (udev context + enum_v4l2_devices).
        v4l2core_init();
        return get_device_list()->num_devices >= 1;
    }

    // Members kept alive for the process: setLibNames strcpys into its own
    // malloc'd storage, but the .data() pointers must be valid across the call.
    static QByteArray m_avcodec, m_ff, m_swr, m_sws, m_avf, m_avu;
    static QByteArray m_udev, m_usb, m_pa, m_v4l2;
};
QByteArray CameraV4l2Env::m_avcodec;
QByteArray CameraV4l2Env::m_ff;
QByteArray CameraV4l2Env::m_swr;
QByteArray CameraV4l2Env::m_sws;
QByteArray CameraV4l2Env::m_avf;
QByteArray CameraV4l2Env::m_avu;
QByteArray CameraV4l2Env::m_udev;
QByteArray CameraV4l2Env::m_usb;
QByteArray CameraV4l2Env::m_pa;
QByteArray CameraV4l2Env::m_v4l2;

// Base fixture: lazily wires the v4l2 stack once for all camera tests.
class CameraCovTest : public ::testing::Test
{
protected:
    void SetUp() override { CameraV4l2Env::ensureV4l2Ready(); }
};

// ============================ DevNumMonitor ================================
// Covers: ctor, init() (timer creation + signal hookup), setWatch (start/stop
// the timer both branches), setCanUse (both values), availableCamera, dtor,
// and timeOutSlot (deep -- runs the real v4l2 device-list check + camInit
// against /dev/video0 once CameraV4l2Env has wired the stack).
class DevNumMonitorCovTest : public CameraCovTest
{
public:
    DevNumMonitor *m{nullptr};
    void SetUp() override
    {
        CameraCovTest::SetUp();  // wires v4l2 once
        EXPECT_NO_FATAL_FAILURE({ m = new DevNumMonitor; });
    }
    void TearDown() override { EXPECT_NO_FATAL_FAILURE({ delete m; }); }
};

TEST_F(DevNumMonitorCovTest, ConstructAndCanUse)
{
    // ctor already ran init(); exercise both setter branches.
    EXPECT_NO_FATAL_FAILURE(m->setCanUse(true));
    EXPECT_NO_FATAL_FAILURE(m->setCanUse(false));
}

TEST_F(DevNumMonitorCovTest, SetWatchStartStop)
{
    // isWatcher=true -> m_pTimer->start(1000); false -> stop().
    EXPECT_NO_FATAL_FAILURE(m->setWatch(true));
    EXPECT_NO_FATAL_FAILURE(m->setWatch(false));
}

TEST_F(DevNumMonitorCovTest, AvailableCameraDefault)
{
    // before any timeOutSlot run, m_availableCamera is the default "".
    QString cam = m->availableCamera();
    EXPECT_TRUE(cam.isNull() || cam.isEmpty());
}

TEST_F(DevNumMonitorCovTest, DestructorIsSafe)
{
    // dtor path: stop + deleteLater the timer. Exercise an explicit
    // construct/destroy cycle so the dtor lines are attributed regardless of
    // fixture-instance reuse.
    EXPECT_NO_FATAL_FAILURE({
        DevNumMonitor *tmp = new DevNumMonitor;
        tmp->setWatch(true);
        delete tmp;
    });
}

// Deep: timeOutSlot with m_canUse=true. Now that CameraV4l2Env has wired the
// v4l2 stack, check_device_list_events() runs against the real udev context
// and the camInit loop probes /dev/video0 (returns 0 -> available). The
// "devices found" + "available camera" branches fire and existDevice(true) is
// emitted. Also leaves m_availableCamera populated.
TEST_F(DevNumMonitorCovTest, TimeOutSlotCanUseTrueEmitsExistDevice)
{
    if (!CameraV4l2Env::ensureV4l2Ready() || get_device_list()->num_devices < 1) {
        GTEST_SKIP() << "no v4l2 camera device available";
    }
    EXPECT_NO_FATAL_FAILURE(m->setCanUse(true));
    QSignalSpy spy(m, &DevNumMonitor::existDevice);

    EXPECT_NO_FATAL_FAILURE({
        // timeOutSlot is a private slot -> invoke via meta system.
        QMetaObject::invokeMethod(m, "timeOutSlot", Qt::DirectConnection);
    });
    // /dev/video0 enumerated + camInit succeeds -> existDevice(true).
    ASSERT_GE(spy.count(), 1);
    EXPECT_TRUE(spy.takeFirst().at(0).toBool());
    // m_availableCamera should now hold a real device node path.
    EXPECT_FALSE(m->availableCamera().isEmpty());
}

// Deep: timeOutSlot with m_canUse=false. Devices exist (>=1) so the
// "num_devices < 1" branch is skipped; the camInit loop is also skipped
// (m_canUse guards it), leaving isExistAvailableCameras==0 but m_canUse false
// -> the "m_canUse && isExistAvailableCameras==0" branch is NOT taken ->
// else branch fires -> existDevice(true).
TEST_F(DevNumMonitorCovTest, TimeOutSlotCanUseFalseEmitsExistDevice)
{
    if (!CameraV4l2Env::ensureV4l2Ready() || get_device_list()->num_devices < 1) {
        GTEST_SKIP() << "no v4l2 camera device available";
    }
    EXPECT_NO_FATAL_FAILURE(m->setCanUse(false));
    QSignalSpy spy(m, &DevNumMonitor::existDevice);

    EXPECT_NO_FATAL_FAILURE({
        QMetaObject::invokeMethod(m, "timeOutSlot", Qt::DirectConnection);
    });
    ASSERT_GE(spy.count(), 1);
    EXPECT_TRUE(spy.takeFirst().at(0).toBool());
}

// ============================ CameraWatcher ================================
class CameraWatcherCovTest : public CameraCovTest
{
public:
    CameraWatcher *w{nullptr};
    void SetUp() override { EXPECT_NO_FATAL_FAILURE({ w = new CameraWatcher; }); }
    void TearDown() override { EXPECT_NO_FATAL_FAILURE({ delete w; }); }
};

TEST_F(CameraWatcherCovTest, ConstructAndSetCoulduse)
{
    EXPECT_NO_FATAL_FAILURE(w->setCoulduseValue(true));
    EXPECT_NO_FATAL_FAILURE(w->setCoulduseValue(false));
}

TEST_F(CameraWatcherCovTest, SetWatchStartStop)
{
    EXPECT_NO_FATAL_FAILURE(w->setWatch(true));
    EXPECT_NO_FATAL_FAILURE(w->setWatch(false));
}

TEST_F(CameraWatcherCovTest, SlotCameraWatcherEmitsStateChange)
{
    // NOTE: main.cpp globally stubs QMediaDevices::videoInputs -> empty list,
    // so couldUse resolves to false inside slotCameraWatcher. m_coulduse
    // defaults to true, so the first call transitions true->false and emits
    // sigCameraState(false). This exercises the "cameras empty" + "state
    // changed -> emit" branches.
    QSignalSpy spy(w, &CameraWatcher::sigCameraState);

    EXPECT_NO_FATAL_FAILURE({
        w->slotCameraWatcher();   // public slot -> call directly
    });
    ASSERT_EQ(spy.count(), 1);
    EXPECT_FALSE(spy.takeFirst().at(0).toBool());
}

TEST_F(CameraWatcherCovTest, SlotCameraWatcherNoChangeNoSignal)
{
    // Pre-set m_coulduse=false (matches the stubbed empty videoInputs result)
    // -> no transition -> no signal. Exercises the "couldUse == m_coulduse"
    // no-emit branch.
    w->setCoulduseValue(false);
    QSignalSpy spy(w, &CameraWatcher::sigCameraState);
    EXPECT_NO_FATAL_FAILURE(w->slotCameraWatcher());
    EXPECT_EQ(spy.count(), 0);
}

// ============================ CameraWidget ================================
// The offscreen-safe event handlers and value setters that ut_camerawidget_ext.h
// does not cover (mouse press/move/release, enter, paint, cameraStop with a
// valid m_cameraUI). The v4l2 capture paths (cameraStart/restartDevices/
// startCameraV4l2) require the v4l2 stack (see file header) and are excluded.
class CameraWidgetCovTest : public CameraCovTest
{
public:
    CameraWidget *cw{nullptr};
    void SetUp() override { EXPECT_NO_FATAL_FAILURE({ cw = new CameraWidget; }); }
    void TearDown() override { EXPECT_NO_FATAL_FAILURE({ delete cw; }); }
};

TEST_F(CameraWidgetCovTest, MousePressReleaseSafe)
{
    cw->resize(40, 40);
    cw->setRecordRect(0, 0, 200, 200);
    EXPECT_NO_FATAL_FAILURE({
        QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(cw, &press);
    });
    EXPECT_NO_FATAL_FAILURE({
        QMouseEvent release(QEvent::MouseButtonRelease, QPointF(5, 5), QPointF(5, 5),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(cw, &release);
    });
}

TEST_F(CameraWidgetCovTest, MouseMoveSafe)
{
    cw->resize(40, 40);
    cw->setRecordRect(0, 0, 200, 200);
    EXPECT_NO_FATAL_FAILURE({
        QMouseEvent move(QEvent::MouseMove, QPointF(10, 10), QPointF(10, 10),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        qApp->sendEvent(cw, &move);
    });
}

TEST_F(CameraWidgetCovTest, EnterEventSafe)
{
    EXPECT_NO_FATAL_FAILURE({
        QEvent e(QEvent::Enter);
        qApp->sendEvent(cw, &e);
    });
}

TEST_F(CameraWidgetCovTest, PaintEventSafe)
{
    cw->resize(48, 48);
    EXPECT_NO_FATAL_FAILURE({
        QPaintEvent pe(cw->rect());
        qApp->sendEvent(cw, &pe);
    });
}

TEST_F(CameraWidgetCovTest, CameraStopAfterInitUI)
{
    // cameraStop dereferences m_cameraUI -> initUI() first so it is non-null,
    // then calls camUnInit() (handler is NULL here -> close is skipped inside
    // camUnInit). m_imgPrcThread is null so its stop() branch is not taken.
    EXPECT_NO_FATAL_FAILURE(cw->initUI());
    EXPECT_NO_FATAL_FAILURE(cw->cameraStop());
}
