/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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

#include "majtoolbar.h"
#include "subtoolbar.h"
#include "maintoolwidget.h"
#include "subtoolwidget.h"

#include <DLabel>
#include <DBlurEffectWidget>
#include <DImageButton>
#include <DFloatingWidget>
#include <DIconButton>

#include <QPainter>
#include <QEvent>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class MainWindow;
enum SaveAction : unsigned int;
class ToolBarWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit ToolBarWidget(MainWindow* pMainwindow,DWidget *parent = nullptr);
    ~ToolBarWidget() Q_DECL_OVERRIDE;
    /**
     * @brief 隐藏工具栏矩形、圆形、箭头、笔画、选项中裁切选项-显示光标
     */
    void hideSomeToolBtn();

    /**
     * @brief 设置禁止滚动截图
     */
    void setScrollShotDisabled();

signals:
    void buttonChecked(QString shapeType);
    void expandChanged(bool expand,  QString shapeType);
    void closed();
    /**
     * @brief 切换截图功能或者录屏功能的信号
     * @param shapeType : "record" or "shot"
     */
      void changeFunctionSignal(QString shapeType);
      void keyBoardCheckedSignal(bool checked);
      void mouseShowCheckedSignalToToolBar(bool checked);
public slots:
    /**
     * @brief 切换截图功能或者录屏功能
     * @param expand :
     * @param shapeType : "record" or "shot"
     */
    void setExpand(bool expand, QString shapeType);
    //void keyBoardCheckedSlot(bool checked);
    void changeArrowAndLineFromBar(int line);
    void setRecordButtonDisableFromMain();
    void setRecordLaunchFromMain(bool recordLaunch);
    void setVideoInitFromMain();
    void shapeClickedFromBar(QString shape);
    void setMicroPhoneEnable(bool status);
    void setSystemAudioEnable(bool status);
    void setCameraDeviceEnable(bool status);

private:
    DLabel *m_hSeparatorLine;
    MainToolWidget *m_mainTool;
    SubToolWidget *m_subTool;
    DImageButton *m_closeButton;
};

class ToolBar : public DLabel
{
    Q_OBJECT
public:
    explicit ToolBar(DWidget *parent = nullptr);
    ~ToolBar() Q_DECL_OVERRIDE;
    //public接口非slots
    void initToolBar(MainWindow* pmainWindow);

    /**
     * @brief 点击滚动截图时，工具栏隐藏一些按钮
     */
    void hideSomeToolBtn();

    /**
     * @brief 设置禁止滚动截图
     */
    void setScrollShotDisabled();

signals:
    void buttonChecked(QString shape);
    void currentFunctionToMain(QString shapeType);
    void mouseShowCheckedToMain(bool checked);
    void shotToolChangedToMain(const QString &func);
public slots:
    void setExpand(bool expand, QString shapeType);
    void showAt(QPoint pos);
//    void specificedSavePath();
    /**
     * @brief 切换截图功能或者录屏功能
     * @param shapeType : "record" or "shot"
     */
    void currentFunctionMode(QString shapeType);
    void changeArrowAndLineFromMain(int line);
    void setRecordButtonDisable();
    void setRecordLaunchMode(bool recordLaunch);
    void setVideoButtonInit();
    void shapeClickedFromMain(QString shape);
    void setMicroPhoneEnable(bool status);
    void setSystemAudioEnable(bool status);
    void setCameraDeviceEnable(bool status);
protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    ToolBarWidget *m_toolbarWidget;
    DPushButton *m_confirmButton;
};
#endif // TOOLBAR_H
