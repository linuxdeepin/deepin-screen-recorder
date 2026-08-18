// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QShowEvent>
#include <QVariant>
#include <QMetaObject>
#include <QDBusConnection>

#include "ut_mock_pluginproxyinterface.h"

#define protected public
#define private public
#include "../../../src/dde-dock-plugins/recordtime/timewidget.h"
#include "../../../src/dde-dock-plugins/recordtime/recordtimeplugin.h"
#undef protected
#undef private

namespace {
class TestRecordTimeCov2 : public testing::Test
{
public:
    void SetUp() override
    {
        m_plugin.reset(new RecordTimePlugin());
        m_plugin->init(&mock_proxy);
        if (!m_plugin->m_timeWidget)
            m_plugin->m_timeWidget = new TimeWidget();
    }
    void TearDown() override {}

public:
    MockPluginProxyInterface mock_proxy;
    std::shared_ptr<RecordTimePlugin> m_plugin;
};

class TestTimeWidgetCov2 : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new TimeWidget();
    }
    void TearDown() override
    {
        if (m_widget)
            delete m_widget;
    }

public:
    TimeWidget *m_widget;
};
}

TEST_F(TestRecordTimeCov2, flags)
{
    Dock::PluginFlags expected = Dock::Type_System | Dock::Attribute_ForceDock | Dock::Attribute_Normal;
    EXPECT_EQ(expected, m_plugin->flags());
}

TEST_F(TestRecordTimeCov2, pluginSizePolicy)
{
    EXPECT_EQ(PluginsItemInterface::Custom, m_plugin->pluginSizePolicy());
}

TEST_F(TestRecordTimeCov2, pluginIsDisableReturnsFalse)
{
    EXPECT_CALL(mock_proxy, getValue(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(QVariant(false)));
    EXPECT_FALSE(m_plugin->pluginIsDisable());
}

TEST_F(TestRecordTimeCov2, pluginIsDisableReturnsTrue)
{
    EXPECT_CALL(mock_proxy, getValue(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(QVariant(true)));
    EXPECT_TRUE(m_plugin->pluginIsDisable());
}

TEST_F(TestRecordTimeCov2, pluginStateSwitchedToDisabled)
{
    EXPECT_CALL(mock_proxy, getValue(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(QVariant(false)));
    EXPECT_CALL(mock_proxy, saveValue(testing::_, testing::_, testing::_))
        .Times(testing::AnyNumber());
    EXPECT_CALL(mock_proxy, itemRemoved(testing::_, testing::_))
        .Times(testing::AtLeast(1));
    EXPECT_CALL(mock_proxy, itemAdded(testing::_, testing::_))
        .Times(testing::Exactly(0));
    m_plugin->pluginStateSwitched();
}

TEST_F(TestRecordTimeCov2, pluginStateSwitchedToEnabled)
{
    EXPECT_CALL(mock_proxy, getValue(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(QVariant(true)));
    EXPECT_CALL(mock_proxy, saveValue(testing::_, testing::_, testing::_))
        .Times(testing::AnyNumber());
    EXPECT_CALL(mock_proxy, itemRemoved(testing::_, testing::_))
        .Times(testing::Exactly(0));
    EXPECT_CALL(mock_proxy, itemAdded(testing::_, testing::_))
        .Times(testing::AtLeast(1));
    m_plugin->pluginStateSwitched();
}

TEST_F(TestRecordTimeCov2, onRecordingCreatesWatcher)
{
    EXPECT_CALL(mock_proxy, getValue(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(QVariant(false)));
    EXPECT_CALL(mock_proxy, itemRemoved(testing::_, testing::_))
        .Times(testing::AnyNumber());
    EXPECT_CALL(mock_proxy, itemAdded(testing::_, testing::_))
        .Times(testing::AnyNumber());
    m_plugin->m_bshow = true;
    m_plugin->onRecording();
    EXPECT_NE(nullptr, m_plugin->m_serviceWatcher);
}

TEST_F(TestRecordTimeCov2, onRecordingNullWidgetCallsOnStart)
{
    EXPECT_CALL(mock_proxy, getValue(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(QVariant(false)));
    EXPECT_CALL(mock_proxy, itemRemoved(testing::_, testing::_))
        .Times(testing::AnyNumber());
    EXPECT_CALL(mock_proxy, itemAdded(testing::_, testing::_))
        .Times(testing::AnyNumber());
    if (m_plugin->m_timeWidget) {
        delete m_plugin->m_timeWidget.data();
        m_plugin->m_timeWidget = nullptr;
    }
    m_plugin->onRecording();
    EXPECT_NE(nullptr, m_plugin->m_timeWidget);
}

TEST_F(TestRecordTimeCov2, onRecordingLambdaTriggersOnStop)
{
    EXPECT_CALL(mock_proxy, getValue(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(QVariant(false)));
    EXPECT_CALL(mock_proxy, itemRemoved(testing::_, testing::_))
        .Times(testing::AnyNumber());
    EXPECT_CALL(mock_proxy, itemAdded(testing::_, testing::_))
        .Times(testing::AnyNumber());
    m_plugin->m_bshow = true;
    m_plugin->onRecording();
    ASSERT_NE(nullptr, m_plugin->m_serviceWatcher);
    QMetaObject::invokeMethod(m_plugin->m_serviceWatcher,
        "serviceUnregistered", Qt::DirectConnection,
        Q_ARG(QString, QString("com.deepin.ScreenRecorder")));
}

TEST_F(TestTimeWidgetCov2, setting)
{
    EXPECT_NE(nullptr, m_widget->setting());
}

TEST_F(TestTimeWidgetCov2, showEvent)
{
    QShowEvent event;
    m_widget->showEvent(&event);
}
