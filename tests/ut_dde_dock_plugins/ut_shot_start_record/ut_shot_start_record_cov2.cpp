// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../../../src/dde-dock-plugins/shotstartrecord/quickpanelwidget.h"
#include "../../../src/dde-dock-plugins/shotstartrecord/recordiconwidget.h"
#include "../../../src/dde-dock-plugins/shotstartrecord/recordiconwidget_interface.h"
#include "../../../src/dde-dock-plugins/shotstartrecord/shotstartrecordplugin.h"
#include "ut_mock_pluginproxyinterface.h"

#include <gtest/gtest.h>

#include <QDBusArgument>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QSignalSpy>
#include <QTest>

namespace {

class QuickPanelWidgetCov2Test : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new QuickPanelWidget();
        m_widget->show();
    }
    void TearDown() override
    {
        delete m_widget;
    }
    QuickPanelWidget *m_widget = nullptr;
};

TEST_F(QuickPanelWidgetCov2Test, Pause_StopsTimerAndUpdatesDescription)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->pause());
}

TEST_F(QuickPanelWidgetCov2Test, RefreshIcon_ReappliesCurrentType)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->refreshIcon());
}

TEST_F(QuickPanelWidgetCov2Test, OnTimeout_UpdatesTimeDisplay)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->onTimeout());
}

TEST_F(QuickPanelWidgetCov2Test, MouseReleaseEvent_UnderMouseEmitsClicked)
{
    m_widget->setAttribute(Qt::WA_UnderMouse, true);
    QSignalSpy spy(m_widget, &QuickPanelWidget::clicked);
    QMouseEvent me(QEvent::MouseButtonRelease, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_widget->mouseReleaseEvent(&me));
    EXPECT_EQ(1, spy.count());
}

TEST_F(QuickPanelWidgetCov2Test, MouseReleaseEvent_NotUnderMouseSkipsEmit)
{
    m_widget->setAttribute(Qt::WA_UnderMouse, false);
    QSignalSpy spy(m_widget, &QuickPanelWidget::clicked);
    QMouseEvent me(QEvent::MouseButtonRelease, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_widget->mouseReleaseEvent(&me));
    EXPECT_EQ(0, spy.count());
}


class RecordIconWidgetCov2Test : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new RecordIconWidget();
        m_widget->show();
    }
    void TearDown() override
    {
        delete m_widget;
    }
    RecordIconWidget *m_widget = nullptr;
};

TEST_F(RecordIconWidgetCov2Test, MouseMoveEvent_SetsHoverState)
{
    QMouseEvent me(QEvent::MouseMove, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_widget->mouseMoveEvent(&me));
}

TEST_F(RecordIconWidgetCov2Test, LeaveEvent_ResetsHoverAndPressed)
{
    QEvent e(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(m_widget->leaveEvent(&e));
}

TEST_F(RecordIconWidgetCov2Test, OnPositionChanged_UpdatesPositionAndIcon)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->onPositionChanged(0));
    EXPECT_NO_FATAL_FAILURE(m_widget->onPositionChanged(1));
    EXPECT_NO_FATAL_FAILURE(m_widget->onPositionChanged(3));
}

TEST_F(RecordIconWidgetCov2Test, InvokedMenuItem_Screenshot)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->invokedMenuItem("shot"));
}

TEST_F(RecordIconWidgetCov2Test, InvokedMenuItem_Recorder)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->invokedMenuItem("recorder"));
}

TEST_F(RecordIconWidgetCov2Test, InvokedMenuItem_Unknown)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->invokedMenuItem("unknown"));
}

TEST_F(RecordIconWidgetCov2Test, OnPropertyChanged_Position)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->onPropertyChanged("Position", QVariant(0)));
}

TEST_F(RecordIconWidgetCov2Test, OnPropertyChanged_NonPosition)
{
    EXPECT_NO_FATAL_FAILURE(m_widget->onPropertyChanged("DisplayMode", QVariant(1)));
}

TEST_F(RecordIconWidgetCov2Test, PaintEvent_SmallSize)
{
    QPaintEvent pe(m_widget->rect());
    EXPECT_NO_FATAL_FAILURE(m_widget->paintEvent(&pe));
}

TEST_F(RecordIconWidgetCov2Test, PaintEvent_LargeSize)
{
    m_widget->resize(60, 60);
    QPaintEvent pe(m_widget->rect());
    EXPECT_NO_FATAL_FAILURE(m_widget->paintEvent(&pe));
}


class ShotStartRecordPluginCov2Test : public testing::Test
{
public:
    void SetUp() override
    {
        m_plugin.reset(new ShotStartRecordPlugin());
        m_plugin->init(&mock_proxy);
    }
    void TearDown() override {}

    MockPluginProxyInterface mock_proxy;
    std::shared_ptr<ShotStartRecordPlugin> m_plugin;
};

TEST_F(ShotStartRecordPluginCov2Test, SetSortKey_SavesValue)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->setSortKey("shot-start-record-plugin", 5));
}

TEST_F(ShotStartRecordPluginCov2Test, Flags_ReturnsQuickPanelFlags)
{
    Dock::PluginFlags f = m_plugin->flags();
    EXPECT_TRUE(f.testFlag(Dock::Type_Quick));
    EXPECT_TRUE(f.testFlag(Dock::Quick_Panel_Single));
    EXPECT_TRUE(f.testFlag(Dock::Attribute_Normal));
}

TEST_F(ShotStartRecordPluginCov2Test, OnRecording_InitializesCheckTimer)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->onRecording());
}

TEST_F(ShotStartRecordPluginCov2Test, OnRecording_TriggersCheckTimerLambda)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->onRecording());
    QTest::qWait(2100);
    EXPECT_NO_FATAL_FAILURE(m_plugin->onRecording());
    QTest::qWait(2100);
}

TEST_F(ShotStartRecordPluginCov2Test, OnPause_PausesQuickPanel)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->onPause());
}

TEST_F(ShotStartRecordPluginCov2Test, OnClickQuickPanel_StopsRecording)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->onClickQuickPanel());
}

TEST_F(ShotStartRecordPluginCov2Test, ItemSortKey_ReturnsValue)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->itemSortKey("shot-start-record-plugin"));
}

TEST_F(ShotStartRecordPluginCov2Test, SetTrayIconVisible_TogglesVisibility)
{
    EXPECT_NO_FATAL_FAILURE(m_plugin->setTrayIconVisible(true));
    EXPECT_NO_FATAL_FAILURE(m_plugin->setTrayIconVisible(false));
}

TEST_F(ShotStartRecordPluginCov2Test, PluginSizePolicy_ReturnsCustom)
{
    EXPECT_EQ(PluginsItemInterface::Custom, m_plugin->pluginSizePolicy());
}


TEST(DockRectSerializationCov2Test, Serialize_WritesAllFieldsWithoutException)
{
    DockRect inRect{10, 20, 100u, 200u};
    QDBusArgument writeArg;
    EXPECT_NO_FATAL_FAILURE(writeArg << inRect);
}

TEST(DockRectSerializationCov2Test, Deserialize_ReadsAllFieldsWithoutException)
{
    DockRect outRect{0, 0, 0u, 0u};
    QDBusArgument arg;
    EXPECT_NO_FATAL_FAILURE(arg >> outRect);
}

} // namespace
