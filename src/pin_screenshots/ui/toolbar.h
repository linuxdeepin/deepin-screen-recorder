/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     Zhang Wenchao <zhangwenchao@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "toolbarwidget.h"
#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class ToolBar : public QObject
{
    Q_OBJECT
public:
    explicit ToolBar(DWidget *parent = nullptr);
    void showAt(QPoint pos); //显示在点pos
    void setHiden(bool value  = true); //设置隐藏
    void shortcutOpoints(); //快捷键显示选项菜单
    QPair<int, int> getSaveInfo(); // 获取保存信息
    int toolBarWidth(); //工具栏的高
    int toolBarHeight(); //工具栏的宽
    void close(); //关闭工具栏
    bool isHidden(); //是否隐藏
    bool isActiveWindow(); //工具栏是否为活动窗口
signals:
    void sendOcrButtonClicked(); //ocr被点击
    void sendSaveButtonClicked(); // 保存按钮被点击
    void sendCloseButtonClicked();//关闭按钮被点击
protected:
    void initToolBar(DWidget *parent); // 初始化工具栏
    void moveAt(QPoint pos);
private:
    /**
     * @brief 工具栏
     */
    ToolBarWidget *m_toolbarWidget; //工具栏
    /**
     * @brief 保存按钮
     */
    DPushButton *m_saveButton; // 保存按钮
    QWidget *m_btWidget; //保存按钮背景窗
};

#endif // TOOLBAR_H
