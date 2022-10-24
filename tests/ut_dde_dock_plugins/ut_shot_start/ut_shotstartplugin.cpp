// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stub.h"
#include "addr_pri.h"
#include <QDebug>

#include "../../../dde-dock-plugins/shotstart/shotstartplugin.h"
#include "ut_mock_pluginproxyinterface.h"

namespace  {
class TestShotStartPlugin : public testing::Test
{

public:
    Stub stub;
    void SetUp() override
    {
        m_shotStartPlugin.reset(new ShotStartPlugin());
        m_shotStartPlugin->init(&mock_proxy);
    }
    void TearDown() override
    {
    }

public:
    MockPluginProxyInterface mock_proxy;
    std::shared_ptr<ShotStartPlugin> m_shotStartPlugin;
};
}

static const QString PLUGIN_NAME = QString("shot-start-plugin");
static const QString PLUGIN_DISPLAY_NAME = QString("Screen Capture");

TEST_F(TestShotStartPlugin, pluginDisplayName)
{
    EXPECT_EQ("Screen Capture", m_shotStartPlugin->pluginDisplayName());
}

TEST_F(TestShotStartPlugin, pluginName)
{
    EXPECT_EQ(PLUGIN_NAME, m_shotStartPlugin->pluginName());
}

TEST_F(TestShotStartPlugin, pluginIsAllowDisable)
{
    EXPECT_EQ(true, m_shotStartPlugin->pluginIsAllowDisable());
}

TEST_F(TestShotStartPlugin, itemWidget)
{
    EXPECT_TRUE(nullptr != m_shotStartPlugin->itemWidget(PLUGIN_NAME));
    EXPECT_FALSE(nullptr != m_shotStartPlugin->itemWidget("test"));
}

TEST_F(TestShotStartPlugin, itemTipsWidget)
{
    EXPECT_TRUE(nullptr != m_shotStartPlugin->itemTipsWidget(PLUGIN_NAME));
    EXPECT_FALSE(nullptr != m_shotStartPlugin->itemTipsWidget("test"));
}

TEST_F(TestShotStartPlugin, itemCommand)
{
    EXPECT_TRUE("dbus-send --print-reply --dest=com.deepin.Screenshot /com/deepin/Screenshot com.deepin.Screenshot.StartScreenshot"
                == m_shotStartPlugin->itemCommand(PLUGIN_NAME));
    EXPECT_TRUE(nullptr == m_shotStartPlugin->itemCommand("test"));
}

TEST_F(TestShotStartPlugin, itemContextMenu)
{
    QString test = m_shotStartPlugin->itemContextMenu(PLUGIN_NAME);
    EXPECT_TRUE(nullptr != m_shotStartPlugin->itemContextMenu(PLUGIN_NAME));
    EXPECT_FALSE(nullptr != m_shotStartPlugin->itemContextMenu("test"));
}

TEST_F(TestShotStartPlugin, invokedMenuItem)
{
    m_shotStartPlugin->invokedMenuItem("test", "shot", true);
}

TEST_F(TestShotStartPlugin, pluginStateSwitched)
{
    m_shotStartPlugin->pluginStateSwitched();
}
