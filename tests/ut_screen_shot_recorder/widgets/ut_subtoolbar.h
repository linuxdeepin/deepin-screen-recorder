// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "stub.h"
#include "../../src/widgets/subtoolbar.h"
#include "../../src/utils/configsettings.h"

using namespace testing;

static QVariant ut_subtoolbar_getValue_stub(void *obj, const QString &group, const QString &key)
{
    Q_UNUSED(obj);
    if (group == "rectangle" && key == "color_index") return 0;
    if (group == "rectangle" && key == "line_width") return 2;
    if (group == "oval" && key == "color_index") return 0;
    if (group == "arrow" && key == "color_index") return 0;
    if (group == "arrow" && key == "line_width") return 2;
    if (group == "arrow" && key == "is_straight") return false;
    if (group == "line" && key == "color_index") return 0;
    if (group == "line" && key == "line_width") return 2;
    if (group == "text" && key == "color_index") return 0;
    if (group == "text" && key == "fontsize") return 16;
    if (group == "common" && key == "color_index") return 0;
    return QVariant();
}

class SubToolbarTest : public Test
{
public:
    SubToolBar *m_tb = nullptr;
    Stub stub;

    void SetUp() override
    {
        stub.set(ADDR(ConfigSettings, getValue), ut_subtoolbar_getValue_stub);
        m_tb = new SubToolBar;
    }
    void TearDown() override
    {
        stub.reset(ADDR(ConfigSettings, getValue));
        delete m_tb;
    }
};

TEST_F(SubToolbarTest, constructNotNull)
{
    EXPECT_NE(m_tb, nullptr);
}

TEST_F(SubToolbarTest, destructorRunsClean)
{
    Stub localStub;
    localStub.set(ADDR(ConfigSettings, getValue), ut_subtoolbar_getValue_stub);
    SubToolBar *tmp = new SubToolBar;
    EXPECT_NO_FATAL_FAILURE(delete tmp);
    localStub.reset(ADDR(ConfigSettings, getValue));
}

TEST_F(SubToolbarTest, switchContentRect)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->switchContent("rectangle"));
}

TEST_F(SubToolbarTest, switchContentArrow)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->switchContent("arrow"));
}

TEST_F(SubToolbarTest, switchContentLine)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->switchContent("line"));
}

TEST_F(SubToolbarTest, switchContentText)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->switchContent("text"));
}

TEST_F(SubToolbarTest, switchContentColor)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->switchContent("color"));
}

TEST_F(SubToolbarTest, switchContentSave)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->switchContent("save"));
}

TEST_F(SubToolbarTest, updateColor)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->updateColor(QColor(255, 0, 0)));
}

TEST_F(SubToolbarTest, setSaveQualityIndex)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->setSaveQualityIndex(0));
    EXPECT_NO_FATAL_FAILURE(m_tb->setSaveQualityIndex(1));
}

TEST_F(SubToolbarTest, emitShapeChanged)
{
    QSignalSpy spy(m_tb, &SubToolBar::shapeChanged);
    m_tb->shapeChanged();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(SubToolbarTest, emitSaveAction)
{
    QSignalSpy spy(m_tb, &SubToolBar::saveAction);
    m_tb->saveAction();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(SubToolbarTest, emitShowSaveTip)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->showSaveTip("test"));
}

TEST_F(SubToolbarTest, emitHideSaveTip)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->hideSaveTip());
}
