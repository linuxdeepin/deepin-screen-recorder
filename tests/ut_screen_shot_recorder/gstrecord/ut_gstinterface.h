// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include "../../src/gstrecord/gstinterface.h"

using namespace testing;

TEST(GstInterfaceTest, constructAndLibPath)
{
    gstInterface iface;
    // libPath 返回非空（系统装有 glib/gstreamer）
    QString p = gstInterface::libPath("libglib-2.0.so");
    EXPECT_FALSE(p.isEmpty());
}

TEST(GstInterfaceTest, initAndUnloadFunctions)
{
    // 加载真实 glib/gstreamer/gobject 库并解析符号
    EXPECT_NO_FATAL_FAILURE(gstInterface::initFunctions());
    // 重复调用走 already-init 分支
    EXPECT_NO_FATAL_FAILURE(gstInterface::initFunctions());
    // unload（m_isInitFunction 未被置 true，走空分支，但函数体仍执行）
    EXPECT_NO_FATAL_FAILURE(gstInterface::unloadFunctions());
}
