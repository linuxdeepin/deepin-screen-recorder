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
#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include <QHBoxLayout>

#include <DGuiApplicationHelper>
#include <DWidget>
#include <DLabel>
#include <DBlurEffectWidget>
#include <DImageButton>
#include <DFloatingWidget>
#include <DIconButton>
#include <DPushButton>
#include <DMenu>


DWIDGET_USE_NAMESPACE
class ToolBarWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit ToolBarWidget(DWidget *parent = nullptr);
    QPair<QString, QString> getSaveInfo(); //获取保存信息
public slots:
    void onOptionButtonClicked(); // 选项按钮被点击
    void onThemeTypeChange(DGuiApplicationHelper::ColorType themeType); //主题变化槽
signals:
    void signalOcrButtonClicked(); // ocr被点击
    void signalCloseButtonClicked();// 关闭按钮被点击
protected:
    void initToolBarWidget(); //初始化工具栏
private:
    /**
     * @brief OCR按钮
     */
    DPushButton *m_ocrButton;
    /**
     * @brief 选项按钮
     */
    DPushButton *m_pinOptionButton;
    /**
     * @brief 关闭按钮按钮
     */
    DImageButton *m_closeButton;
    /**
     * @brief 菜单选项
     */
    DMenu *m_optionMenu;
    /**
     * @brief 当前保存信息
     */
    QPair<QString, QString> m_SaveInfo;
};

#endif // TOOLBARWIDGET_H
