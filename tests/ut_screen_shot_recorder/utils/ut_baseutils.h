// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include "../../src/utils/baseutils.h"
#include <gtest/gtest.h>



using namespace testing;

class Baseutils_Test:public testing::Test{

public:
    virtual void SetUp() override{
        std::cout << "start UtilsTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end UtilsTest" << std::endl;
    }
};

TEST_F(Baseutils_Test, colorIndexOf)
{
    int a, r, g, b;
    QColor color;

    // Index 0: Black   #000000
    color = BaseUtils::colorIndexOf(0);
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0x00, r);
    EXPECT_EQ(0x00, g);
    EXPECT_EQ(0x00, b);

    // Index 1: Gray    #7D7D7D
    color = BaseUtils::colorIndexOf(1);
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0x7D, r);
    EXPECT_EQ(0x7D, g);
    EXPECT_EQ(0x7D, b);

    // Index 2: White   #FFFFFF
    color = BaseUtils::colorIndexOf(2);
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0xFF, r);
    EXPECT_EQ(0xFF, g);
    EXPECT_EQ(0xFF, b);

    // Index 3: Red     #F82A2A
    color = BaseUtils::colorIndexOf(3);
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0xF8, r);
    EXPECT_EQ(0x2A, g);
    EXPECT_EQ(0x2A, b);

    // Index 4: Orange  #FF8100
    color = BaseUtils::colorIndexOf(4);
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0xFF, r);
    EXPECT_EQ(0x81, g);
    EXPECT_EQ(0x00, b);

    // Index 5: Yellow  #FFF100
    color = BaseUtils::colorIndexOf(5);
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0xFF, r);
    EXPECT_EQ(0xF1, g);
    EXPECT_EQ(0x00, b);

    // Index 9: Blue    #0089F7
    color = BaseUtils::colorIndexOf(9);
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0x00, r);
    EXPECT_EQ(0x89, g);
    EXPECT_EQ(0xF7, b);

    // Index 11: DarkBlue #0C00A0
    color = BaseUtils::colorIndexOf(11);
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0x0C, r);
    EXPECT_EQ(0x00, g);
    EXPECT_EQ(0xA0, b);
}

TEST_F(Baseutils_Test, colorIndex)
{
    // Valid colors from the current color list
    int index = BaseUtils::colorIndex(QColor("#000000"));
    EXPECT_EQ(0, index);

    index = BaseUtils::colorIndex(QColor("#FF8100"));
    EXPECT_EQ(4, index);

    index = BaseUtils::colorIndex(QColor("#FFF100"));
    EXPECT_EQ(5, index);

    index = BaseUtils::colorIndex(QColor("#0089F7"));
    EXPECT_EQ(9, index);

    // A color not in the list -> index == -1
    index = BaseUtils::colorIndex(QColor("#123456"));
    EXPECT_EQ(-1, index);
}
TEST_F(Baseutils_Test, isValidFormat)
{
    bool valid = BaseUtils::isValidFormat("bmp");
    EXPECT_TRUE(valid);

    valid = BaseUtils::isValidFormat("mp4");
    EXPECT_FALSE(valid);
}
/*
TEST_F(Baseutils_Test, getFileContent)
{
    const QString testValue = "0123456789abcdefg";
    const QString fileName = "testFile.conf";
    QString cmd = QString("echo '%1' >> %2").arg(testValue).arg(fileName);
    system("echo '0123456789abcdefg' >> testFile.conf");
    QString string = getFileContent(fileName);
    EXPECT_FALSE(string.isEmpty());
    EXPECT_TRUE(string.split("\n")[0] == testValue);
    system("rm testFile.conf");
}
*/
TEST_F(Baseutils_Test, stringWidth)
{
    const QString testValue = "0123456789abcdefg";
    const QString testValueCn = "统信UOS";
    QFont font("Arial");
    qDebug() << BaseUtils::stringWidth(font, testValue);
    //qDebug() << stringWidth(font, testValueCn);
    //EXPECT_EQ(17, stringWidth(font, testValue));
    //EXPECT_EQ(10, stringWidth(font, testValueCn));
}
TEST_F(Baseutils_Test, setCursorShape)
{
    QString cursorName = "start";
    int colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "rotate";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);


    cursorName = "rectangle";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "oval";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);


    cursorName = "arrow";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "text";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "line";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "straightLine";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);
}
