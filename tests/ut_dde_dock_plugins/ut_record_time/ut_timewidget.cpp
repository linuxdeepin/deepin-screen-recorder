#include <QWidget>
#include <gtest/gtest.h>
#include <DGuiApplicationHelper>

#define protected public
#define private public
#include "../../../dde-dock-plugins/recordtime/timewidget.h"

DGUI_USE_NAMESPACE

namespace  {
    class TestDBusService : public testing::Test {

    public:

        void SetUp() override
        {
            m_timeWidget = new TimeWidget();
        }
        void TearDown() override
        {
        }

    public:
        TimeWidget *m_timeWidget;
    };
}

TEST_F(TestDBusService,enabled)
{
    EXPECT_EQ(true,m_timeWidget->enabled());
}

TEST_F(TestDBusService,start)
{
    m_timeWidget->start();
}

TEST_F(TestDBusService,stop)
{
    m_timeWidget->stop();
}

TEST_F(TestDBusService,sizeHint)
{
    EXPECT_EQ(86,m_timeWidget->sizeHint().width());
    EXPECT_EQ(36,m_timeWidget->sizeHint().height());
}

TEST_F(TestDBusService,onTimeout)
{
    m_timeWidget->onTimeout();
}

TEST_F(TestDBusService,onPositionChanged)
{
    m_timeWidget->onPositionChanged(1);
}
