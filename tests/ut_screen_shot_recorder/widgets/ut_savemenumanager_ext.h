// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QAction>
#include <QMetaObject>
#include "../../src/widgets/savemenumanager.h"

using namespace testing;

class SaveMenuManagerExtTest : public Test
{
public:
    SaveMenuManager *m_m;
    void SetUp() override { m_m = new SaveMenuManager; }
    void TearDown() override { delete m_m; }
};

TEST_F(SaveMenuManagerExtTest, constructAndGetters)
{
    EXPECT_NE(m_m->getMenu(), nullptr);
    // 默认状态
    EXPECT_EQ(m_m->getCurrentSaveOption(), SaveOption::AskEachTime);
    EXPECT_EQ(m_m->getCurrentLocationState(), LocationState::ChooseOnSave);
    EXPECT_TRUE(m_m->getCurrentCustomPath().isEmpty());
}

TEST_F(SaveMenuManagerExtTest, initializeFromConfig)
{
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
}

TEST_F(SaveMenuManagerExtTest, updateCustomPath)
{
    QSignalSpy optSpy(m_m, &SaveMenuManager::saveOptionChanged);
    QSignalSpy pathSpy(m_m, &SaveMenuManager::customPathChanged);
    EXPECT_NO_FATAL_FAILURE(m_m->updateCustomPath(QStringLiteral("/home/uos/Desktop")));
    EXPECT_FALSE(m_m->getCurrentCustomPath().isEmpty());
    // 桌面路径 -> LocationState::Desktop
    EXPECT_NO_FATAL_FAILURE(m_m->updateCustomPath(QStringLiteral("/home/uos/Pictures")));
    EXPECT_NO_FATAL_FAILURE(m_m->updateCustomPath(QStringLiteral("/some/random/path")));
    EXPECT_GE(pathSpy.count(), 1);
}

TEST_F(SaveMenuManagerExtTest, slotsWithActions)
{
    // 构造动作并触发槽（槽读取 action->data() 判定分支）
    QAction a1, a2;
    a1.setData(static_cast<int>(SaveOption::SpecifiedLocation));
    a2.setData(static_cast<int>(LocationState::Desktop));
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, &a1)));
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onLocationActionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, &a2)));
    // 空数据动作 -> default 分支
    QAction a3;
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onLocationActionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, &a3)));
}
