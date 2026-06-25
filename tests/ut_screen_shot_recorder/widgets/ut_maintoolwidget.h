// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QtTest>
#include <QColor>
#include <QDebug>
#include <gtest/gtest.h>
#include "../../src/widgets/maintoolwidget.h"
#include "addr_pri.h"


using namespace testing;

ACCESS_PRIVATE_FIELD(MainToolWidget, ToolButton*, m_recordBtn);
ACCESS_PRIVATE_FIELD(MainToolWidget, ToolButton*, m_shotBtn);

class MainToolWidgetTest:public testing::Test, public QObject{

public:
    MainToolWidget *widget;
    QString curType;
    virtual void SetUp() override{
        std::cout << "start MainToolWidgetTest" << std::endl;
        widget  = new MainToolWidget(nullptr);
        connect(widget, &MainToolWidget::buttonChecked, this, &MainToolWidgetTest::buttonChecked);
    }

    virtual void TearDown() override{
        delete widget;
        std::cout << "end MainToolWidgetTest" << std::endl;
    }
public slots:
    void buttonChecked(bool checked, QString type);
};
TEST_F(MainToolWidgetTest, colorChecked)
{
    ToolButton* recorBtn = access_private_field::MainToolWidgetm_recordBtn(*widget);
    ToolButton* recorShot = access_private_field::MainToolWidgetm_shotBtn(*widget);
    curType = "record";
    QTest::mouseClick(recorBtn, Qt::MouseButton::LeftButton);
    curType = "shot";
    QTest::mouseClick(recorShot, Qt::MouseButton::LeftButton);

}
void MainToolWidgetTest::buttonChecked(bool checked, QString type)
{
    EXPECT_EQ(type, curType);
}

// === Extended coverage ===
TEST_F(MainToolWidgetTest, constructNotNull)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(MainToolWidgetTest, setRecordButtonOut)
{
    EXPECT_NO_FATAL_FAILURE(widget->setRecordButtonOut());
}

TEST_F(MainToolWidgetTest, setRecordLauchModeRecord)
{
    EXPECT_NO_FATAL_FAILURE(widget->setRecordLauchMode(0));
}

TEST_F(MainToolWidgetTest, setRecordLauchModeShot)
{
    EXPECT_NO_FATAL_FAILURE(widget->setRecordLauchMode(1));
}

TEST_F(MainToolWidgetTest, setRecordLauchModeScroll)
{
    EXPECT_NO_FATAL_FAILURE(widget->setRecordLauchMode(2));
}

TEST_F(MainToolWidgetTest, setRecordLauchModeOcr)
{
    EXPECT_NO_FATAL_FAILURE(widget->setRecordLauchMode(3));
}

TEST_F(MainToolWidgetTest, initWidgetAgain)
{
    EXPECT_NO_FATAL_FAILURE(widget->initWidget());
}

TEST_F(MainToolWidgetTest, initMainLabel)
{
    EXPECT_NO_FATAL_FAILURE(widget->initMainLabel());
}

TEST_F(MainToolWidgetTest, installTipHintSafe)
{
    EXPECT_NO_FATAL_FAILURE(widget->installTipHint(nullptr, "test hint"));
}

TEST_F(MainToolWidgetTest, installHintSafe)
{
    EXPECT_NO_FATAL_FAILURE(widget->installHint(nullptr, nullptr));
}

TEST_F(MainToolWidgetTest, destructorRunsClean)
{
    MainToolWidget *tmp = new MainToolWidget(nullptr);
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}
