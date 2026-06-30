// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QMetaObject>
#include "../../src/ext-image-capture/manager/extoutputsourcemanager.h"

using namespace testing;

// 覆盖 extoutputsourcemanager.cpp 未触及的安全分支：
//  - 构造（QWaylandClientExtension 在 offscreen 下安全）
//  - protocolVersion（默认 0）
//  - isProtocolAvailable（offscreen -> false）
//  - createSourceForOutput 守卫（协议不可用 / null output）
//  - onActiveChanged：offscreen 非 active -> emit protocolUnavailable
// 注意：bind 是 protected 非 slot，无法 invokeMethod，故不测。
class ExtOutputSourceManagerCovTest : public Test
{
public:
    ExtOutputSourceManager *m_mgr;
    void SetUp() override { m_mgr = new ExtOutputSourceManager; }
    void TearDown() override { delete m_mgr; }
};

// 构造 + 默认版本
TEST_F(ExtOutputSourceManagerCovTest, constructAndDefaults)
{
    EXPECT_GE(m_mgr->protocolVersion(), 0);
    EXPECT_FALSE(m_mgr->isProtocolAvailable());
}

// createSourceForOutput：协议不可用 -> nullptr
TEST_F(ExtOutputSourceManagerCovTest, createSourceFailsWhenUnavailable)
{
    EXPECT_EQ(m_mgr->createSourceForOutput(nullptr), nullptr);
}

// onActiveChanged：offscreen 下 isActive()==false -> emit protocolUnavailable
TEST_F(ExtOutputSourceManagerCovTest, onActiveChangedEmitsUnavailable)
{
    QSignalSpy unavailSpy(m_mgr, &ExtOutputSourceManager::protocolUnavailable);
    QMetaObject::invokeMethod(m_mgr, "onActiveChanged", Qt::DirectConnection);
    EXPECT_GE(unavailSpy.count(), 1);
}

// 反复析构/构造安全（验证 setParent 与 Private 生命周期）
TEST_F(ExtOutputSourceManagerCovTest, repeatedConstructDestruct)
{
    for (int i = 0; i < 3; ++i) {
        ExtOutputSourceManager *m = new ExtOutputSourceManager;
        EXPECT_FALSE(m->isProtocolAvailable());
        delete m;
    }
}
