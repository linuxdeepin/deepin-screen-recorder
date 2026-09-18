// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../../../src/dde-dock-plugins/shotstartrecord/shotstartrecordplugin.h"
#include "ut_mock_pluginproxyinterface.h"

#include <gtest/gtest.h>

#include "stub.h"
#include "addr_pri.h"

#include <QDebug>

namespace {
class TestShotStartRecordPlugin : public testing::Test
{
public:
    Stub stub;
    void SetUp() override
    {
        m_shotStartRecordPlugin.reset(new ShotStartRecordPlugin());
        m_shotStartRecordPlugin->init(&mock_proxy);
    }
    void TearDown() override {}

public:
    MockPluginProxyInterface mock_proxy;
    std::shared_ptr<ShotStartRecordPlugin> m_shotStartRecordPlugin;
};
}  // namespace

static const QString PLUGIN_NAME = QString("shot-start-record-plugin");
static const QString PLUGIN_DISPLAY_NAME = QString("Record");

TEST_F(TestShotStartRecordPlugin, pluginDisplayName)
{
    EXPECT_EQ(PLUGIN_DISPLAY_NAME, m_shotStartRecordPlugin->pluginDisplayName());
}

TEST_F(TestShotStartRecordPlugin, pluginName)
{
    EXPECT_EQ(PLUGIN_NAME, m_shotStartRecordPlugin->pluginName());
}

TEST_F(TestShotStartRecordPlugin, pluginIsAllowDisable)
{
    EXPECT_EQ(true, m_shotStartRecordPlugin->pluginIsAllowDisable());
}

TEST_F(TestShotStartRecordPlugin, itemWidget)
{
    EXPECT_TRUE(nullptr != m_shotStartRecordPlugin->itemWidget(PLUGIN_NAME));
    EXPECT_FALSE(nullptr != m_shotStartRecordPlugin->itemWidget("test"));
}

TEST_F(TestShotStartRecordPlugin, itemTipsWidget)
{
    EXPECT_TRUE(nullptr != m_shotStartRecordPlugin->itemTipsWidget(PLUGIN_NAME));
    EXPECT_FALSE(nullptr != m_shotStartRecordPlugin->itemTipsWidget("test"));
}

TEST_F(TestShotStartRecordPlugin, itemCommand)
{
    EXPECT_TRUE(
        "dbus-send --print-reply --dest=com.deepin.ScreenRecorder /com/deepin/ScreenRecorder com.deepin.ScreenRecorder.stopRecord" ==
        m_shotStartRecordPlugin->itemCommand(PLUGIN_NAME));
    EXPECT_TRUE(nullptr == m_shotStartRecordPlugin->itemCommand("test"));
}

TEST_F(TestShotStartRecordPlugin, itemContextMenu)
{
    EXPECT_TRUE(m_shotStartRecordPlugin->itemContextMenu(PLUGIN_NAME).isEmpty());
    EXPECT_TRUE(m_shotStartRecordPlugin->itemContextMenu("test").isEmpty());
}

TEST_F(TestShotStartRecordPlugin, invokedMenuItem)
{
    m_shotStartRecordPlugin->invokedMenuItem("test", "shot", true);
}

TEST_F(TestShotStartRecordPlugin, pluginStateSwitched)
{
    m_shotStartRecordPlugin->pluginStateSwitched();
}

TEST_F(TestShotStartRecordPlugin, FlowControl)
{
    m_shotStartRecordPlugin->onStart();
    EXPECT_TRUE(m_shotStartRecordPlugin->m_isRecording);

    m_shotStartRecordPlugin->onStop();
    EXPECT_FALSE(m_shotStartRecordPlugin->m_isRecording);
}

