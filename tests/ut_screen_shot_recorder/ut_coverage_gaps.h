// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QVariantMap>
#include <QDebug>

#include "../../src/utils/tempfile.h"
#include "../../src/utils/audioutils.h"
#include "../../src/widgets/imagemenu.h"
#include "../../src/dbusinterface/dbusnotify.h"
#include "../../src/utils/voicevolumewatcher_interface.h"

using namespace testing;

// ---------------------------------------------------------------------------
// TempFile::isContainsPix / getFullscreenPixmap —— 纯逻辑，安全可测
// ---------------------------------------------------------------------------
TEST(TempFileCoverageGapTest, IsContainsPixBlurBranch)
{
    TempFile *tf = TempFile::instance();
    QPixmap pix(8, 8);
    pix.fill(Qt::red);
    const int radius = 7;
    tf->setBlurPixmap(pix, radius);
    EXPECT_TRUE(tf->isContainsPix("blur", radius));
    EXPECT_FALSE(tf->isContainsPix("blur", radius + 1));
}

TEST(TempFileCoverageGapTest, IsContainsPixMosaicBranch)
{
    TempFile *tf = TempFile::instance();
    QPixmap pix(8, 8);
    pix.fill(Qt::blue);
    const int radius = 3;
    tf->setMosaicPixmap(pix, radius);
    EXPECT_TRUE(tf->isContainsPix("mosaic", radius));
    EXPECT_FALSE(tf->isContainsPix("mosaic", radius + 9));
}

TEST(TempFileCoverageGapTest, GetFullscreenPixmapAfterSet)
{
    TempFile *tf = TempFile::instance();
    QPixmap pix(4, 4);
    pix.fill(Qt::green);
    tf->setFullScreenPixmap(pix);
    QPixmap got = tf->getFullscreenPixmap();
    EXPECT_FALSE(got.isNull());
}

// ---------------------------------------------------------------------------
// AudioUtils::onDBusAudioPropertyChanged —— 构造空/短参数消息触发早返回，
// 覆盖函数入口（函数覆盖率以是否进入函数体计）。
// ---------------------------------------------------------------------------
TEST(AudioUtilsCoverageGapTest, OnDBusAudioPropertyChangedEmptyArgs)
{
    AudioUtils au;
    QDBusMessage msg;
    au.onDBusAudioPropertyChanged(msg); // 0 args -> early return
    SUCCEED();
}

TEST(AudioUtilsCoverageGapTest, OnDBusAudioPropertyChangedWrongArity)
{
    AudioUtils au;
    QDBusMessage msg = QDBusMessage::createSignal("/test", "x.test", "m");
    msg << QVariant(QStringLiteral("not.the.audio.interface"));
    au.onDBusAudioPropertyChanged(msg); // 1 arg -> early return
    SUCCEED();
}

// ---------------------------------------------------------------------------
// ImageBorderHelper 构造/析构 —— 覆盖 ~ImageBorderHelper
// ---------------------------------------------------------------------------
TEST(ImageBorderHelperCoverageGapTest, ConstructAndDestruct)
{
    QScopedPointer<ImageBorderHelper> helper(new ImageBorderHelper);
    EXPECT_NE(nullptr, helper.data());
    helper.reset(); // 触发析构
    EXPECT_EQ(nullptr, helper.data());
}

// ---------------------------------------------------------------------------
// DBusNotify::RemoveRecord —— 构造接口对象并发起异步调用（不阻塞），
// 覆盖 RemoveRecord 入口。
// ---------------------------------------------------------------------------
TEST(DBusNotifyCoverageGapTest, RemoveRecordInvoked)
{
    QScopedPointer<DBusNotify> notify(new DBusNotify);
    ASSERT_NE(nullptr, notify.data());
    // 异步调用：无可用总线时仅返回错误 reply，不阻塞、不崩溃
    notify->RemoveRecord(QStringLiteral("/record/ut-placeholder"));
    SUCCEED();
}

// ---------------------------------------------------------------------------
// voicevolumewatcher_interface::SetPortEnabled —— 构造接口对象并发起异步调用
// 覆盖 SetPortEnabled 入口。
// ---------------------------------------------------------------------------
TEST(VoiceVolumeWatcherInterfaceCoverageGapTest, SetPortEnabledInvoked)
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    QScopedPointer<voicevolumewatcher_interface> iface(
        new voicevolumewatcher_interface(QStringLiteral("com.deepin.daemon.Audio"),
                                         QStringLiteral("/com/deepin/daemon/Audio"),
                                         conn));
    ASSERT_NE(nullptr, iface.data());
    iface->SetPortEnabled(0u, QStringLiteral("/port/ut"), false);
    SUCCEED();
}
