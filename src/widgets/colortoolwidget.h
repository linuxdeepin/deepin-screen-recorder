// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORTOOLWIDGET_H
#define COLORTOOLWIDGET_H

#include "toolbutton.h"

#include <DStackedWidget>
#include <DLabel>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class ColorToolWidget : public DLabel
{
    Q_OBJECT
public:
    explicit ColorToolWidget(DWidget *parent = nullptr);
    ~ColorToolWidget();

    void initWidget();
    //录屏截屏控件按钮初始化
    void initColorLabel();

signals:
    void colorChecked(QString colorType);

public slots:
    void setFunction(const QString &func);

private:
    QVBoxLayout *m_baseLayout;
    bool m_isChecked;
    QString m_function;
    ToolButton *m_redBtn;
    ToolButton *m_yellowBtn;
    ToolButton *m_blueBtn;
    ToolButton *m_greenBtn;
};

#endif // COLORTOOLWIDGET_H
