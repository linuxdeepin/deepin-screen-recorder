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
#include "../utils/desktopinfo.h"

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
    void colorChanged(QColor color);
    void saveImage();
    void shapePressed(QString tool);
    void saveBtnPressed(SaveAction index);
    void saveSpecifiedPath();
    void closed();
    void changeFunctionSignal(QString shapeType);
    void keyBoardCheckedSignal(bool checked);
    void microphoneActionCheckedSignal(bool checked);
    void systemAudioActionCheckedSignal(bool checked);
    void cameraActionCheckedSignal(bool checked);
    void mouseCheckedSignalToToolBar(bool checked);
    void mouseShowCheckedSignalToToolBar(bool checked);
    void gifActionCheckedSignal(bool checked);
    void mp4ActionCheckedSignal(bool checked);
    void frameRateChangedSignal(int frameRate);
    void shotToolChangedSignal(const QString &func);
    void closeButtonSignal();
public slots:
    //bool isButtonChecked();
    void setExpand(bool expand, QString shapeType);
    //void specifiedSavePath();
    void keyBoardCheckedSlot(bool checked);
    void changeArrowAndLineFromBar(int line);
    void setRecordButtonDisableFromMain();
    void setRecordLaunchFromMain(bool recordLaunch);
    //void setIsZhaoxinPlatform(bool isZhaoxin);
    void setVideoInitFromMain();
    void shapeClickedFromBar(QString shape);
    void setMicroPhoneEnable(bool status);
    void setSystemAudioEnable(bool status);
    void setCameraDeviceEnable(bool status);

protected:
    void paintEvent(QPaintEvent *e)Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;


private:
    //MajToolBar *m_majToolbar;
    DLabel *m_hSeparatorLine;
    //SubToolBar *m_subToolbar;

    MainToolWidget *m_mainTool;
    SubToolWidget *m_subTool;
    DImageButton *m_closeButton;
    DesktopInfo m_deskinfo;

    //bool  m_expanded;
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
    void heightChanged();
    void buttonChecked(QString shape);
    void updateColor(QColor color);
    void requestSaveScreenshot();
    void shapePressed(QString tool);
    void saveBtnPressed(SaveAction index);
    void saveSpecifiedPath();
    void closed();
    void currentFunctionToMain(QString shapeType);
    void keyBoardCheckedToMain(bool checked);
    void microphoneActionCheckedToMain(bool checked);
    void systemAudioActionCheckedToMain(bool checked);
    void cameraActionCheckedToMain(bool checked);
    void mouseCheckedToMain(bool checked);
    void mouseShowCheckedToMain(bool checked);
    void gifActionCheckedToMain(bool checked);
    void mp4ActionCheckedToMain(bool checked);
    void frameRateChangedToMain(int frameRate);
    void shotToolChangedToMain(const QString &func);
    void closeButtonToMain();
public slots:
    //bool isButtonChecked();
    void setExpand(bool expand, QString shapeType);
    void showAt(QPoint pos);
//    void specificedSavePath();
    void currentFunctionMode(QString shapeType);
    void keyBoardCheckedToMainSlot(bool checked);
    void microphoneActionCheckedToMainSlot(bool checked);
    void systemAudioActionCheckedToMainSlot(bool checked);
    void changeArrowAndLineFromMain(int line);
    void setRecordButtonDisable();
    void setRecordLaunchMode(bool recordLaunch);
    //void setIsZhaoxinPlatform(bool isZhaoxin);
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

    //bool m_expanded;
};
#endif // TOOLBAR_H
