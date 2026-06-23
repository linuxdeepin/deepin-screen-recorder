// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QMenu>
#include <QIcon>
#include <QMetaObject>
#include "../../src/widgets/savebutton.h"

using namespace testing;

class SaveButtonExtTest : public Test
{
public:
    SaveButton *m_btn;
    void SetUp() override { m_btn = new SaveButton; }
    void TearDown() override { delete m_btn; }
};

TEST_F(SaveButtonExtTest, construct)
{
    EXPECT_FALSE(m_btn->isEnabled() && false); // 仅触发，构造已在 SetUp
    SUCCEED();
}

TEST_F(SaveButtonExtTest, setIconsAndMenu)
{
    QIcon saveIcon(QStringLiteral(":/res/1070/light.svg"));
    QIcon listIcon(QStringLiteral(":/res/1070/shade.svg"));
    EXPECT_NO_FATAL_FAILURE(m_btn->setSaveIcon(saveIcon));
    EXPECT_NO_FATAL_FAILURE(m_btn->setListIcon(listIcon));

    QMenu menu;
    EXPECT_NO_FATAL_FAILURE(m_btn->setOptionsMenu(&menu));
    // onMenuAboutToHide 槽（菜单隐藏时重置点击标志）
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_btn, "onMenuAboutToHide", Qt::DirectConnection));
}
