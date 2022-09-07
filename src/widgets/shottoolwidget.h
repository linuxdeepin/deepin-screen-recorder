// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHOTTOOLWIDGET_H
#define SHOTTOOLWIDGET_H

#include "toolbutton.h"

#include <DLabel>
#include <DStackedWidget>

#include <QObject>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class ShotToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit ShotToolWidget(DWidget *parent = nullptr);
    ~ShotToolWidget();

    void initWidget();
    //截屏矩形按钮二级控件初始化
    void initRectLabel();
    //截屏圆形按钮二级控件初始化
    void initCircLabel();
    //截屏直线按钮二级控件初始化
    void initLineLabel();
    //截屏画笔按钮二级控件初始化
    void initPenLabel();
    //截屏文本按钮二级控件初始化
    void initTextLabel();

signals:
    void changeArrowAndLine(int line); //0 for line, 1 for arrow

public slots:
    void switchContent(QString shapeType);
    void colorChecked(QString colorType);

private:
    DLabel *m_rectSubTool;
    DLabel *m_circSubTool;
    DLabel *m_lineSubTool;
    DLabel *m_penSubTool;
    DLabel *m_textSubTool;

    bool m_rectInitFlag;
    bool m_circInitFlag;
    bool m_lineInitFlag;
    bool m_penInitFlag;
    bool m_textInitFlag;

    ToolButton *m_blurRectButton;
    ToolButton *m_mosaicRectButton;
    ToolButton *m_blurCircButton;
    ToolButton *m_mosaicCircButton;

    QString m_currentType;
    bool m_arrowFlag;
    QButtonGroup *m_buttonGroup;
    QButtonGroup *m_thicknessBtnGroup;
    QButtonGroup *m_funcBtnGroup;
    QVBoxLayout *m_rectLayout;
};

#endif // SHOTTOOLWIDGET_H
