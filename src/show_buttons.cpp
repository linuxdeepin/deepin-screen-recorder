/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "show_buttons.h"
#include "keydefine.h"
#include <QDebug>


const ShowButtons::KeyDescription ShowButtons::keyDescriptions[] = {
    { "ESC", BUTTON_KEY_ESCAPE},
    { "F1", BUTTON_KEY_F1},
    { "F2", BUTTON_KEY_F2},
    { "F3", BUTTON_KEY_F3},

    { "F4", BUTTON_KEY_F4},
    { "F5", BUTTON_KEY_F5},
    { "F6", BUTTON_KEY_F6},
    { "F7", BUTTON_KEY_F7},

    { "F8", BUTTON_KEY_F8},
    { "F9", BUTTON_KEY_F9},
    { "F10", BUTTON_KEY_F10},
    { "F11", BUTTON_KEY_F11},

    { "F12", BUTTON_KEY_F12},
    { "1", BUTTON_KEY_1},
    { "2", BUTTON_KEY_2},
    { "3", BUTTON_KEY_3},

    { "4", BUTTON_KEY_4},
    { "5", BUTTON_KEY_5},
    { "6", BUTTON_KEY_6},
    { "7", BUTTON_KEY_7},
    { "8", BUTTON_KEY_8},
    { "9", BUTTON_KEY_9},
    { "0", BUTTON_KEY_0},
    { "-", BUTTON_KEY_MINUS},

    { "+", BUTTON_KEY_PLUS},
    { "=", BUTTON_KEY_EQUAL},
    { "A", BUTTON_KEY_A},
    { "B", BUTTON_KEY_B},

    { "C", BUTTON_KEY_C},
    { "D", BUTTON_KEY_D},
    { "E", BUTTON_KEY_E},
    { "F", BUTTON_KEY_F},

    { "G", BUTTON_KEY_G},
    { "H", BUTTON_KEY_H},
    { "I", BUTTON_KEY_I},
    { "J", BUTTON_KEY_J},

    { "K", BUTTON_KEY_K},
    { "L", BUTTON_KEY_L},
    { "M", BUTTON_KEY_M},
    { "N", BUTTON_KEY_N},
    { "O", BUTTON_KEY_O},
    { "P", BUTTON_KEY_P},
    { "Q", BUTTON_KEY_Q},
    { "R", BUTTON_KEY_R},

    { "S", BUTTON_KEY_S},
    { "T", BUTTON_KEY_T},
    { "U", BUTTON_KEY_U},
    { "V", BUTTON_KEY_V},

    { "W", BUTTON_KEY_W},
    { "X", BUTTON_KEY_X},
    { "Y", BUTTON_KEY_Y},
    { "Z", BUTTON_KEY_Z},

    { "[", BUTTON_KEY_BRACKET_OPEN},
    { "]", BUTTON_KEY_BRACKET_CLOSE},
    { ";", BUTTON_KEY_SEMICOLON},
    { "'", BUTTON_KEY_QUOTE},

    { "\\", BUTTON_KEY_BACKSLASH},
    { "|", BUTTON_KEY_VBAR},
    { ",", BUTTON_KEY_COMMA},
    { ".", BUTTON_KEY_PERIOD},
    { "/", BUTTON_KEY_SLASH},
    { "?", BUTTON_KEY_QUESTION},
    { "TAB", BUTTON_KEY_TAB},
    { "Aa", BUTTON_KEY_CAPSLOCK},

    { "LSHIFT", BUTTON_KEY_LSHIFT},
    { "LCTRL", BUTTON_KEY_LCONTROL},
    { "LALT", BUTTON_KEY_LALT},
    { "SPACE", BUTTON_KEY_SPACE},

    { "RALT", BUTTON_KEY_RALT},
    { "WIN", BUTTON_KEY_LWIN},
    { "MENU", BUTTON_KEY_MENU},
    { "RCTRL", BUTTON_KEY_RCTRL},

    { "RSHIFT", BUTTON_KEY_RSHIFT},
    { "ENTER", BUTTON_KEY_ENTER},
    { "BACKSPACE", BUTTON_KEY_BACKSPACE},
    { "SCRLOCK", BUTTON_KEY_SCROLLLOCK},
    { "PAUSE", BUTTON_KEY_PAUSE},

    { "INSERT", BUTTON_KEY_INSERT},
    { "HOME", BUTTON_KEY_HOME},
    { "PAGEUP", BUTTON_KEY_PAGEUP},
    { "DELETE", BUTTON_KEY_DELETE},

    { "END", BUTTON_KEY_END},
    { "PAGEDOWN", BUTTON_KEY_PAGEDOWN},
    { "LEFT", BUTTON_KEY_LEFT},
    { "UP", BUTTON_KEY_UP},

    { "RIGHT", BUTTON_KEY_RIGHT},
    { "DOWN", BUTTON_KEY_DOWN},
    { "NUM", BUTTON_KEY_NUMLOCK},
    { "NUM /", BUTTON_KEY_DIVIDE},
    { "NUM *", BUTTON_KEY_MULTIPLY},
    { "NUM 7", BUTTON_KEY_NUM7},
    { "NUM 8", BUTTON_KEY_NUM8},

    { "NUM 9", BUTTON_KEY_NUM9},
    { "NUM -", BUTTON_KEY_NUMMINUS},
    { "NUM 4", BUTTON_KEY_NUM4},
    { "NUM 5", BUTTON_KEY_NUM5},

    { "NUM 6", BUTTON_KEY_NUM6},
    { "NUM +", BUTTON_KEY_NUMPLUS},
    { "NUM 1", BUTTON_KEY_NUM1},
    { "NUM 2", BUTTON_KEY_NUM2},
    { "NUM 3", BUTTON_KEY_NUM3},
    { "NUM 0", BUTTON_KEY_NUM0},
    { "NUM DEL", BUTTON_KEY_NUMDELETE},
    { "NUM =", BUTTON_KEY_NUMENTER}
};

ShowButtons::ShowButtons(DWidget *parent) : DWidget(parent)
{
}


void ShowButtons::showContentButtons(unsigned char keyCode)
{
    QString t_keyCode = "";
    t_keyCode = getKeyCodeFromEvent(keyCode);

    if (t_keyCode != "") {
        if (m_keyCodeVec.contains(t_keyCode)) {
            return;
        }

        else {
            m_keyCodeVec.append(t_keyCode);
            qDebug() << t_keyCode << " key press";
            emit keyShowSignal(t_keyCode);
        }
    }
}

void ShowButtons::releaseContentButtons(unsigned char keyCode)
{
    QString t_keyCode = "";
    t_keyCode = getKeyCodeFromEvent(keyCode);

    if (t_keyCode != "") {
        if (m_keyCodeVec.contains(t_keyCode)) {
            m_keyCodeVec.removeOne(t_keyCode);
            return;
        }
    }
}

QString ShowButtons::getKeyCodeFromEvent(unsigned char keyCode)
{
    QString t_keyCode = "";
    if (keyCode == KEY_F1) {
        t_keyCode = "F1";
    }

    else if (keyCode == KEY_F2) {
        t_keyCode = "F2";
    }

    else if (keyCode == KEY_F3) {
        t_keyCode = "F3";
    }

    else if (keyCode == KEY_F4) {
        t_keyCode = "F4";
    }

    else if (keyCode == KEY_F5) {
        t_keyCode = "F5";
    }

    else if (keyCode == KEY_F6) {
        t_keyCode = "F6";
    }

    else if (keyCode == KEY_F7) {
        t_keyCode = "F7";
    }

    else if (keyCode == KEY_F8) {
        t_keyCode = "F8";
    }

    else if (keyCode == KEY_F9) {
        t_keyCode = "F9";
    }

    else if (keyCode == KEY_F10) {
        t_keyCode = "F10";
    }

    else if (keyCode == KEY_F11) {
        t_keyCode = "F11";
    }

    else if (keyCode == KEY_F12) {
        t_keyCode = "F12";
    }

    else if (keyCode == KEY_1) {
        t_keyCode = "1";
    }

    else if (keyCode == KEY_2) {
        t_keyCode = "2";
    }

    else if (keyCode == KEY_3) {
        t_keyCode = "3";
    }

    else if (keyCode == KEY_4) {
        t_keyCode = "4";
    }

    else if (keyCode == KEY_5) {
        t_keyCode = "5";
    }

    else if (keyCode == KEY_6) {
        t_keyCode = "6";
    }

    else if (keyCode == KEY_7) {
        t_keyCode = "7";
    }

    else if (keyCode == KEY_8) {
        t_keyCode = "8";
    }

    else if (keyCode == KEY_9) {
        t_keyCode = "9";
    }

    else if (keyCode == KEY_0) {
        t_keyCode = "0";
    }

    if (keyCode == KEY_MINUS) {
        t_keyCode = "-";
    }

    else if (keyCode == KEY_PLUS) {
        t_keyCode = "+";
    }

    else if (keyCode == KEY_EQUAL) {
        t_keyCode = "=";
    }

    else if (keyCode == KEY_A) {
        t_keyCode = "A";
    }

    else if (keyCode == KEY_B) {
        t_keyCode = "B";
    }

    else if (keyCode == KEY_C) {
        t_keyCode = "C";
    }

    else if (keyCode == KEY_D) {
        t_keyCode = "D";
    }

    else if (keyCode == KEY_E) {
        t_keyCode = "E";
    }

    else if (keyCode == KEY_F) {
        t_keyCode = "F";
    }

    else if (keyCode == KEY_G) {
        t_keyCode = "G";
    }

    else if (keyCode == KEY_H) {
        t_keyCode = "H";
    }

    else if (keyCode == KEY_I) {
        t_keyCode = "I";
    }

    else if (keyCode == KEY_J) {
        t_keyCode = "J";
    }

    else if (keyCode == KEY_K) {
        t_keyCode = "K";
    }

    else if (keyCode == KEY_L) {
        t_keyCode = "L";
    }

    else if (keyCode == KEY_M) {
        t_keyCode = "M";
    }

    else if (keyCode == KEY_N) {
        t_keyCode = "N";
    }

    else if (keyCode == KEY_O) {
        t_keyCode = "O";
    }

    else if (keyCode == KEY_P) {
        t_keyCode = "P";
    }

    else if (keyCode == KEY_Q) {
        t_keyCode = "Q";
    }

    else if (keyCode == KEY_R) {
        t_keyCode = "R";
    }

    else if (keyCode == KEY_S) {
        t_keyCode = "S";
    }

    else if (keyCode == KEY_T) {
        t_keyCode = "T";
    }

    else if (keyCode == KEY_U) {
        t_keyCode = "U";
    }

    else if (keyCode == KEY_V) {
        t_keyCode = "V";
    }

    else if (keyCode == KEY_W) {
        t_keyCode = "W";
    }

    else if (keyCode == KEY_X) {
        t_keyCode = "X";
    }

    else if (keyCode == KEY_Y) {
        t_keyCode = "Y";
    }

    else if (keyCode == KEY_Z) {
        t_keyCode = "Z";
    }

    else if (keyCode == KEY_BRACKET_OPEN) {
        t_keyCode = "[";
    }

    else if (keyCode == KEY_BRACKET_CLOSE) {
        t_keyCode = "]";
    }

    else if (keyCode == KEY_SEMICOLON) {
        t_keyCode = ";";
    }

    else if (keyCode == KEY_QUOTE) {
        t_keyCode = "\'";
    }

    else if (keyCode == KEY_BACKSLASH) {
        t_keyCode = "\\";
    }

    else if (keyCode == KEY_VBAR) {
        t_keyCode = "|";
    }

    else if (keyCode == KEY_COMMA) {
        t_keyCode = ",";
    }

    else if (keyCode == KEY_PERIOD) {
        t_keyCode = ".";
    }

    else if (keyCode == KEY_SLASH) {
        t_keyCode = "/";
    }

    else if (keyCode == KEY_QUESTION) {
        t_keyCode = "?";
    }

    else if (keyCode == KEY_TAB) {
        t_keyCode = "TAB";
    }

    else if (keyCode == KEY_CAPSLOCK) {
        t_keyCode = "CAPS";
    }

    else if (keyCode == KEY_LSHIFT) {
        t_keyCode = "L SHIFT";
    }

    else if (keyCode == KEY_LCONTROL) {
        t_keyCode = "L CTRL";
    }

    else if (keyCode == KEY_LALT) {
        t_keyCode = "L ALT";
    }

    else if (keyCode == KEY_SPACE) {
        t_keyCode = "SPACE";
    }

    else if (keyCode == KEY_RALT) {
        t_keyCode = "R ALT";
    }

    else if (keyCode == KEY_RWIN) {
        t_keyCode = "WIN";
    }

    else if (keyCode == KEY_RSUPER) {
        t_keyCode = "SUPER";
    }

    else if (keyCode == KEY_APPS) {
        t_keyCode = "APPS";
    }

    else if (keyCode == KEY_MENU) {
        t_keyCode = "MENU";
    }

    else if (keyCode == KEY_RCTRL) {
        t_keyCode = "R CTRL";
    }

    else if (keyCode == KEY_RSHIFT) {
        t_keyCode = "R SHIFT";
    }

    else if (keyCode == KEY_ENTER) {
        t_keyCode = "ENTER";
    }

    else if (keyCode == KEY_BACKSPACE) {
        t_keyCode = "BKSP";
    }

    else if (keyCode == KEY_SCROLLLOCK) {
        t_keyCode = "SCR";
    }

    else if (keyCode == KEY_PAUSE) {
        t_keyCode = "PAUSE";
    }

    else if (keyCode == KEY_INSERT) {
        t_keyCode = "INS";
    }

    else if (keyCode == KEY_HOME) {
        t_keyCode = "HOME";
    }

    else if (keyCode == KEY_PAGEUP) {
        t_keyCode = "PGUP";
    }

    else if (keyCode == KEY_DELETE) {
        t_keyCode = "DELETE";
    }

    else if (keyCode == KEY_END) {
        t_keyCode = "END";
    }

    else if (keyCode == KEY_PAGEDOWN) {
        t_keyCode = "PGDN";
    }

    else if (keyCode == KEY_LEFT) {
        t_keyCode = "LEFT";
    }

    else if (keyCode == KEY_UP) {
        t_keyCode = "UP";
    }

    else if (keyCode == KEY_RIGHT) {
        t_keyCode = "RIGHT";
    }

    else if (keyCode == KEY_DOWN) {
        t_keyCode = "DOWN";
    }

    else if (keyCode == KEY_NUMLOCK) {
        t_keyCode = "NUM";
    }

    else if (keyCode == KEY_DIVIDE) {
        t_keyCode = "NUM /";
    }

    else if (keyCode == KEY_MULTIPLY) {
        t_keyCode = "NUM *";
    }

    else if (keyCode == KEY_NUM7) {
        t_keyCode = "NUM 7";
    }

    else if (keyCode == KEY_NUM8) {
        t_keyCode = "NUM 8";
    }

    else if (keyCode == KEY_NUM9) {
        t_keyCode = "NUM 9";
    }

    else if (keyCode == KEY_NUMMINUS) {
        t_keyCode = "NUM -";
    }

    else if (keyCode == KEY_NUM4) {
        t_keyCode = "NUM 4";
    }

    else if (keyCode == KEY_NUM5) {
        t_keyCode = "NUM 5";
    }

    else if (keyCode == KEY_NUM6) {
        t_keyCode = "NUM 6";
    }

    else if (keyCode == KEY_NUMPLUS) {
        t_keyCode = "NUM +";
    }

    else if (keyCode == KEY_NUM1) {
        t_keyCode = "NUM 1";
    }

    else if (keyCode == KEY_NUM2) {
        t_keyCode = "NUM 2";
    }

    else if (keyCode == KEY_NUM3) {
        t_keyCode = "NUM 3";
    }

    else if (keyCode == KEY_NUM0) {
        t_keyCode = "NUM 0";
    }

    else if (keyCode == KEY_NUMDELETE) {
        t_keyCode = "NUM DEL";
    }

    else if (keyCode == KEY_NUMENTER) {
        t_keyCode = "NUM =";
    }

    return t_keyCode;
}
