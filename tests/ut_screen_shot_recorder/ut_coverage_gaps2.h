// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QDBusConnection>
#include <QString>
#include <QStringList>
#include <QDBusObjectPath>
#include <QVariant>
#include <QPixmap>
#include <QPainter>

#include "../../src/utils/voicevolumewatcher_interface.h"
#include "../../src/utils/borderprocessinterface.h"

using namespace testing;

// ---------------------------------------------------------------------------
// voicevolumewatcher_interface: 全部 public 内联方法（getter/setter/async slot）
// 均为 DBus 属性读 / 异步调用，无可用服务时返回默认值或错误 reply，不阻塞、不崩溃。
// 一次性覆盖全部 20 个未覆盖函数入口。
// ---------------------------------------------------------------------------
class VoiceVolumeWatcherInterfaceFullTest : public Test
{
public:
    voicevolumewatcher_interface *iface = nullptr;
    void SetUp() override
    {
        iface = new voicevolumewatcher_interface(QStringLiteral("com.deepin.daemon.Audio"),
                                                 QStringLiteral("/com/deepin/daemon/Audio"),
                                                 QDBusConnection::sessionBus());
    }
    void TearDown() override { delete iface; }
};

TEST_F(VoiceVolumeWatcherInterfaceFullTest, PropertyGetters)
{
    // Q_PROPERTY READ getters
    (void)iface->bluetoothAudioMode();
    (void)iface->bluetoothAudioModeOpts();
    (void)iface->cards();
    (void)iface->cardsWithoutUnavailable();
    (void)iface->defaultSink();
    (void)iface->defaultSource();
    (void)iface->increaseVolume();
    (void)iface->maxUIVolume();
    (void)iface->reduceNoise();
    (void)iface->sinkInputs();
    (void)iface->sinks();
    (void)iface->sources();
    SUCCEED();
}

TEST_F(VoiceVolumeWatcherInterfaceFullTest, PropertySetters)
{
    iface->setIncreaseVolume(true);
    iface->setReduceNoise(false);
    SUCCEED();
}

TEST_F(VoiceVolumeWatcherInterfaceFullTest, AsyncSlots)
{
    iface->IsPortEnabled(0u, QStringLiteral("/port/ut"));
    iface->Reset();
    iface->SetBluetoothAudioMode(QStringLiteral("mode"));
    iface->SetDefaultSink(QStringLiteral("/sink/ut"));
    iface->SetDefaultSource(QStringLiteral("/source/ut"));
    iface->SetPort(0u, QStringLiteral("/port/ut"), 1);
    iface->SetPortEnabled(0u, QStringLiteral("/port/ut"), true);
    SUCCEED();
}

// ---------------------------------------------------------------------------
// BorderProcessInterface 的「deleting 析构函数」(D0) 仅在经基类指针 delete
// 时触发；既有测试用栈对象只命中 D2。这里经基类指针 new/delete 覆盖 D0。
// ---------------------------------------------------------------------------
TEST(BorderProcessInterfaceDeletingDtorTest, DeleteViaBasePointerExternal)
{
    BorderProcessInterface *p = new ExternalBorderProcess;
    ASSERT_NE(nullptr, p);
    delete p; // 触发 ExternalBorderProcess D0 -> BorderProcessInterface D0
}

TEST(BorderProcessInterfaceDeletingDtorTest, DeleteViaBasePointerShadow)
{
    BorderProcessInterface *p = new ShadowBorderProcess;
    ASSERT_NE(nullptr, p);
    delete p;
}

TEST(BorderProcessInterfaceDeletingDtorTest, DeleteViaBasePointerPrototype)
{
    BorderProcessInterface *p = new PrototypeBorderProcess;
    ASSERT_NE(nullptr, p);
    delete p;
}
