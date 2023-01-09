// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUBTOOLWIDGET_H
#define SUBTOOLWIDGET_H
#include "toolbutton.h"

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
    enum SAVEPATH {
        CLIPBOARD = 0, //剪切板
        DESKTOP, //桌面
        PICTURES, //图片
        FOLDER, //指定位置
    };
    enum SAVEFORMAT {
        PNG = 0,
        JPG,
        BMP
    };
    explicit SubToolWidget(DWidget *parent = nullptr);
    QPair<int, int> getSaveInfo();
public slots:
    void onOptionButtonClicked(); // 选项按钮被点击
    void updateOptionChecked();
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
    ToolButton *m_ocrButton;
    /**
     * @brief 选项按钮
     */
    ToolButton *m_pinOptionButton;
    /**
     * @brief 菜单选项
     */
    DMenu *m_optionMenu;
    /**
     * @brief 当前保存信息
     */
    QPair<int, int> m_SaveInfo;
    QMap<int, QAction *> m_SavePathActions;
    QMap<int, QAction *> m_SaveFormatActions;
};

#endif // SUBTOOLWIDGET_H
