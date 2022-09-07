// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    explicit SubToolWidget(MainWindow *pmainwindow, DWidget *parent = nullptr);
    ~SubToolWidget();

    void initWidget();
    void initRecordLabel();
    void initShotLabel();
    /**
     * @brief 快捷键或命令行启动滚动截图时，初始化滚动截图工具栏
     */
    void initScrollLabel();
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
    void setScrollShotDisabled(const bool state);

    /**
     * @brief 设置贴图的激活状态
     */
    void setPinScreenshotsEnable(const bool &state);

    void setOcrScreenshotEnable(const bool &state);

    void setButEnableOnLockScreen(const bool &state);
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
    void setRecordLaunchMode(const unsigned int funType);
    void setVideoButtonInitFromSub();
    void shapeClickedFromWidget(QString shape);
    void setMicroPhoneEnable(bool status);
    void setCameraDeviceEnable(bool status);
    void setSystemAudioEnable(bool status);
    //void setIsZhaoxinPlatform(bool isZhaoxin);
    /**
     * @brief 当m_microphoneAction或m_systemAudioAction被点击或者程序主动调用trigg()时，会触发工具栏音频
     * 采集图标的改变及发射实际需要录制的音频
     * @param checked
     */
    void onChangeAudioType(bool checked);

protected:
    /**
     * @brief 此方法为保持切换录屏保存类型时不改变音频已经设置的选项
     */
    void changeRecordLaunchMode();

    /**
     * @brief 用来设置 当前录制制视频所采集的音频信息
     */
    void setRecordAudioType(bool setMicAudio, bool setSysAudio);
private:
    /**
     * @brief 录屏功能工具栏
     */
    DLabel *m_recordSubTool = nullptr;
    /**
     * @brief 截屏功能工具栏
     */
    DLabel *m_shotSubTool = nullptr;
    /**
     * @brief 滚动截图功能工具栏
     */
    DLabel *m_scrollShotSubTool = nullptr;
    QString m_currentType;
    /**
     * @brief 滚动截图工具栏按钮
     */
    ToolButton *m_scrollShotButton = nullptr;
    /**
     * @brief ocr识别工具栏按钮
     */
    ToolButton *m_ocrButton = nullptr;
    /**
     * @brief 贴图工具栏按钮
     */
    ToolButton *m_pinButton = nullptr;
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
    /**
     * @brief 采集麦克风音频功能
     *  注意： QAction 的trigger()函数会改变当前的checked状态
     */
    QAction *m_microphoneAction = nullptr;
    /**
     * @brief 采集系统音频功能
     *  注意： QAction 的trigger()函数会改变当前的checked状态
     */
    QAction *m_systemAudioAction = nullptr;
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
    bool t_saveGif;
    bool t_saveMkv;
};

#endif // SUBTOOLWIDGET_H
