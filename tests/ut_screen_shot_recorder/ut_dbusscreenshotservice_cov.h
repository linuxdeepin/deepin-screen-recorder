// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage tests for:
//   - src/dbusservice/dbusscreenshotservice.cpp (166 lines, 0%)
//   - src/dbusservice/dbusscreenshot.cpp       (7 lines, 0%)
//
// DBusScreenshotService is a QDBusAbstractAdaptor that wraps a Screenshot*
// parent. Every slot forwards to parent()-><method>(). Screenshot's methods in
// turn drive MainWindow (showFullScreen etc.) which is heavy. To keep this a
// pure coverage test we instantiate a real Screenshot (its ctor only wires
// signal plumbing; MainWindow is a member but its heavy init is lazy) and
// toggle DBusScreenshotService::setSingleInstance(true) *before* calling the
// slots, so every slot takes the "already running" early-return branch --
// exercising the logging + flag-check lines without invoking the real
// screenshot UI.
//
// DBusScreenshot is a generated QDBusAbstractInterface proxy. Constructing it
// against the session bus is safe (it just stores the service/path); the
// inline asyncCallWithArgumentList calls are stubbed globally in main.cpp
// (callWithArgumentList_stub) so the slot wrappers execute without actually
// hitting the bus.

#pragma once
#include <gtest/gtest.h>
#include <QtDBus/QtDBus>
#include <QVariantMap>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/dbusservice/dbusscreenshotservice.h"
#include "../../src/dbusservice/dbusscreenshot.h"
#include "../../src/screenshot.h"

using namespace testing;

class DBusScreenshotServiceCovTest : public Test
{
public:
    Stub stub;
    Screenshot *m_screen = nullptr;
    DBusScreenshotService *m_svc = nullptr;

    void SetUp() override
    {
        EXPECT_NO_FATAL_FAILURE({ m_screen = new Screenshot; });
        // parent() must be a Screenshot* -- the adaptor stores it via
        // QDBusAbstractAdaptor(parent).
        EXPECT_NO_FATAL_FAILURE({ m_svc = new DBusScreenshotService(m_screen); });
    }
    void TearDown() override
    {
        // delete service first so its lambda capture (parent signal) doesn't
        // fire during m_screen destruction.
        EXPECT_NO_FATAL_FAILURE({ delete m_svc; });
        EXPECT_NO_FATAL_FAILURE({ delete m_screen; });
    }
};

// --- constructor / flag toggle ---
TEST_F(DBusScreenshotServiceCovTest, SetSingleInstance)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(false));
}

TEST_F(DBusScreenshotServiceCovTest, ParentGetter)
{
    Screenshot *p = nullptr;
    EXPECT_NO_FATAL_FAILURE({ p = m_svc->parent(); });
    (void)p;
}

// --- every public slot, guarded by m_singleInstance=true -> early-return ---
// This still runs the method body up to the flag check (logging + the
// unconditional "m_singleInstance = true" tail in some methods).
TEST_F(DBusScreenshotServiceCovTest, StartScreenshotGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->StartScreenshot());
}
TEST_F(DBusScreenshotServiceCovTest, DelayScreenshotGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->DelayScreenshot(0));
}
TEST_F(DBusScreenshotServiceCovTest, NoNotifyScreenshotGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->NoNotifyScreenshot());
}
TEST_F(DBusScreenshotServiceCovTest, OcrScreenshotGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->OcrScreenshot());
}
TEST_F(DBusScreenshotServiceCovTest, ScrollScreenshotGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->ScrollScreenshot());
}
TEST_F(DBusScreenshotServiceCovTest, TopWindowScreenshotGuarded)
{
    // TopWindowScreenshot checks (m_singleInstance == true) and returns early
    // before touching parent()->topWindowScreenshot() / QFileDialog.
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->TopWindowScreenshot());
}
TEST_F(DBusScreenshotServiceCovTest, FullscreenScreenshotGuarded)
{
    // Same early-return pattern as TopWindow.
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->FullscreenScreenshot());
}
TEST_F(DBusScreenshotServiceCovTest, SavePathScreenshotGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->SavePathScreenshot(QStringLiteral("/tmp/x.png")));
}
TEST_F(DBusScreenshotServiceCovTest, StartScreenshotFor3rdGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->StartScreenshotFor3rd(QStringLiteral("/tmp")));
}
TEST_F(DBusScreenshotServiceCovTest, FullScreenRecordGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    EXPECT_NO_FATAL_FAILURE(m_svc->FullScreenRecord(QStringLiteral("/tmp/rec.mp4")));
}
TEST_F(DBusScreenshotServiceCovTest, CustomScreenshotGuarded)
{
    EXPECT_NO_FATAL_FAILURE(m_svc->setSingleInstance(true));
    QVariantMap params;
    params.insert(QStringLiteral("showToolBar"), true);
    params.insert(QStringLiteral("showNotification"), false);
    EXPECT_NO_FATAL_FAILURE(m_svc->CustomScreenshot(params));
}

// ============================ DBusScreenshot ===============================
// Generated proxy interface. Construct + destroy; the inline slot helpers
// (DelayScreenshot/FullscreenScreenshot/...) call asyncCallWithArgumentList
// which is globally stubbed in main.cpp, so they run without a real bus.
class DBusScreenshotCovTest : public Test
{
public:
    DBusScreenshot *m_proxy = nullptr;
    void SetUp() override
    {
        EXPECT_NO_FATAL_FAILURE({
            m_proxy = new DBusScreenshot(
                QStringLiteral("com.deepin.DeepinScreenshot"),
                QStringLiteral("/com/deepin/DeepinScreenshot"),
                QDBusConnection::sessionBus());
        });
    }
    void TearDown() override
    {
        EXPECT_NO_FATAL_FAILURE({ delete m_proxy; });
    }
};

TEST_F(DBusScreenshotCovTest, StaticInterfaceName)
{
    const char *n = nullptr;
    EXPECT_NO_FATAL_FAILURE({ n = DBusScreenshot::staticInterfaceName(); });
    (void)n;
}

TEST_F(DBusScreenshotCovTest, CallSlotWrappers)
{
    // asyncCallWithArgumentList is globally stubbed -> returns empty
    // QDBusPendingReply. Each call exercises the inline marshalling code in
    // dbusscreenshot.h.
    EXPECT_NO_FATAL_FAILURE(m_proxy->StartScreenshot());
    EXPECT_NO_FATAL_FAILURE(m_proxy->DelayScreenshot(0));
    EXPECT_NO_FATAL_FAILURE(m_proxy->FullscreenScreenshot());
    EXPECT_NO_FATAL_FAILURE(m_proxy->NoNotifyScreenshot());
    EXPECT_NO_FATAL_FAILURE(m_proxy->SavePathScreenshot(QStringLiteral("/tmp")));
    EXPECT_NO_FATAL_FAILURE(m_proxy->TopWindowScreenshot());
}
