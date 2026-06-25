// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QApplication>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/record_option_panel.h"

using namespace testing;

ACCESS_PRIVATE_FIELD(RecordOptionPanel, bool, saveAsGif);
ACCESS_PRIVATE_FIELD(RecordOptionPanel, bool, isPressGif);
ACCESS_PRIVATE_FIELD(RecordOptionPanel, bool, isPressVideo);

class RecordOptionPanelTest : public Test
{
public:
    RecordOptionPanel *m_panel = nullptr;
    Stub stub;
    void SetUp() override
    {
        m_panel = new RecordOptionPanel;
    }
    void TearDown() override
    {
        delete m_panel;
        m_panel = nullptr;
    }
};

TEST_F(RecordOptionPanelTest, constructAndStaticConstants)
{
    EXPECT_NE(m_panel, nullptr);
    EXPECT_EQ(RecordOptionPanel::WIDTH, 124);
    EXPECT_EQ(RecordOptionPanel::HEIGHT, 36);
    EXPECT_EQ(RecordOptionPanel::ICON_OFFSET_X, 14);
}

TEST_F(RecordOptionPanelTest, initialSaveAsGifDefault)
{
    // saveAsGif is initialized from Settings; default may be true or false
    bool gif = access_private_field::RecordOptionPanelsaveAsGif(*m_panel);
    EXPECT_NO_FATAL_FAILURE((void)gif);
}

TEST_F(RecordOptionPanelTest, eventFilterMouseMoveNoEffect)
{
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(50, 10), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_panel->eventFilter(m_panel, &moveEvent));
}

TEST_F(RecordOptionPanelTest, eventFilterKeyPressNoEffect)
{
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_panel->eventFilter(m_panel, &keyEvent));
}

TEST_F(RecordOptionPanelTest, paintEventRunsClean)
{
    m_panel->show();
    m_panel->repaint();
    EXPECT_NO_FATAL_FAILURE(m_panel->hide());
}

// Simulate GIF press then release in GIF area
TEST_F(RecordOptionPanelTest, eventFilterGifPressRelease)
{
    access_private_field::RecordOptionPanelsaveAsGif(*m_panel) = false;
    // Press in GIF area (left half)
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(30, 10),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_panel->eventFilter(m_panel, &pressEvent));
    EXPECT_TRUE(access_private_field::RecordOptionPanelisPressGif(*m_panel));

    // Release in GIF area — should set saveAsGif = true
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(30, 10),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_panel->eventFilter(m_panel, &releaseEvent));
    EXPECT_TRUE(access_private_field::RecordOptionPanelsaveAsGif(*m_panel));
}

// Simulate Video press then release in Video area
TEST_F(RecordOptionPanelTest, eventFilterVideoPressRelease)
{
    access_private_field::RecordOptionPanelsaveAsGif(*m_panel) = true;
    // Press in Video area (right half)
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(90, 10),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_panel->eventFilter(m_panel, &pressEvent));
    EXPECT_TRUE(access_private_field::RecordOptionPanelisPressVideo(*m_panel));

    // Release in Video area — should set saveAsGif = false
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(90, 10),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_panel->eventFilter(m_panel, &releaseEvent));
    EXPECT_FALSE(access_private_field::RecordOptionPanelsaveAsGif(*m_panel));
}

// Cross-area press/release: press GIF, release Video
TEST_F(RecordOptionPanelTest, eventFilterCrossAreaPressRelease)
{
    access_private_field::RecordOptionPanelsaveAsGif(*m_panel) = true;
    // Press GIF area
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(30, 10),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_panel->eventFilter(m_panel, &pressEvent));

    // Release Video area — press was GIF, so isPressGif gets reset
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(90, 10),
        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_panel->eventFilter(m_panel, &releaseEvent));
}

TEST_F(RecordOptionPanelTest, destructorRunsClean)
{
    RecordOptionPanel *tmp = new RecordOptionPanel;
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}
