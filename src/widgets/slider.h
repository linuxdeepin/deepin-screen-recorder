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
        restoreSafeCursor();
    }

    void setSafeCursorManagement(bool enabled)
    {
        m_safeCursorManagement = enabled;
    }

protected:
    void enterEvent(QEnterEvent *e) override
    {
        if (m_safeCursorManagement) {
            // The TreeLand toolbar can replace the application cursor stack
            // while this slider is hovered. Never retain that raw pointer.
            if (!m_cursorOverridden) {
                qApp->setOverrideCursor(Qt::ArrowCursor);
                m_cursorOverridden = true;
            }
        } else {
            // Keep the original X11 behavior unchanged.
            m_lastCursorShape = qApp->overrideCursor();
            qApp->setOverrideCursor(Qt::ArrowCursor);
        }
        DSlider::enterEvent(e);
    }

    void leaveEvent(QEvent *e) override
    {
        if (m_safeCursorManagement) {
            restoreSafeCursor();
        } else if (m_lastCursorShape) {
            qApp->setOverrideCursor(m_lastCursorShape->shape());
        }
        DSlider::leaveEvent(e);
    }

private:
    void restoreSafeCursor()
    {
        if (m_safeCursorManagement && m_cursorOverridden) {
            qApp->restoreOverrideCursor();
            m_cursorOverridden = false;
        }
    }

    bool m_isMousePress = false;
    bool m_safeCursorManagement = false;
    bool m_cursorOverridden = false;
    QCursor *m_lastCursorShape = nullptr;
};
#endif // SLIDER_H
