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
#ifndef SHOTTOOLWIDGET_H
#define SHOTTOOLWIDGET_H

#include <QObject>
#include <DLabel>
#include <DStackedWidget>

#include "toolbutton.h"

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

private:
    DLabel *m_rectSubTool;
    DLabel *m_circSubTool;
    DLabel *m_lineSubTool;
    DLabel *m_penSubTool;
    DLabel *m_textSubTool;

    ToolButton *m_blurRectButton;
    ToolButton *m_mosaicRectButton;
    ToolButton *m_blurCircButton;
    ToolButton *m_mosaicCircButton;

    QString m_currentType;
    bool m_arrowFlag;
};

#endif // SHOTTOOLWIDGET_H
