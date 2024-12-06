// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZOOMINDICATORGL_H
#define ZOOMINDICATORGL_H

#include <DLabel>
#include <DWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QOpenGLWidget>

DWIDGET_USE_NAMESPACE

class ZoomIndicatorGL : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit ZoomIndicatorGL(DWidget *parent = nullptr);
    ~ZoomIndicatorGL();

    void showMagnifier(QPoint pos);

protected:
    void paintGL() override;
    void paintEvent(QPaintEvent *event) override;

private:
    QRect m_globalRect;
    QRect m_centerRect;
    QBrush m_lastCenterPosBrush;
};

#endif // ZOOMINDICATORGL_H
