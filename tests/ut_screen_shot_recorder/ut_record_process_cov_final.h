// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QImage>
#include <QPoint>
#include <QDBusConnection>
#include "addr_pri.h"
#include "ut_scope_guard.h"
#include "../../src/record_process.h"
#include "../../src/utils.h"
#include "../../src/utils/configsettings.h"
#include "../../src/dbusinterface/pinscreenshotsinterface.h"

using namespace testing;

class RecordProcessCovFinalTest : public Test
{
public:
    RecordProcess *m_p = nullptr;
    UtScopeGuard<bool> m_ffmpegGuard;
    UtScopeGuard<bool> m_waylandGuard;
    UtScopeGuard<bool> m_treelandGuard;

    void SetUp() override
    {
        // RAII guards restore the original global flags on fixture destruction
        m_ffmpegGuard.protect(Utils::isFFmpegEnv, true);
        m_waylandGuard.protect(Utils::isWaylandMode, false);
        m_treelandGuard.protect(Utils::isTreelandMode, false);
        m_p = new RecordProcess;
    }
    void TearDown() override
    {
        delete m_p;
    }
};

TEST_F(RecordProcessCovFinalTest, stopRecordX11PathAfterInitProcess)
{
    call_private_fun::RecordProcessinitProcess(*m_p);
    access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kMP4);
    EXPECT_NO_FATAL_FAILURE(m_p->stopRecord());
}

TEST_F(RecordProcessCovFinalTest, stopRecordGifConnectBranch)
{
    call_private_fun::RecordProcessinitProcess(*m_p);
    access_private_field::RecordProcessm_recordType(*m_p) = static_cast<int>(Utils::kGIF);
    EXPECT_NO_FATAL_FAILURE(m_p->stopRecord());
}

class PinInterfaceCovFinalTest : public Test
{
};

TEST_F(PinInterfaceCovFinalTest, openFileIssuesDbusCallSafely)
{
    PinScreenShotsInterface iface("com.deepin.PinScreenShots",
                                  "/com/deepin/PinScreenShots",
                                  QDBusConnection::sessionBus());
    EXPECT_NO_FATAL_FAILURE(iface.openFile(QString("/tmp/ut_nonexist_final.png")));
}

TEST_F(PinInterfaceCovFinalTest, openImageEncodesAndCallsSafely)
{
    PinScreenShotsInterface iface("com.deepin.PinScreenShots",
                                  "/com/deepin/PinScreenShots",
                                  QDBusConnection::sessionBus());
    QImage img(2, 2, QImage::Format_ARGB32);
    img.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(iface.openImage(img));
}

TEST_F(PinInterfaceCovFinalTest, openImageAndNameAsyncCallSafely)
{
    PinScreenShotsInterface iface("com.deepin.PinScreenShots",
                                  "/com/deepin/PinScreenShots",
                                  QDBusConnection::sessionBus());
    QImage img(2, 2, QImage::Format_ARGB32);
    img.fill(Qt::blue);
    EXPECT_NO_FATAL_FAILURE(iface.openImageAndName(img, QStringLiteral("shot"), QPoint(10, 20)));
}
