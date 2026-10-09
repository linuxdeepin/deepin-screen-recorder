// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLERECOGNIZINGWIDGET_H
#define TABLERECOGNIZINGWIDGET_H

#include "tableglasspanel.h"

#include <DLabel>

#include <DSpinner>

DWIDGET_USE_NAMESPACE

/**
 * @brief 表格识别中浮层
 *
 * 显示在贴图窗口中央的毛玻璃卡片：加载动画使用设计稿中的 DTK 加载动画
 * (DSpinner)，提示文字使用 DLabel，卡片的背景模糊/圆角/描边由 TableGlassPanel 提供。
 */
class TableRecognizingWidget : public TableGlassPanel
{
    Q_OBJECT
public:
    explicit TableRecognizingWidget(DWidget *parent = nullptr);
    ~TableRecognizingWidget() override;

    /**
     * @brief 在宿主窗口中央显示并开始动画
     */
    void startOn(QWidget *host);

    /**
     * @brief 宿主窗口变化时重新定位（尺寸不变）
     */
    void repositionOn(QWidget *host);

    /**
     * @brief 停止动画并隐藏
     */
    void stop();

private:
    /// 浅色/深色切换时刷新卡片填充、描边与提示文字颜色
    void applyThemeColors();

    DLabel *m_tipLabel = nullptr;
    DSpinner *m_spinner = nullptr;
};

#endif // TABLERECOGNIZINGWIDGET_H
