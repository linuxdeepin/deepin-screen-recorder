/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Zheng Youge<youge.zheng@deepin.com>
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
#ifndef SHOW_BUTTONS_H
#define SHOW_BUTTONS_H

#include <DWidget>
#include <QObject>
#include <QMap>
#include <QVector>

DWIDGET_USE_NAMESPACE

class ShowButtons : public DWidget
{
    Q_OBJECT
public:
    enum Buttons {
        BUTTON_KEY_ESCAPE = 0,
        BUTTON_KEY_F1,
        BUTTON_KEY_F2,
        BUTTON_KEY_F3,
        BUTTON_KEY_F4,
        BUTTON_KEY_F5,
        BUTTON_KEY_F6,
        BUTTON_KEY_F7,
        BUTTON_KEY_F8,
        BUTTON_KEY_F9,
        BUTTON_KEY_F10,
        BUTTON_KEY_F11,
        BUTTON_KEY_F12,
        BUTTON_KEY_TILDE,
        BUTTON_KEY_1,
        BUTTON_KEY_2,
        BUTTON_KEY_3,
        BUTTON_KEY_4,
        BUTTON_KEY_5,
        BUTTON_KEY_6,
        BUTTON_KEY_7,
        BUTTON_KEY_8,
        BUTTON_KEY_9,
        BUTTON_KEY_0,
        BUTTON_KEY_MINUS,
        BUTTON_KEY_PLUS,
        BUTTON_KEY_EQUAL,
        BUTTON_KEY_A,
        BUTTON_KEY_B,
        BUTTON_KEY_C,
        BUTTON_KEY_D,
        BUTTON_KEY_E,
        BUTTON_KEY_F,
        BUTTON_KEY_G,
        BUTTON_KEY_H,
        BUTTON_KEY_I,
        BUTTON_KEY_J,
        BUTTON_KEY_K,
        BUTTON_KEY_L,
        BUTTON_KEY_M,
        BUTTON_KEY_N,
        BUTTON_KEY_O,
        BUTTON_KEY_P,
        BUTTON_KEY_Q,
        BUTTON_KEY_R,
        BUTTON_KEY_S,
        BUTTON_KEY_T,
        BUTTON_KEY_U,
        BUTTON_KEY_V,
        BUTTON_KEY_W,
        BUTTON_KEY_X,
        BUTTON_KEY_Y,
        BUTTON_KEY_Z,
        BUTTON_KEY_BRACKET_OPEN,
        BUTTON_KEY_BRACKET_CLOSE,
        BUTTON_KEY_SEMICOLON,
        BUTTON_KEY_QUOTE,
        BUTTON_KEY_BACKSLASH,
        BUTTON_KEY_VBAR,
        BUTTON_KEY_COMMA,
        BUTTON_KEY_PERIOD,
        BUTTON_KEY_SLASH,
        BUTTON_KEY_QUESTION,
        BUTTON_KEY_TAB,
        BUTTON_KEY_CAPSLOCK,
        BUTTON_KEY_LSHIFT,
        BUTTON_KEY_LCONTROL,
        BUTTON_KEY_LCTRL,
        BUTTON_KEY_LALT,
        BUTTON_KEY_SPACE,
        BUTTON_KEY_RALT,
        BUTTON_KEY_LWIN,
        BUTTON_KEY_LSUPER,
        BUTTON_KEY_RWIN,
        BUTTON_KEY_RSUPER,
        BUTTON_KEY_APPS,
        BUTTON_KEY_MENU,
        BUTTON_KEY_RCTRL,
        BUTTON_KEY_RSHIFT,
        BUTTON_KEY_ENTER,
        BUTTON_KEY_RETURN,
        BUTTON_KEY_BACKSPACE,
        BUTTON_KEY_SCROLLLOCK,
        BUTTON_KEY_PAUSE,
        BUTTON_KEY_INSERT,
        BUTTON_KEY_HOME,
        BUTTON_KEY_PAGEUP,
        BUTTON_KEY_DELETE,
        BUTTON_KEY_END,
        BUTTON_KEY_PAGEDOWN,
        BUTTON_KEY_LEFT,
        BUTTON_KEY_UP,
        BUTTON_KEY_RIGHT,
        BUTTON_KEY_DOWN,
        BUTTON_KEY_NUMLOCK,
        BUTTON_KEY_DIVIDE,
        BUTTON_KEY_DIV,
        BUTTON_KEY_NUMSLASH,
        BUTTON_KEY_MULTIPLY,
        BUTTON_KEY_MUL,
        BUTTON_KEY_STAR,
        BUTTON_KEY_NUMSTAR,
        BUTTON_KEY_NUM7,
        BUTTON_KEY_NUM8,
        BUTTON_KEY_NUM9,
        BUTTON_KEY_NUMMINUS,
        BUTTON_KEY_NUM4,
        BUTTON_KEY_NUM5,
        BUTTON_KEY_NUM6,
        BUTTON_KEY_NUMPLUS,
        BUTTON_KEY_NUM1,
        BUTTON_KEY_NUM2,
        BUTTON_KEY_NUM3,
        BUTTON_KEY_NUM0,
        BUTTON_KEY_NUMDELETE,
        BUTTON_KEY_NUMDEL,
        BUTTON_KEY_NUMENTER,
        BUTTON_KEY_NUMRETURN

    };

    static const struct KeyDescription {
        QString text;
        Buttons button;
    } keyDescriptions[];


    explicit ShowButtons(DWidget *parent = nullptr);

private:
    void initButtons();

signals:
    void keyShowSignal(const QString &key);

public slots:
    void showContentButtons(unsigned char keyCode);
    void releaseContentButtons(unsigned char keyCode);
    QString getKeyCodeFromEvent(unsigned char keyCode);

private:
//    QMap<QString, DPushButton*> m_textButtonMap;
//    QString m_keyCode;//保存上次的输入按钮，防止长按
    QVector<QString> m_keyCodeVec;//保存上次的输入按钮，防止长按
};

#endif // SHOW_BUTTONS_H
