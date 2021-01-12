#include <QWidget>
#include <gtest/gtest.h>
#include "ut_mock_pluginproxyinterface.h"

#define protected public
#define private public
#include "../../../src/dde-dock-plugins/recordtime/recordtimeplugin.h"

namespace  {
class TestRecordTimePlugin : public testing::Test {
public:

    void SetUp() override
    {
        m_recordTimePlugin.reset(new RecordTimePlugin());
        m_recordTimePlugin->init(&mock_proxy);
    }
    void TearDown() override
    {
    }

public:
    MockPluginProxyInterface mock_proxy;
    std::shared_ptr<RecordTimePlugin> m_recordTimePlugin;
};
}

static const QString PLUGIN_NAME = QString("deepin-screen-recorder-plugin");
static const QString PLUGIN_DISPLAY_NAME = QString("截图录屏");

TEST_F(TestRecordTimePlugin, pluginName)
{
    EXPECT_EQ(PLUGIN_NAME,m_recordTimePlugin->pluginName());
}

TEST_F(TestRecordTimePlugin, pluginDisplayName)
{
    EXPECT_EQ(PLUGIN_DISPLAY_NAME,m_recordTimePlugin->pluginDisplayName());
}

TEST_F(TestRecordTimePlugin, pluginIsAllowDisable)
{
    EXPECT_EQ(true,m_recordTimePlugin->pluginIsAllowDisable());
}

TEST_F(TestRecordTimePlugin, itemWidget)
{
    EXPECT_TRUE( nullptr != m_recordTimePlugin->itemWidget("dummy") );
}

TEST_F(TestRecordTimePlugin, onStart)
{
    emit m_recordTimePlugin->m_dBusService->start();
}

TEST_F(TestRecordTimePlugin, onStop)
{
    emit m_recordTimePlugin->m_dBusService->stop();
}

TEST_F(TestRecordTimePlugin, refresh)
{
    m_recordTimePlugin->refresh();
}
