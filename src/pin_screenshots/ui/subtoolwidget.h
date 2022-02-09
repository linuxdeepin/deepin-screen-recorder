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
#ifndef SUBTOOLWIDGET_H
#define SUBTOOLWIDGET_H
#include <QObject>
#include <QHBoxLayout>

#include <DGuiApplicationHelper>
#include <DLabel>
#include <DStackedWidget>
#include <DImageButton>
#include <DIconButton>
#include <DPushButton>
#include <DMenu>

DWIDGET_USE_NAMESPACE
class SubToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    enum SAVEINFO {
        DESKTOP = 1, //桌面
        PICTURES, //图片
        FOLDER, //指定位置
        CLIPBOARD, //剪切板
        PNG,
        JPG,
        BMP
    };
    explicit SubToolWidget(DWidget *parent = nullptr);
    QPair<int, int> getSaveInfo();
public slots:
    void onOptionButtonClicked(); // 选项按钮被点击
signals:
    void signalOcrButtonClicked(); // ocr被点击
    void signalCloseButtonClicked();// 关闭按钮被点击
protected:
    void initShotLable();
private:
    /**
     * @brief 贴图功能工具栏
     */
    DLabel *m_shotSubTool;
    /**
     * @brief OCR按钮
     */
    DPushButton *m_ocrButton;
    /**
     * @brief 选项按钮
     */
    DPushButton *m_pinOptionButton;
    /**
     * @brief 菜单选项
     */
    DMenu *m_optionMenu;
    /**
     * @brief 当前保存信息
     */
    QPair<int, int> m_SaveInfo;
};

#endif // SUBTOOLWIDGET_H
