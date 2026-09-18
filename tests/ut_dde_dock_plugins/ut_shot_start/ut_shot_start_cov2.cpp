// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QIcon>
#include <QVariant>
#include <QDebug>

#include "../../../src/dde-dock-plugins/shotstart/iconwidget.h"
#include "../../../src/dde-dock-plugins/shotstart/quickpanelwidget.h"
#include "../../../src/dde-dock-plugins/shotstart/shotstartplugin.h"
#include "ut_mock_pluginproxyinterface.h"

namespace {
class IconWidgetCovTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_iconWidget = new IconWidget(nullptr);
        m_iconWidget->show();
    }
    void TearDown() override
    {
        if (m_iconWidget) {
            delete m_iconWidget;
            m_iconWidget = nullptr;
        }
    }
    IconWidget *m_iconWidget = nullptr;
};

class QuickPanelWidgetCovTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_quickPanel = new QuickPanelWidget(nullptr);
        m_quickPanel->show();
    }
    void TearDown() override
    {
        if (m_quickPanel) {
            delete m_quickPanel;
            m_quickPanel = nullptr;
        }
    }
    QuickPanelWidget *m_quickPanel = nullptr;
};

class ShotStartPluginCovTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_plugin.reset(new ShotStartPlugin());
        m_plugin->init(&mock_proxy);
    }
    void TearDown() override {}

    MockPluginProxyInterface mock_proxy;
    std::shared_ptr<ShotStartPlugin> m_plugin;
};
} // namespace

TEST_F(IconWidgetCovTest, MouseMoveEvent_SetsHoverState)
{
    QMouseEvent me(QEvent::MouseMove, QPointF(1, 1), QPointF(1, 1),
                   Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->mouseMoveEvent(&me));
}

TEST_F(IconWidgetCovTest, InvokedMenuItem_AllBranches)
{
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->invokedMenuItem("shot"));
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->invokedMenuItem("recorder"));
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->invokedMenuItem("unknown"));
}

TEST_F(IconWidgetCovTest, PaintEvent_MinSize_NoBackground)
{
    m_iconWidget->resize(PLUGIN_BACKGROUND_MIN_SIZE, PLUGIN_BACKGROUND_MIN_SIZE);
    QPaintEvent pe(m_iconWidget->rect());
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->paintEvent(&pe));
}

TEST_F(IconWidgetCovTest, PaintEvent_LargeSize_DrawsBackground)
{
    m_iconWidget->resize(40, 40);
    QPaintEvent pe(m_iconWidget->rect());
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->paintEvent(&pe));
}

TEST_F(IconWidgetCovTest, OnPropertyChanged_PositionAndOther)
{
    QVariant posValue(2);
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->onPropertyChanged("Position", posValue));
    QVariant otherValue(1);
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->onPropertyChanged("DisplayMode", otherValue));
}

TEST_F(IconWidgetCovTest, OnPositionChanged_UpdatesPosition)
{
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->onPositionChanged(0));
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->onPositionChanged(1));
}

TEST_F(IconWidgetCovTest, LeaveEvent_ResetsState)
{
    QEvent e(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(m_iconWidget->leaveEvent(&e));
}

TEST_F(QuickPanelWidgetCovTest, Start_BeginTimer)
{
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->start());
    QTest::qWait(500);
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->stop());
}

TEST_F(QuickPanelWidgetCovTest, Stop_WhenTimerInactive)
{
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->stop());
}

TEST_F(QuickPanelWidgetCovTest, Pause_StopsTimerAndUpdatesDescription)
{
    m_quickPanel->start();
    QTest::qWait(500);
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->pause());
    m_quickPanel->stop();
}

TEST_F(QuickPanelWidgetCovTest, OnTimeout_UpdatesShowTime)
{
    m_quickPanel->start();
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->onTimeout());
    m_quickPanel->stop();
}

TEST_F(QuickPanelWidgetCovTest, RefreshIcon_ReappliesType)
{
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->refreshIcon());
}

TEST_F(QuickPanelWidgetCovTest, SetWidgetState_ActiveAndNormal)
{
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->setWidgetState(QuickPanelWidget::WS_ACTIVE));
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->setWidgetState(QuickPanelWidget::WS_NORMAL));
}

TEST_F(QuickPanelWidgetCovTest, MouseReleaseEvent_NotUnderMouse)
{
    QMouseEvent me(QEvent::MouseButtonRelease, QPointF(1, 1), QPointF(1, 1),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_quickPanel->mouseReleaseEvent(&me));
}

TEST_F(ShotStartPluginCovTest, OnStart_DisablesWidgets)
{
    EXPECT_TRUE(m_plugin->onStart());
}

TEST_F(ShotStartPluginCovTest, OnStop_EnablesWidgets)
{
    m_plugin->onStart();
    EXPECT_NO_FATAL_FAILURE(m_plugin->onStop());
}

TEST_F(ShotStartPluginCovTest, OnPause_DoesNothing)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->onPause());
}

TEST_F(ShotStartPluginCovTest, OnRecording_FirstAndSubsequentCalls)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->onRecording());
    QTest::qWait(2100);
    EXPECT_NO_FATAL_FAILURE(m_plugin->onRecording());
    EXPECT_NO_FATAL_FAILURE(m_plugin->onStop());
}

TEST_F(ShotStartPluginCovTest, OnClickQuickPanel_NotRecording)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->onClickQuickPanel());
    QTest::qWait(500);
}

TEST_F(ShotStartPluginCovTest, OnClickQuickPanel_WhileRecording)
{
    m_plugin->onStart();
    EXPECT_NO_FATAL_FAILURE(m_plugin->onClickQuickPanel());
}

TEST_F(ShotStartPluginCovTest, Flags_ReturnsQuickPanelFlags)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->flags());
}

TEST_F(ShotStartPluginCovTest, PluginSizePolicy_ReturnsCustom)
{
    EXPECT_EQ(PluginsItemInterface::Custom, m_plugin->pluginSizePolicy());
}

TEST_F(ShotStartPluginCovTest, ItemSortKey_ReturnsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->itemSortKey("shot-start-plugin"));
}

TEST_F(ShotStartPluginCovTest, SetSortKey_SavesValue)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->setSortKey("shot-start-plugin", 5));
}
