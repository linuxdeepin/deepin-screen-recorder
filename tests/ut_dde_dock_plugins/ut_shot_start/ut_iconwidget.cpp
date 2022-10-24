// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stub.h"
#include "addr_pri.h"
#include <QDebug>
#include <QTest>

#include "../../../dde-dock-plugins/shotstart/iconwidget.h"

namespace  {
class TesticonWidget : public testing::Test
{

public:
    Stub stub;
    void SetUp() override
    {
        m_iconWidget = new IconWidget(nullptr);
        m_iconWidget->show();
//        stub.set(ADDR(DBusService,start), stub_start);
    }
    void TearDown() override
    {
        if (m_iconWidget) {
            delete  m_iconWidget;
            m_iconWidget = nullptr;
        }
    }

public:
    IconWidget *m_iconWidget;
};
}

TEST_F(TesticonWidget, enabled)
{
    EXPECT_TRUE(m_iconWidget->enabled());
}

TEST_F(TesticonWidget, itemContextMenu)
{
    QString menuItem = m_iconWidget->itemContextMenu();
    qInfo() << menuItem;
}

//TEST_F(TesticonWidget, invokedMenuItem)
//{
//    m_iconWidget->invokedMenuItem("shot");
//}

TEST_F(TesticonWidget, mouseEvent)
{
    QTest::mouseMove(m_iconWidget, QPoint(), 200);
    QTest::mousePress(m_iconWidget, Qt::LeftButton, Qt::NoModifier, QPoint(), 100);
    QTest::mouseRelease(m_iconWidget, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);
}
