// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SLIDER_H
#define SLIDER_H

#include <DSlider>
#include <DWidget>

#include <QApplication>

DWIDGET_USE_NAMESPACE

class Slider : public DSlider
{
    Q_OBJECT
public:
    explicit Slider(DWidget *parent = 0)
    {
    }
    explicit Slider(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = nullptr)
        : DSlider(orientation, parent)
    {
    }

    ~Slider() override
    {
        restoreCursor();
    }

protected:
    void enterEvent(QEnterEvent *e) override
    {
        // Never retain QApplication::overrideCursor()'s pointer: the override
        // stack can be changed by sibling widgets, leaving that pointer
        // dangling. Pair the temporary arrow cursor with a restore instead.
        if (!m_cursorOverridden && qApp) {
            qApp->setOverrideCursor(Qt::ArrowCursor);
            m_cursorOverridden = true;
        }
        DSlider::enterEvent(e);
    }

    void leaveEvent(QEvent *e) override
    {
        restoreCursor();
        DSlider::leaveEvent(e);
    }

private:
    void restoreCursor()
    {
        if (m_cursorOverridden && qApp) {
            qApp->restoreOverrideCursor();
            m_cursorOverridden = false;
        }
    }

    bool m_isMousePress = false;
    bool m_cursorOverridden = false;
};
#endif // SLIDER_H
