// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <DPushButton>
#include <DPalette>

#include <QApplication>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class ToolButton : public DPushButton
{
    Q_OBJECT
public:
    explicit ToolButton(DWidget *parent = 0)
    {
        Q_UNUSED(parent);
        setCheckable(true);
        m_tips = "";
        this->setFocusPolicy(Qt::NoFocus);
//        this->setFocusPolicy(Qt::NoFocus);
//        DPalette pa = this->palette();
//        pa.setColor(DPalette::Highlight, Qt::transparent);
//        this->setPalette(pa);
    }
    ~ToolButton() {}

public slots:
    void setTips(QString tips)
    {
        m_tips = tips;
    }

    QString getTips()
    {
        return m_tips;
    }

signals:
    void onEnter();
    void onExist();
    void onPress();

protected:
    void enterEvent(QEvent *e) override
    {
        emit onEnter();
        DPushButton::enterEvent(e);
    }

    void leaveEvent(QEvent *e) override
    {
        emit onExist();
        DPushButton::leaveEvent(e);
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        emit onPress();
        DPushButton::mousePressEvent(e);
    }

private:
    QString m_tips;

};
#endif // TOOLBUTTON_H
