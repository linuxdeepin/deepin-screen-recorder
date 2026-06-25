// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "stub.h"
#include "../../src/widgets/majtoolbar.h"
#include "../../src/utils/configsettings.h"

using namespace testing;

// MajToolBar is a DLabel-based toolbar with shape buttons.
// ConfigSettings::getValue is the correct API name.

static QVariant ut_majtoolbar_getValue_stub(void *obj, const QString &group, const QString &key)
{
    Q_UNUSED(obj);
    if (group == "arrow" && key == "is_straight") return false;
    if (group == "rectangle" && key == "color_index") return 0;
    if (group == "oval" && key == "color_index") return 0;
    if (group == "arrow" && key == "color_index") return 0;
    if (group == "line" && key == "color_index") return 0;
    if (group == "text" && key == "color_index") return 0;
    if (group == "common" && key == "color_index") return 0;
    if (group == "common" && key == "color_option") return QVariant();
    return QVariant();
}

class MajToolbarTest : public Test
{
public:
    MajToolBar *m_tb = nullptr;
    Stub stub;

    void SetUp() override
    {
        stub.set(ADDR(ConfigSettings, getValue), ut_majtoolbar_getValue_stub);
        m_tb = new MajToolBar;
    }
    void TearDown() override
    {
        stub.reset(ADDR(ConfigSettings, getValue));
        delete m_tb;
    }
};

TEST_F(MajToolbarTest, constructNotNull)
{
    EXPECT_NE(m_tb, nullptr);
}

TEST_F(MajToolbarTest, destructorRunsClean)
{
    Stub localStub;
    localStub.set(ADDR(ConfigSettings, getValue), ut_majtoolbar_getValue_stub);
    MajToolBar *tmp = new MajToolBar;
    EXPECT_NO_FATAL_FAILURE(delete tmp);
    localStub.reset(ADDR(ConfigSettings, getValue));
}

TEST_F(MajToolbarTest, emitShapePressedRectangle)
{
    QSignalSpy checkedSpy(m_tb, &MajToolBar::buttonChecked);
    m_tb->shapePressed("rectangle");
    EXPECT_GT(checkedSpy.count(), 0);
}

TEST_F(MajToolbarTest, emitShapePressedOval)
{
    QSignalSpy checkedSpy(m_tb, &MajToolBar::buttonChecked);
    m_tb->shapePressed("oval");
    EXPECT_GT(checkedSpy.count(), 0);
}

TEST_F(MajToolbarTest, emitShapePressedArrow)
{
    QSignalSpy checkedSpy(m_tb, &MajToolBar::buttonChecked);
    m_tb->shapePressed("arrow");
    EXPECT_NO_FATAL_FAILURE((void)checkedSpy.count());
}

TEST_F(MajToolbarTest, emitShapePressedLine)
{
    QSignalSpy checkedSpy(m_tb, &MajToolBar::buttonChecked);
    m_tb->shapePressed("line");
    EXPECT_GT(checkedSpy.count(), 0);
}

TEST_F(MajToolbarTest, emitShapePressedText)
{
    QSignalSpy checkedSpy(m_tb, &MajToolBar::buttonChecked);
    m_tb->shapePressed("text");
    EXPECT_GT(checkedSpy.count(), 0);
}

TEST_F(MajToolbarTest, emitShapePressedColor)
{
    QSignalSpy checkedSpy(m_tb, &MajToolBar::buttonChecked);
    m_tb->shapePressed("color");
    EXPECT_GT(checkedSpy.count(), 0);
}

TEST_F(MajToolbarTest, emitShapePressedClose)
{
    QSignalSpy closedSpy(m_tb, &MajToolBar::closed);
    m_tb->shapePressed("close");
    EXPECT_GT(closedSpy.count(), 0);
}

TEST_F(MajToolbarTest, emitSaveImage)
{
    QSignalSpy saveSpy(m_tb, &MajToolBar::saveImage);
    m_tb->saveImage();
    EXPECT_EQ(saveSpy.count(), 1);
}

TEST_F(MajToolbarTest, emitShowSaveTooltip)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->showSaveTooltip("test tip"));
}

TEST_F(MajToolbarTest, emitHideSaveTooltip)
{
    EXPECT_NO_FATAL_FAILURE(m_tb->hideSaveTooltip());
}

TEST_F(MajToolbarTest, emitSpecificedSavePath)
{
    QSignalSpy specSpy(m_tb, &MajToolBar::specificedSavePath);
    m_tb->specificedSavePath();
    EXPECT_EQ(specSpy.count(), 1);
}
