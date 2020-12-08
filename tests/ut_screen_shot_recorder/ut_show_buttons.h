#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QPoint>
#include "../../src/screenshot.h"
#include "../../src/keydefine.h"


using namespace testing;

class ShowButtonsTest:public testing::Test{

public:
    ShowButtons showButtons;
    virtual void SetUp() override{
        std::cout << "start ShowButtonsTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end ShowButtonsTest" << std::endl;
    }
};

TEST_F(ShowButtonsTest, releaseContentButtons)
{

}


TEST_F(ShowButtonsTest, showContentButtons)
{
    showButtons.showContentButtons(KEY_1);
    showButtons.releaseContentButtons(KEY_1);

    showButtons.showContentButtons(KEY_2);
    showButtons.showContentButtons(KEY_2);
    showButtons.showContentButtons(KEY_2);
    showButtons.releaseContentButtons(KEY_2);
}

TEST_F(ShowButtonsTest, getKeyCodeFromEvent)
{
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_VBAR).compare(QString("\\")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_QUESTION).compare(QString("/")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_RSUPER).compare(QString("WIN")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_MENU).compare(QString("APPS")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_EQUAL).compare(QString("+")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F1).compare(QString("F1")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F2).compare(QString("F2")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F3).compare(QString("F3")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F4).compare(QString("F4")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F5).compare(QString("F5")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F6).compare(QString("F6")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F7).compare(QString("F7")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F8).compare(QString("F8")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F9).compare(QString("F9")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F10).compare(QString("F10")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F11).compare(QString("F11")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F12).compare(QString("F12")) == 0);

    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_1).compare(QString("1")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_2).compare(QString("2")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_3).compare(QString("3")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_4).compare(QString("4")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_5).compare(QString("5")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_6).compare(QString("6")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_7).compare(QString("7")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_8).compare(QString("8")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_9).compare(QString("9")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_0).compare(QString("0")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_MINUS).compare(QString("-")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_PLUS).compare(QString("+")) == 0);

    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_A).compare(QString("A")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_B).compare(QString("B")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_C).compare(QString("C")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_D).compare(QString("D")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_E).compare(QString("E")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_F).compare(QString("F")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_G).compare(QString("G")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_H).compare(QString("H")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_I).compare(QString("I")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_J).compare(QString("J")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_K).compare(QString("K")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_L).compare(QString("L")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_M).compare(QString("M")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_N).compare(QString("N")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_O).compare(QString("O")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_P).compare(QString("P")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_Q).compare(QString("Q")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_R).compare(QString("R")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_S).compare(QString("S")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_T).compare(QString("T")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_U).compare(QString("U")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_V).compare(QString("V")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_W).compare(QString("W")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_X).compare(QString("X")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_Y).compare(QString("Y")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_Z).compare(QString("Z")) == 0);

    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_BRACKET_OPEN).compare(QString("[")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_BRACKET_CLOSE).compare(QString("]")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_SEMICOLON).compare(QString(";")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_QUOTE).compare(QString("\'")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_BACKSLASH).compare(QString("\\")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_COMMA).compare(QString(",")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_PERIOD).compare(QString(".")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_SLASH).compare(QString("/")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_TAB).compare(QString("TAB")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_CAPSLOCK).compare(QString("CAPS")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_LSHIFT).compare(QString("L SHIFT")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_LCONTROL).compare(QString("L CTRL")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_LALT).compare(QString("L ALT")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_SPACE).compare(QString("SPACE")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_RALT).compare(QString("R ALT")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_RWIN).compare(QString("WIN")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_APPS).compare(QString("APPS")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_RCTRL).compare(QString("R CTRL")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_RSHIFT).compare(QString("R SHIFT")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_ENTER).compare(QString("ENTER")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_BACKSPACE).compare(QString("BKSP")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_SCROLLLOCK).compare(QString("SCR")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_PAUSE).compare(QString("PAUSE")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_INSERT).compare(QString("INS")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_HOME).compare(QString("HOME")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_PAGEUP).compare(QString("PGUP")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_DELETE).compare(QString("DELETE")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_END).compare(QString("END")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_PAGEDOWN).compare(QString("PGDN")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_LEFT).compare(QString("LEFT")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_UP).compare(QString("UP")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_RIGHT).compare(QString("RIGHT")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_DOWN).compare(QString("DOWN")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUMLOCK).compare(QString("NUM")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_DIVIDE).compare(QString("NUM /")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_MULTIPLY).compare(QString("NUM *")) == 0);


    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM0).compare(QString("NUM 0")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM1).compare(QString("NUM 1")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM2).compare(QString("NUM 2")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM3).compare(QString("NUM 3")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM4).compare(QString("NUM 4")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM5).compare(QString("NUM 5")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM6).compare(QString("NUM 6")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM7).compare(QString("NUM 7")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM8).compare(QString("NUM 8")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUM9).compare(QString("NUM 9")) == 0);


    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUMMINUS).compare(QString("NUM -")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUMDELETE).compare(QString("NUM DEL")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUMENTER).compare(QString("NUM =")) == 0);
    EXPECT_TRUE(showButtons.getKeyCodeFromEvent(KEY_NUMPLUS).compare(QString("NUM +")) == 0);
}
