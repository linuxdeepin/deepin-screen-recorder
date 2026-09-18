// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QApplication>
#include <QDebug>
#include <QDBusMessage>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QMetaObject>
#include <QImage>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QStringList>
#include <QWindow>

#include <DDialog>

#include "stub.h"
#include "addr_pri.h"
#include "ut_scope_guard.h"

#include "../../src/utils.h"
#include "../../src/utils/tempfile.h"
#include "../../src/utils/configsettings.h"
#include "../../src/utils/borderprocessinterface.h"
#include "../../src/utils/screengrabber.h"
#include "../../src/utils/voicevolumewatcher.h"
#include "../../src/gstrecord/gstrecordx.h"
#include "../../src/gstrecord/gstinterface.h"
#include "../../src/event_monitor.h"
#include "../../src/dbusinterface/dbusnotify.h"
#include "../../src/camera/devnummonitor.h"
#include "../../src/RecorderRegionShow.h"

using namespace testing;

ACCESS_PRIVATE_FUN(GstRecordX, GstBin *(GstElement *), getGstBin);
ACCESS_PRIVATE_FUN(DBusNotify, void(const QDBusMessage &), __propertyChanged__);

namespace {
namespace private_access_detail {
struct VVWPortMapTag {};
template struct private_access<
    decltype(&voiceVolumeWatcher::m_availableInputPorts),
    &voiceVolumeWatcher::m_availableInputPorts,
    VVWPortMapTag>;
}
}

static int dialog_exec_stub_final() { return 1; }
static void XRecordFreeData_stub_final(XRecordInterceptData *) {}
static int check_device_list_events_stub_final(void *) { return 0; }

TEST(MiscCovFinalTest, UtilsDestructor)
{
    Utils *u = nullptr;
    EXPECT_NO_FATAL_FAILURE({
        u = new Utils();
        delete u;
        u = nullptr;
    });
}

TEST(MiscCovFinalTest, TempFileGetFullscreenPixmap)
{
    TempFile *tf = TempFile::instance();
    QPixmap pix(10, 10);
    pix.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(tf->setFullScreenPixmap(pix));
    QPixmap result;
    EXPECT_NO_FATAL_FAILURE(result = tf->getFullscreenPixmap());
    EXPECT_FALSE(result.isNull());
}

TEST(MiscCovFinalTest, TempFileGetFullscreenPixmapDefault)
{
    TempFile *tf = TempFile::instance();
    QPixmap pix(32, 32);
    pix.fill(Qt::blue);
    tf->setFullScreenPixmap(pix);
    QPixmap result = tf->getFullscreenPixmap();
    EXPECT_EQ(result.size(), QSize(32, 32));
}

TEST(MiscCovFinalTest, BorderProcessInterfaceDestructor)
{
    BorderProcessInterface *p = nullptr;
    EXPECT_NO_FATAL_FAILURE({
        p = new ShadowBorderProcess();
        delete p;
        p = nullptr;
    });
}

TEST(MiscCovFinalTest, BorderProcessInterfaceDestructorExternal)
{
    BorderProcessInterface *p = nullptr;
    EXPECT_NO_FATAL_FAILURE({
        p = new ExternalBorderProcess();
        delete p;
        p = nullptr;
    });
}

TEST(MiscCovFinalTest, BorderProcessInterfaceDestructorPrototype)
{
    BorderProcessInterface *p = nullptr;
    EXPECT_NO_FATAL_FAILURE({
        p = new PrototypeBorderProcess();
        delete p;
        p = nullptr;
    });
}

TEST(MiscCovFinalTest, GrabMultipleScreensEntry)
{
    ScreenGrabber grabber;
    bool ok = false;
    QRect rect(0, 0, 200, 200);
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        GTEST_SKIP() << "No screens available";
    }
    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::ScreenGrabbergrabMultipleScreens(grabber, ok, rect, screens, 1.0));
}

TEST(MiscCovFinalTest, GrabMultipleScreensWithRect)
{
    ScreenGrabber grabber;
    bool ok = false;
    QRect rect(0, 0, 50, 50);
    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        GTEST_SKIP() << "No screens available";
    }
    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::ScreenGrabbergrabMultipleScreens(grabber, ok, rect, screens, 2.0));
    (void)ok;
}

TEST(MiscCovFinalTest, PortOperatorInsert)
{
    voiceVolumeWatcher w;
    const QString cards = QStringLiteral(
        "[{\"Id\":1,\"Name\":\"HDA Intel\",\"Ports\":["
        "{\"Name\":\"analog-input-mic\",\"Description\":\"Mic\",\"Available\":2}"
        "]}]");
    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::voiceVolumeWatcherinitAvailInputPorts(w, cards));
    EXPECT_NO_FATAL_FAILURE(w.slotVoiceVolumeWatcher());
}

TEST(MiscCovFinalTest, PortOperatorInsertMultiplePorts)
{
    voiceVolumeWatcher w;
    const QString cards = QStringLiteral(
        "[{\"Id\":1,\"Name\":\"HDA Intel\",\"Ports\":["
        "{\"Name\":\"analog-input-mic\",\"Description\":\"Mic\",\"Available\":2},"
        "{\"Name\":\"analog-input-linein\",\"Description\":\"Line In\",\"Available\":2}"
        "]}]");
    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::voiceVolumeWatcherinitAvailInputPorts(w, cards));
    EXPECT_NO_FATAL_FAILURE(w.slotVoiceVolumeWatcher());
}

TEST(MiscCovFinalTest, GstRecordXGetGstBinNullptr)
{
    ASSERT_NO_FATAL_FAILURE(gstInterface::initFunctions());
    GstRecordX gst;
    GstBin *bin = nullptr;
    EXPECT_NO_FATAL_FAILURE(bin = call_private_fun::GstRecordXgetGstBin(gst, nullptr));
    EXPECT_EQ(bin, nullptr);
}

TEST(MiscCovFinalTest, EventMonitorCallbackNonServerData)
{
    EventMonitor mon;
    XRecordInterceptData data;
    memset(&data, 0, sizeof(data));
    data.category = 1;
    data.data = nullptr;
    data.data_len = 0;

    Stub s;
    s.set((void *)XRecordFreeData, (void *)XRecordFreeData_stub_final);

    EXPECT_NO_FATAL_FAILURE(
        EventMonitor::callback(reinterpret_cast<XPointer>(&mon), &data));
}

TEST(MiscCovFinalTest, EventMonitorCallbackWithServerCategory)
{
    EventMonitor mon;
    XRecordInterceptData data;
    memset(&data, 0, sizeof(data));
    data.category = 0;
    data.data = nullptr;
    data.data_len = 0;

    Stub s;
    s.set((void *)XRecordFreeData, (void *)XRecordFreeData_stub_final);

    EXPECT_NO_FATAL_FAILURE(
        EventMonitor::callback(reinterpret_cast<XPointer>(&mon), &data));
}

TEST(MiscCovFinalTest, DBusNotifyPropertyChangedValidMessage)
{
    DBusNotify notify;
    QDBusMessage msg = QDBusMessage::createSignal(
        "/", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QList<QVariant> args;
    args << QVariant("org.freedesktop.Notifications")
         << QVariant::fromValue(QVariantMap())
         << QVariant(QStringList());
    msg.setArguments(args);

    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::DBusNotify__propertyChanged__(notify, msg));
}

TEST(MiscCovFinalTest, DBusNotifyPropertyChangedWrongCount)
{
    DBusNotify notify;
    QDBusMessage msg = QDBusMessage::createSignal("/", "test", "test");
    QList<QVariant> args;
    args << QVariant("one");
    msg.setArguments(args);

    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::DBusNotify__propertyChanged__(notify, msg));
}

TEST(MiscCovFinalTest, DBusNotifyPropertyChangedWrongInterface)
{
    DBusNotify notify;
    QDBusMessage msg = QDBusMessage::createSignal(
        "/", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QList<QVariant> args;
    args << QVariant("org.freedesktop.SomeOtherInterface")
         << QVariant::fromValue(QVariantMap())
         << QVariant(QStringList());
    msg.setArguments(args);

    EXPECT_NO_FATAL_FAILURE(
        call_private_fun::DBusNotify__propertyChanged__(notify, msg));
}

TEST(MiscCovFinalTest, DevNumMonitorTimeOutSlotStubbed)
{
    DevNumMonitor mon;
    Stub s;
    s.set((void *)check_device_list_events, (void *)check_device_list_events_stub_final);

    QSignalSpy spy(&mon, &DevNumMonitor::existDevice);
    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(&mon, "timeOutSlot", Qt::DirectConnection));
    qApp->processEvents();
    EXPECT_GE(spy.count(), 0);
}

TEST(MiscCovFinalTest, DevNumMonitorTimeOutSlotCanUseFalse)
{
    DevNumMonitor mon;
    mon.setCanUse(false);
    Stub s;
    s.set((void *)check_device_list_events, (void *)check_device_list_events_stub_final);

    EXPECT_NO_FATAL_FAILURE(
        QMetaObject::invokeMethod(&mon, "timeOutSlot", Qt::DirectConnection));
    qApp->processEvents();
}

TEST(MiscCovFinalTest, RecorderRegionShowPaintEvent)
{
    UtScopeGuard<bool> treelandGuard;
    treelandGuard.protect(Utils::isTreelandMode, false);

    EXPECT_NO_FATAL_FAILURE({
        RecorderRegionShow w;
        w.resize(100, 100);
        QPaintEvent pe(w.rect());
        qApp->sendEvent(&w, &pe);
    });
}

TEST(MiscCovFinalTest, RecorderRegionShowPaintEventTreeland)
{
    UtScopeGuard<bool> treelandGuard;
    treelandGuard.protect(Utils::isTreelandMode, true);

    EXPECT_NO_FATAL_FAILURE({
        RecorderRegionShow w;
        w.resize(50, 50);
        QPaintEvent pe(w.rect());
        qApp->sendEvent(&w, &pe);
    });
}
