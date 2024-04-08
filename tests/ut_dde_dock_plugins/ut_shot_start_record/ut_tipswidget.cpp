// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../../../src/dde-dock-plugins/shotstartrecord/shotstartrecordplugin.h"
#include "ut_mock_pluginproxyinterface.h"

#include <gtest/gtest.h>

#include "stub.h"
#include "addr_pri.h"

#include <QDebug>
#include <QTest>

namespace {
class TestTipsWidget : public testing::Test
{
public:
    void SetUp() override { tips = new TipsWidget(nullptr); }
    void TearDown() override
    {
        if (tips) {
            delete tips;
            tips = nullptr;
        }
    }

public:
    TipsWidget *tips;
};
}  // namespace

TEST_F(TestTipsWidget, setText)
{
    tips->setText("tips test");
    tips->show();
    QTest::qWait(500);
}

TEST_F(TestTipsWidget, setTextList)
{
    QStringList list;
    list.append("test1");
    list.append("test2");

    tips->setTextList(list);
    tips->show();
}
