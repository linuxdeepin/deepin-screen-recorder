// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../../../src/dde-dock-plugins/shotstartrecord/recordiconwidget.h"

#include <gtest/gtest.h>

#include "stub.h"
#include "addr_pri.h"

#include <QDebug>
#include <QTest>

namespace {
class TestRecordIconWidget : public testing::Test
{
public:
    Stub stub;
    void SetUp() override
    {
        m_iconWidget = new RecordIconWidget(nullptr);
        m_iconWidget->show();
    }
    void TearDown() override
    {
        if (m_iconWidget) {
            delete m_iconWidget;
            m_iconWidget = nullptr;
        }
    }

public:
    RecordIconWidget *m_iconWidget;
};
}  // namespace

TEST_F(TestRecordIconWidget, enabled)
{
    EXPECT_TRUE(m_iconWidget->enabled());
}

TEST_F(TestRecordIconWidget, itemContextMenu)
{
    QString menuItem = m_iconWidget->itemContextMenu();
    qInfo() << menuItem;
}

TEST_F(TestRecordIconWidget, mouseEvent)
{
    QTest::mouseMove(m_iconWidget, QPoint(), 200);
    QTest::mousePress(m_iconWidget, Qt::LeftButton, Qt::NoModifier, QPoint(), 100);
    QTest::mouseRelease(m_iconWidget, Qt::LeftButton, Qt::NoModifier, QPoint(), 200);
}
