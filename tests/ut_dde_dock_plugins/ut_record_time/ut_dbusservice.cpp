#include <gtest/gtest.h>
#include "stub.h"
#include "addr_pri.h"
#include <QDebug>
#include "../../../dde-dock-plugins/recordtime/dbusservice.h"

//void stub_start(){}

namespace  {
class TestDBusService : public testing::Test {

public:
    Stub stub;
    void SetUp() override
    {
        m_dBusService = new DBusService(nullptr);
//        stub.set(ADDR(DBusService,start), stub_start);
    }
    void TearDown() override
    {
    }

public:
    DBusService *m_dBusService;
};
}

TEST_F(TestDBusService,onStart)
{
    m_dBusService->onStart();
}

TEST_F(TestDBusService,onStop)
{
    m_dBusService->onStop();
}
