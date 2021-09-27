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

#ifndef SUBTOOLWIDGET_H
#define SUBTOOLWIDGET_H

#include "toolbutton.h"
#include "../utils/saveutils.h"
#include "filter.h"

#include <DLabel>
#include <DStackedWidget>

#include <QObject>

DWIDGET_USE_NAMESPACE
class MainWindow;
class SubToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit SubToolWidget(MainWindow* pmainwindow,DWidget *parent = nullptr);
    ~SubToolWidget();

    void initWidget();
    void initRecordLabel();
    void initShotLabel();
//    void initVirtualCard();
    /**
     * @brief 安装工具栏的提示
     * @param w : 工具按钮
     * @param hintstr : 提示信息
     */
    void installTipHint(QWidget *w, const QString &hintstr);
    /**
     * @brief 安装工具栏提示的触发
     * @param w : 工具按钮
     * @param hint : 提示信息
     */
    void installHint(QWidget *w, QWidget *hint);

    /**
     * @brief 隐藏工具栏矩形、圆形、箭头、笔画、选项中裁切选项-显示光标
     */
    void hideSomeToolBtn();

    /**
     * @brief 设置禁止滚动截图
     */
    void setScrollShotDisabled();
signals:
    void keyBoardButtonClicked(bool checked);
    void mouseBoardButtonClicked(bool checked);
    void mouseShowButtonClicked(bool checked);
    void microphoneActionChecked(bool checked);
    void systemAudioActionChecked(bool checked);
    //void gifActionChecked(bool checked);
    //void mp4ActionChecked(bool checked);
    //void mkvActionChecked(bool checked);
    void cameraActionChecked(bool checked);
    //void videoFrameRateChanged(int frameRate);
    /**
     * @brief 截图功能选择的工具被改变
     * @param 工具的名称
     */
    void changeShotToolFunc(const QString &func);
    void saveMethodChanged(SaveAction saveact, const QString path);
    void saveToClipBoard(bool isClip);
public slots:
    /**
     * @brief 切换截图功能或者录屏功能
     * @param shapeType : "record" or "shot"
     */
    void switchContent(QString shapeType);
    //void systemAudioActionCheckedSlot(bool checked);
    void changeArrowAndLineFromSideBar(int line);
    void setRecordLaunchMode(bool recordLaunch);
    void setVideoButtonInitFromSub();
    void shapeClickedFromWidget(QString shape);
    void setMicroPhoneEnable(bool status);
    void setCameraDeviceEnable(bool status);
    void setSystemAudioEnable(bool status);
    //void setIsZhaoxinPlatform(bool isZhaoxin);
private:
    /**
     * @brief 录屏功能工具栏
     */
    DLabel *m_recordSubTool = nullptr;
    /**
     * @brief 截屏功能工具栏
     */
    DLabel *m_shotSubTool = nullptr;
    QString m_currentType;
    QAction *m_systemAudioAction = nullptr;
    /**
     * @brief 滚动截图工具栏按钮
     */
    ToolButton *m_scrollShotButton = nullptr;
    /**
     * @brief ocr识别工具栏按钮
     */
    ToolButton *m_ocrButton = nullptr;
    /**
     * @brief 截图功能中矩形工具按钮
     */
    ToolButton *m_rectButton = nullptr;
    /**
     * @brief 截图功能中圆形工具按钮
     */
    ToolButton *m_circleButton = nullptr;
    /**
     * @brief 截图功能中箭头工具按钮
     */
    ToolButton *m_lineButton = nullptr;
    /**
     * @brief 截图功能中画笔工具按钮
     */
    ToolButton *m_penButton = nullptr;
    /**
     * @brief 截图功能中文本编辑工具按钮
     */
    ToolButton *m_textButton = nullptr;
    /**
     * @brief 录屏功能中摄像头工具按钮
     */
    ToolButton *m_cameraButton = nullptr;
    /**
     * @brief 录屏功能中键盘工具按钮
     */
    ToolButton *m_keyBoardButton = nullptr;
    /**
     * @brief 录屏功能中鼠标工具按钮
     */
    ToolButton *m_mouseButton = nullptr;
    /**
     * @brief 录屏功能中选项按钮
     */
    ToolButton *m_optionButton = nullptr;
    /**
     * @brief 截图功能中选项按钮
     */
    ToolButton *m_shotOptionButton = nullptr;
    QAction *m_microphoneAction = nullptr;
    /**
     * @brief 录屏功能中音频工具按钮
     */
    ToolButton *m_audioButton = nullptr;
    QAction *m_recorderCheck = nullptr;
    QAction *m_recorderMouse = nullptr;

    /**
     * @brief 截图功能选项按钮打开的二级菜单中的选项
     */
    QAction *m_clipTitleAction = nullptr;
    /**
     * @brief 截图选项中的显示光标
     */
    QAction *m_saveCursorAction = nullptr;

    bool m_haveMicroPhone = false;
    bool m_haveSystemAudio = false;
    int m_lineflag;
    HintFilter *hintFilter = nullptr;
    DMenu *m_optionMenu = nullptr;
    DMenu *m_audioMenu = nullptr;
    DMenu *m_cursorMenu = nullptr;
    DMenu *m_recordOptionMenu = nullptr;

    QButtonGroup *m_recordBtnGroup = nullptr;
    QButtonGroup *m_shotBtnGroup = nullptr;

    MainWindow *m_pMainWindow = nullptr;

};

#endif // SUBTOOLWIDGET_H
