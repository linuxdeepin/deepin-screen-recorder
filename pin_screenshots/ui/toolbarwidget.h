// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H
#include "subtoolwidget.h"
#include <QHBoxLayout>

#include <DGuiApplicationHelper>
#include <DWidget>
#include <DLabel>
#include <DBlurEffectWidget>
#include <DImageButton>
#include <DFloatingWidget>



DWIDGET_USE_NAMESPACE
class ToolBarWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit ToolBarWidget(DWidget *parent = nullptr);
    QPair<int, int> getSaveInfo(); //获取保存信息
public slots:
    void onOptionButtonClicked(); // 选项按钮被点击
    void onThemeTypeChange(DGuiApplicationHelper::ColorType themeType); //主题变化槽
signals:
    void signalOcrButtonClicked(); // ocr被点击
    void signalCloseButtonClicked();// 关闭按钮被点击
protected:
    void initToolBarWidget(); //初始化工具栏
    /**
     * @brief 重写鼠标移动事件：解决工具栏可以被拖动的问题
     * 工具栏暂无鼠标移动事件
     * @param event
     */
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    SubToolWidget *m_subTool;
    DImageButton *m_closeButton;
};

#endif // TOOLBARWIDGET_H
