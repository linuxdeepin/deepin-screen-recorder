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

#include "subtoolwidget.h"
#include "../utils/audioutils.h"
#include "../camera_process.h"
#include "../utils/configsettings.h"
#include "../settings.h"
#include "tooltips.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"

#include <DSlider>
#include <DLineEdit>
#include <DMenu>
#include <DInputDialog>
#include <DFontSizeManager>
#include <DComboBox>
#include <DListWidget>

#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QThread>

#include <unistd.h>


DWIDGET_USE_NAMESPACE

namespace {
const int BUTTON_SPACING = 4;
const int SHOT_BUTTON_SPACING = 4;
const QSize TOOL_ICON_SIZE = QSize(25, 25);
const QSize MAX_TOOL_ICON_SIZE = QSize(40, 40);
const QSize MEDIUM_TOOL_BUTTON_SIZE = QSize(52, 40);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(42, 40);
}

SubToolWidget::SubToolWidget(MainWindow *pmainwindow, DWidget *parent) : DStackedWidget(parent)
{
    m_pMainWindow = pmainwindow;
    initWidget();
//    this->setAttribute(Qt::WA_StyledBackground,true);
//    this->setStyleSheet("background-color: rgb(255,122, 255)");
}



void SubToolWidget::initWidget()
{
    t_saveGif = false;
    t_saveMkv = false;
    hintFilter = new HintFilter(this);
    initRecordLabel();
    initShotLabel();
    setCurrentWidget(m_shotSubTool);
}

void SubToolWidget::initRecordLabel()
{
    m_recordSubTool = new DLabel(this);
    //QButtonGroup *rectBtnGroup = new QButtonGroup();
    m_recordBtnGroup = new QButtonGroup(this);
    m_recordBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;
    DPalette pa;

    int t_frameRate = 0;

    ConfigSettings *t_settings = ConfigSettings::instance();
    QVariant t_saveGifVar = t_settings->value("recordConfig", "save_as_gif");
    QVariant t_saveMkvVar = t_settings->value("recordConfig", "lossless_recording");
    QVariant t_frameRateVar = t_settings->value("recordConfig", "mkv_framerate");

    //保持格式的配置文件判断
    if (t_saveGifVar.toString() == "true") {
        t_saveGif = true;
    } else if (t_saveGifVar.toString() == "false") {
        t_saveGif = false;
    } else {
        t_settings->setValue("recordConfig", "save_as_gif", true);
        t_saveGif = true;
    }

    if (t_saveMkvVar.toString() == "true") {
        t_saveMkv = true;
    } else if (t_saveMkvVar.toString() == "false") {
        t_saveMkv = false;
    }

    // mips sw不支持GIF录制
#if defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__)
    t_settings->setValue("recordConfig", "save_as_gif", false);
    t_saveGif = false;
#endif

    //保持帧数的配置文件判断
    t_frameRate = t_frameRateVar.toString().toInt();
    //    //添加音频按钮
    m_audioButton = new ToolButton(this);

    //audioButton->setObjectName("AudioButton");
    Utils::setAccessibility(m_audioButton, AC_SUBTOOLWIDGET_AUDIO_BUTTON);
    m_audioButton->setText(" ");
    m_audioButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_audioButton, tr("Sound On"));
    m_audioButton->setIcon(QIcon::fromTheme("microphone_normal"));

    m_recordBtnGroup->addButton(m_audioButton);
    m_audioButton->setFixedSize(MEDIUM_TOOL_BUTTON_SIZE);
    btnList.append(m_audioButton);

    m_audioMenu = new DMenu(this);

    DFontSizeManager::instance()->bind(m_audioMenu, DFontSizeManager::T8);
    m_microphoneAction = new QAction(m_audioMenu);
    m_systemAudioAction = new QAction(m_audioMenu);
    m_microphoneAction->setText(tr("Microphone"));
    m_microphoneAction->setCheckable(true);
    m_microphoneAction->setChecked(false);
    m_systemAudioAction->setText(tr("System Audio"));
    m_systemAudioAction->setCheckable(true);
    m_systemAudioAction->setChecked(false);

    //麦克风和系统声卡初始状态默认为失能
    m_microphoneAction->setEnabled(false);
    m_haveMicroPhone = false;
    m_systemAudioAction->setEnabled(false);
    m_haveSystemAudio = false;
    if (Utils::themeType == 1) {
        m_microphoneAction->setIcon(QIcon(":/newUI/normal/microphone.svg"));
        m_systemAudioAction->setIcon(QIcon(":/newUI/normal/audio frequency.svg"));
    } else {
        m_microphoneAction->setIcon(QIcon(":/newUI/dark/normal/microphone.svg"));
        m_systemAudioAction->setIcon(QIcon(":/newUI/dark/normal/audio frequency.svg"));
    }



    //当m_microphoneAction被点击或者程序主动调用trigg()时，信号会发送
    connect(m_microphoneAction, &QAction::triggered, this, &SubToolWidget::onChangeAudioType);

    //当m_systemAudioAction被点击或者程序主动调用trigg()时，信号会发送
    connect(m_systemAudioAction, &QAction::triggered, this, &SubToolWidget::onChangeAudioType);

//    m_haveMicroPhone = true;
//    m_microphoneAction->setCheckable(true);
//    m_microphoneAction->trigger();
//    m_systemAudioAction->setCheckable(true);
//    m_systemAudioAction->trigger();
//    m_haveSystemAudio = true;

    //    m_systemAudioAction->setDisabled(!AudioUtils().canVirtualCardOutput());
    m_audioMenu->addAction(m_microphoneAction);
    m_audioMenu->addSeparator();
    m_audioMenu->addAction(m_systemAudioAction);
    m_audioButton->setMenu(m_audioMenu);

    m_keyBoardButton = new ToolButton();

    //m_keyBoardButton->setObjectName("KeyBoardButton");
    Utils::setAccessibility(m_keyBoardButton, AC_SUBTOOLWIDGET_KEYBOARD_BUTTON);
    m_keyBoardButton->setIconSize(MAX_TOOL_ICON_SIZE);
    installTipHint(m_keyBoardButton, tr("Show Keystroke"));
    m_keyBoardButton->setIcon(QIcon::fromTheme("key_mormal"));


    m_recordBtnGroup->addButton(m_keyBoardButton);
    m_keyBoardButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_keyBoardButton);

    //发送键盘按键按钮状态信号
    connect(m_keyBoardButton, SIGNAL(clicked(bool)),
            this, SIGNAL(keyBoardButtonClicked(bool)));

    connect(m_keyBoardButton, &DPushButton::clicked, this, [ = ] {
        if (m_keyBoardButton->isChecked())
        {
            installTipHint(m_keyBoardButton, tr("Hide Keystroke"));
        } else
        {
            installTipHint(m_keyBoardButton, tr("Show Keystroke"));
        }
    });

    m_cameraButton = new ToolButton();
    m_cameraButton->setDisabled((QCameraInfo::availableCameras().count() <= 0));

    //m_cameraButton->setObjectName("CameraButton");
    Utils::setAccessibility(m_cameraButton, AC_SUBTOOLWIDGET_CAMERA_BUTTON);
    m_cameraButton->setIconSize(MAX_TOOL_ICON_SIZE);
    installTipHint(m_cameraButton, tr("Webcam On"));

    m_cameraButton->setIcon(QIcon::fromTheme("webcam_normal"));
    m_recordBtnGroup->addButton(m_cameraButton);
    m_cameraButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_cameraButton);

    connect(m_cameraButton, &DPushButton::clicked, this, [ = ] {
        if (m_cameraButton->isChecked())
        {
            installTipHint(m_cameraButton, tr("Webcam Off"));
        }

        if (!m_cameraButton->isChecked())
        {
            installTipHint(m_cameraButton, tr("Webcam On"));
        }
        emit cameraActionChecked(m_cameraButton->isChecked());
    });
    /*
    m_mouseButton = new ToolButton();
    //m_mouseButton->setObjectName("MouseButton");
    Utils::setAccessibility(m_mouseButton, AC_SUBTOOLWIDGET_MOUSE_BUTTON);
    m_mouseButton->setIconSize(MAX_TOOL_ICON_SIZE);
    m_mouseButton->setIcon(QIcon::fromTheme("mouse_mormal"));
    installTipHint(m_mouseButton, tr("Show Click"));
    rectBtnGroup->addButton(m_mouseButton);

    m_mouseButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_mouseButton);

    connect(m_mouseButton, &DPushButton::clicked, this, [ = ] {
        if (m_mouseButton->isChecked()){
            installTipHint(m_mouseButton, tr("Hide Click"));
        }

        if (!m_mouseButton->isChecked()){
            installTipHint(m_mouseButton, tr("Show Click"));
        }
    });

    //发送鼠标按键按钮状态信号
    connect(m_mouseButton, SIGNAL(clicked(bool)),
            this, SIGNAL(mouseBoardButtonClicked(bool)));
    */
    // 新增光标选项
    m_mouseButton = new ToolButton();

    //audioButton->setObjectName("AudioButton");
    Utils::setAccessibility(m_mouseButton, AC_SUBTOOLWIDGET_MOUSE_BUTTON);
    m_mouseButton->setText(" ");
    m_mouseButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(m_mouseButton, tr("Mouse"));
    m_mouseButton->setIcon(QIcon::fromTheme("mouse2"));
    m_recordBtnGroup->addButton(m_mouseButton);
    m_mouseButton->setFixedSize(MEDIUM_TOOL_BUTTON_SIZE);
    btnList.append(m_mouseButton);

    m_cursorMenu = new DMenu(this);
    DFontSizeManager::instance()->bind(m_cursorMenu, DFontSizeManager::T8);
    m_recorderMouse = new QAction(m_cursorMenu);
    m_recorderCheck = new QAction(m_cursorMenu);

    m_recorderMouse->setText(tr("Show Pointer"));
    m_recorderMouse->setCheckable(true);
    m_recorderMouse->setChecked(true);
    m_recorderCheck->setText(tr("Show Click"));
    m_recorderCheck->setCheckable(true);

    if (Utils::themeType == 1) {
        m_recorderMouse->setIcon(QIcon(":/newUI/normal/mouse2.svg"));
        m_recorderCheck->setIcon(QIcon(":/newUI/normal/touch.svg"));
    } else {
        m_recorderMouse->setIcon(QIcon(":/newUI/dark/normal/mouse2.svg"));
        m_recorderCheck->setIcon(QIcon(":/newUI/dark/normal/touch.svg"));
    }



    m_cursorMenu->addAction(m_recorderMouse);
    m_cursorMenu->addAction(m_recorderCheck);


    connect(m_recorderMouse, &QAction::triggered, this, [ = ] {
        emit mouseShowButtonClicked(m_recorderMouse->isChecked());
        if (m_recorderCheck->isChecked() && m_recorderMouse->isChecked())
        {
            m_mouseButton->setIcon(QIcon::fromTheme("mouseandtouch"));
        }
        if (!m_recorderCheck->isChecked() && !m_recorderMouse->isChecked())
        {
            m_mouseButton->setIcon(QIcon::fromTheme("hide"));
        }
        if (!m_recorderCheck->isChecked() && m_recorderMouse->isChecked())
        {
            m_mouseButton->setIcon(QIcon::fromTheme("mouse2"));
        }
        if (m_recorderCheck->isChecked() && !m_recorderMouse->isChecked())
        {
            m_mouseButton->setIcon(QIcon::fromTheme("touch"));
        }
    });

    connect(m_recorderCheck, &QAction::triggered, this, [ = ] {
        emit mouseBoardButtonClicked(m_recorderCheck->isChecked());
        if (m_recorderCheck->isChecked() && m_recorderMouse->isChecked())
        {
            m_mouseButton->setIcon(QIcon::fromTheme("mouseandtouch"));
        }
        if (!m_recorderCheck->isChecked() && !m_recorderMouse->isChecked())
        {
            m_mouseButton->setIcon(QIcon::fromTheme("hide"));
        }
        if (!m_recorderCheck->isChecked() && m_recorderMouse->isChecked())
        {
            m_mouseButton->setIcon(QIcon::fromTheme("mouse2"));
        }
        if (m_recorderCheck->isChecked() && !m_recorderMouse->isChecked())
        {
            m_mouseButton->setIcon(QIcon::fromTheme("touch"));
        }
    });

    m_mouseButton->setMenu(m_cursorMenu);


    //2019-10-14：新增选项按钮
    m_optionButton = new ToolButton();
    DFontSizeManager::instance()->bind(m_optionButton, DFontSizeManager::T8);
    Utils::setAccessibility(m_optionButton, AC_SUBTOOLWIDGET_RECORD_OPTION_BUT);
    m_optionButton->setText(tr("Options"));
    m_optionButton->setMinimumSize(QSize(73, 40));
    installTipHint(m_optionButton, tr("Options"));
    m_recordBtnGroup->addButton(m_optionButton);

    btnList.append(m_optionButton);

    QActionGroup *t_formatGroup = new QActionGroup(this);
    QActionGroup *t_fpsGroup = new QActionGroup(this);
    t_formatGroup->setExclusive(true);
    t_fpsGroup->setExclusive(true);

    m_recordOptionMenu = new DMenu(this);
    DFontSizeManager::instance()->bind(m_recordOptionMenu, DFontSizeManager::T8);
    //for test
    QAction *formatTitleAction = new QAction(m_recordOptionMenu);
    QAction *gifAction = new QAction(m_recordOptionMenu);
    QAction *mp4Action = new QAction(m_recordOptionMenu);
    QAction *mkvAction = new QAction(m_recordOptionMenu);
    QAction *fpsTitleAction = new QAction(m_recordOptionMenu);
    QAction *fps5Action = new QAction(m_recordOptionMenu);
    QAction *fps10Action = new QAction(m_recordOptionMenu);
    QAction *fps20Action = new QAction(m_recordOptionMenu);
    QAction *fps24Action = new QAction(m_recordOptionMenu);
    QAction *fps30Action = new QAction(m_recordOptionMenu);


    Utils::setAccessibility(gifAction, "gifAction");
    Utils::setAccessibility(mp4Action, "mp4Action");
    Utils::setAccessibility(mkvAction, "mkvAction");
    Utils::setAccessibility(fps5Action, "fps5Action");
    Utils::setAccessibility(fps10Action, "fps10Action");
    Utils::setAccessibility(fps20Action, "fps20Action");
    Utils::setAccessibility(fps24Action, "fps24Action");
    Utils::setAccessibility(fps30Action, "fps30Action");

    formatTitleAction->setDisabled(true);
    formatTitleAction->setText(tr("Format:"));
    gifAction->setText(tr("GIF"));
    gifAction->setCheckable(true);
    mp4Action->setText(tr("MP4"));
    mp4Action->setCheckable(true);
    mkvAction->setText(tr("MKV"));
    mkvAction->setCheckable(true);
    t_formatGroup->addAction(gifAction);
    t_formatGroup->addAction(mp4Action);
    t_formatGroup->addAction(mkvAction);

    fpsTitleAction->setDisabled(true);
    fpsTitleAction->setText(tr("FPS:"));
    fps5Action->setText(tr("5 fps"));
    fps5Action->setCheckable(true);
    fps10Action->setText(tr("10 fps"));
    fps10Action->setCheckable(true);
    fps20Action->setText(tr("20 fps"));
    fps20Action->setCheckable(true);
    fps24Action->setText(tr("24 fps"));
    fps24Action->setCheckable(true);
    fps30Action->setText(tr("30 fps"));
    fps30Action->setCheckable(true);

    t_fpsGroup->addAction(fps5Action);
    t_fpsGroup->addAction(fps10Action);
    t_fpsGroup->addAction(fps20Action);
    t_fpsGroup->addAction(fps24Action);
    t_fpsGroup->addAction(fps30Action);

    m_recordOptionMenu->addAction(formatTitleAction);
#if !(defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__))
    m_recordOptionMenu->addAction(gifAction);
#endif
    m_recordOptionMenu->addAction(mp4Action);

    m_recordOptionMenu->addAction(mkvAction);
    m_recordOptionMenu->addSeparator();

    m_recordOptionMenu->addAction(fpsTitleAction);
    m_recordOptionMenu->addAction(fps5Action);
    m_recordOptionMenu->addAction(fps10Action);
    m_recordOptionMenu->addAction(fps20Action);
    m_recordOptionMenu->addAction(fps24Action);
    m_recordOptionMenu->addAction(fps30Action);
    m_recordOptionMenu->hide();
    m_optionButton->setMenu(m_recordOptionMenu);

    // change by hmy

    if (t_saveGif == true) {
        gifAction->setChecked(true);
        gifAction->trigger();
        fps5Action->setEnabled(false);
        fps10Action->setEnabled(false);
        fps20Action->setEnabled(false);
        fps24Action->setEnabled(false);
        fps30Action->setEnabled(false);
        m_microphoneAction->setEnabled(false);
        m_microphoneAction->setChecked(true);
        m_microphoneAction->trigger();
        m_systemAudioAction->setEnabled(false);
        m_systemAudioAction->setChecked(false);
        m_systemAudioAction->trigger();
        m_audioButton->setEnabled(false);

    } else if (t_saveMkv == true) {
        mkvAction->setChecked(true);
        mkvAction->trigger();
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
        m_audioButton->setEnabled(true);
        if (m_haveMicroPhone) {
            m_microphoneAction->setEnabled(true);
            m_microphoneAction->setChecked(false);
            m_microphoneAction->trigger();
        }

        if (m_haveSystemAudio) {
            m_systemAudioAction->setEnabled(true);
            m_systemAudioAction->setChecked(false);
            m_systemAudioAction->trigger();
        }

    } else {
        mp4Action->setChecked(true);
        mp4Action->trigger();
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
        m_audioButton->setEnabled(true);
        if (m_haveMicroPhone) {
            m_microphoneAction->setEnabled(true);
            m_microphoneAction->setChecked(false);
            //trigger()函数会改变当前的checked状态
            m_microphoneAction->trigger();
        }

        if (m_haveSystemAudio) {
            m_systemAudioAction->setEnabled(true);
            m_microphoneAction->setChecked(false);
            //trigger()函数会改变当前的checked状态
            m_microphoneAction->trigger();
        }

    }


    connect(gifAction, &QAction::triggered, this, [ = ](bool checked) {
        Q_UNUSED(checked);
        t_saveGif = true;
        t_saveMkv = false;
        t_settings->setValue("recordConfig", "lossless_recording", false);
        t_settings->setValue("recordConfig", "save_as_gif", true);
        fps5Action->setEnabled(false);
        fps10Action->setEnabled(false);
        fps20Action->setEnabled(false);
        fps24Action->setEnabled(false);
        fps30Action->setEnabled(false);
        m_audioButton->setEnabled(false);
        changeRecordLaunchMode();
    });

    connect(mp4Action, &QAction::triggered, this, [ = ](bool checked) {
        Q_UNUSED(checked);
        t_saveGif = false;
        t_saveMkv = false;
        t_settings->setValue("recordConfig", "lossless_recording", false);
        t_settings->setValue("recordConfig", "save_as_gif", false);
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
        m_audioButton->setEnabled(true);
        changeRecordLaunchMode();
    });

    connect(mkvAction, &QAction::triggered, this, [ = ](bool checked) {
        Q_UNUSED(checked);
        t_saveGif = false;
        t_saveMkv = true;
        t_settings->setValue("recordConfig", "lossless_recording", true);
        t_settings->setValue("recordConfig", "save_as_gif", false);
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
        m_audioButton->setEnabled(true);
        changeRecordLaunchMode();
    });

    connect(t_fpsGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        //int t_frameRateSelected = 0;
        if (t_act == fps5Action) {
            //t_frameRateSelected = 5;
            t_settings->setValue("recordConfig", "mkv_framerate", "5");
        } else if (t_act == fps10Action) {
            //t_frameRateSelected = 10;
            t_settings->setValue("recordConfig", "mkv_framerate", "10");
        } else if (t_act == fps20Action) {
            //t_frameRateSelected = 20;
            t_settings->setValue("recordConfig", "mkv_framerate", "20");
        } else if (t_act == fps24Action) {
            //t_frameRateSelected = 24;
            t_settings->setValue("recordConfig", "mkv_framerate", "24");
        } else if (t_act == fps30Action) {
            //t_frameRateSelected = 30;
            t_settings->setValue("recordConfig", "mkv_framerate", "30");
        }

        //emit videoFrameRateChanged(t_frameRateSelected);
    });
    switch (t_frameRate) {
    case 5:
        fps5Action->triggered();
        fps5Action->setChecked(true);
        break;
    case 10:
        fps10Action->triggered();
        fps10Action->setChecked(true);
        break;
    case 20:
        fps20Action->triggered();
        fps20Action->setChecked(true);
        break;
    case 24:
        fps24Action->triggered();
        fps24Action->setChecked(true);
        break;
    case 30:
        fps30Action->triggered();
        fps30Action->setChecked(true);
        break;
    default:
        fps24Action->triggered();
        fps24Action->setChecked(true);
        break;
    }

    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(7);
    //    rectLayout->addWidget(audioButton);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(BUTTON_SPACING);
    }
    m_recordSubTool->setLayout(rectLayout);
    addWidget(m_recordSubTool);
}

void SubToolWidget::initShotLabel()
{
    m_shotSubTool = new DLabel(this);

    //QButtonGroup *rectBtnGroup = new QButtonGroup();
    m_shotBtnGroup = new QButtonGroup(this);
    m_shotBtnGroup->setExclusive(true);

    QList<ToolButton *> btnList;
    DPalette pa;

    //分割线
    ToolButton *seperator1 = new ToolButton(this);
    qDebug() << "Utils::themeType: " << Utils::themeType;
    if (Utils::themeType != 1) {
        seperator1->setStyleSheet("border:0px solid rgba(0, 0, 0, 0);border-radius:0px;background-color:rgba(0, 0, 0, 0)");
    } else {
        seperator1->setStyleSheet("border:0px solid rgba(255, 255, 255, 0);border-radius:0px;background-color:rgba(255, 255, 255, 0)");
    }
    seperator1->setDisabled(true);
    seperator1->setFixedSize(QSize(1, 30));
    //添加分割线
    btnList.append(seperator1);

    //添加贴图按钮
    m_pinButton = new ToolButton();
    m_pinButton->setIconSize(QSize(35, 35));
    m_pinButton->setIcon(QIcon::fromTheme("pinscreenshots"));
    Utils::setAccessibility(m_pinButton, AC_SUBTOOLWIDGET_PINSCREENSHOTS_BUTTON);
    m_shotBtnGroup->addButton(m_pinButton);
    m_pinButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    installTipHint(m_pinButton, tr("Pin Screenshots"));
    btnList.append(m_pinButton);

    //添加滚动截图按钮
    m_scrollShotButton = new ToolButton();
    m_scrollShotButton->setIconSize(QSize(35, 35));
    m_scrollShotButton->setIcon(QIcon::fromTheme("scrollShot"));
    Utils::setAccessibility(m_scrollShotButton, AC_SUBTOOLWIDGET_SCROLLSHOT_BUTTON);
    m_shotBtnGroup->addButton(m_scrollShotButton);
    m_scrollShotButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    installTipHint(m_scrollShotButton, tr("Scrollshot"));
#ifdef  OCR_SCROLL_FLAGE_ON
    btnList.append(m_scrollShotButton);
#endif
    //添加ocr图文识别按钮
    m_ocrButton = new ToolButton();
    m_ocrButton->setIconSize(QSize(35, 35));
    m_ocrButton->setIcon(QIcon::fromTheme("ocr-normal"));
    Utils::setAccessibility(m_ocrButton, AC_SUBTOOLWIDGET_OCR_BUTTON);
    m_shotBtnGroup->addButton(m_ocrButton);
    m_ocrButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    installTipHint(m_ocrButton, tr("Extract Text"));
#ifdef  OCR_SCROLL_FLAGE_ON
    btnList.append(m_ocrButton);
#endif

    ToolButton *seperator2 = new ToolButton(this);
    if (Utils::themeType != 1) {
        seperator2->setStyleSheet("border:0px solid rgba(0, 0, 0, 0);border-radius:0px;background-color:rgba(0, 0, 0, 0)");
    } else {
        seperator2->setStyleSheet("border:0px solid rgba(255, 255, 255, 0);border-radius:0px;background-color:rgba(255, 255, 255, 0)");
    }
    seperator2->setDisabled(true);
    seperator2->setFixedSize(QSize(3, 30));
    //添加分割线
    btnList.append(seperator2);

    //添加矩形按钮
    m_rectButton = new ToolButton();
    m_rectButton->setIconSize(QSize(35, 35));

    m_rectButton->setIcon(QIcon::fromTheme("rectangle-normal"));
    //m_rectButton->setObjectName("RectButton");
    Utils::setAccessibility(m_rectButton, AC_SUBTOOLWIDGET_RECT_BUTTON);
    m_shotBtnGroup->addButton(m_rectButton);
    m_rectButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    installTipHint(m_rectButton, tr("Rectangle"));
    btnList.append(m_rectButton);

    m_circleButton = new ToolButton();
    m_circleButton->setIconSize(QSize(35, 35));
    m_circleButton->setIcon(QIcon::fromTheme("oval-normal"));
    installTipHint(m_circleButton, tr("Ellipse"));
    //m_circleButton->setObjectName("CircleButton");
    Utils::setAccessibility(m_circleButton, AC_SUBTOOLWIDGET_CIRCL_BUTTON);

    m_shotBtnGroup->addButton(m_circleButton);
    m_circleButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_circleButton);

    m_lineButton = new ToolButton();
    m_lineButton->setIconSize(QSize(35, 35));



    bool t_arrowStatus = ConfigSettings::instance()->value("arrow", "is_straight").toBool();
    if (t_arrowStatus) {
        installTipHint(m_lineButton, tr("Line"));
        m_lineflag = 0;
        m_lineButton->setIcon(QIcon::fromTheme("line-normal"));
    } else {
        installTipHint(m_lineButton, tr("Arrow"));
        m_lineflag = 1;
        m_lineButton->setIcon(QIcon::fromTheme("Arrow-normal"));
    }
    //m_lineButton->setObjectName("LineButton");
    Utils::setAccessibility(m_lineButton, AC_SUBTOOLWIDGET_LINE_BUTTON);
    m_shotBtnGroup->addButton(m_lineButton);
    m_lineButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_lineButton);

    m_penButton = new ToolButton();
    m_penButton->setIconSize(QSize(35, 35));
    installTipHint(m_penButton, tr("Pencil"));
    m_penButton->setIcon(QIcon::fromTheme("Combined Shape-normal"));
    //m_penButton->setObjectName("PenButton");
    Utils::setAccessibility(m_penButton, AC_SUBTOOLWIDGET_PEN_BUTTON);
    m_shotBtnGroup->addButton(m_penButton);
    m_penButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_penButton);

    m_textButton = new ToolButton();
    m_textButton->setIconSize(QSize(30, 30));
    m_textButton->setIcon(QIcon::fromTheme("text"));
    if (Utils::isTabletEnvironment) {
        m_textButton->hide();
    }
    //    m_textButton->setToolTip(tr("Text"));
    installTipHint(m_textButton, tr("Text"));
    //m_textButton->setObjectName("TextButton");
    Utils::setAccessibility(m_textButton, AC_SUBTOOLWIDGET_TEXT_BUTTON);
    m_shotBtnGroup->addButton(m_textButton);
    m_textButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_textButton);

    ToolButton *seperator3 = new ToolButton(this);
    if (Utils::themeType != 1) {
        seperator3->setStyleSheet("border:0px solid rgba(0, 0, 0, 0);border-radius:0px;background-color:rgba(0, 0, 0, 0)");
    } else {
        seperator3->setStyleSheet("border:0px solid rgba(255, 255, 255, 0);border-radius:0px;background-color:rgba(255, 255, 255, 0)");
    }
    seperator3->setDisabled(true);
    seperator3->setFixedSize(QSize(3, 30));
    //添加分割线
    btnList.append(seperator3);

    //2019-10-15：添加截图选项按钮
    m_shotOptionButton = new ToolButton();
    if (Utils::isTabletEnvironment && nullptr != m_shotOptionButton) {
        m_shotOptionButton->hide();
    }
    DFontSizeManager::instance()->bind(m_shotOptionButton, DFontSizeManager::T8);
    m_shotOptionButton->setText(tr("Options"));
    Utils::setAccessibility(m_shotOptionButton, AC_SUBTOOLWIDGET_SHOT_OPTION_BUT);
    m_shotOptionButton->setMinimumSize(QSize(73, 40));
    installTipHint(m_shotOptionButton, tr("Options"));
    m_shotBtnGroup->addButton(m_shotOptionButton);
    if (Utils::is3rdInterfaceStart) {
        m_shotOptionButton->hide();
        m_scrollShotButton->hide(); //隐藏滚动截图按钮
        m_ocrButton->hide(); //隐藏ocr按钮
    }
    btnList.append(m_shotOptionButton);

    QActionGroup *t_saveGroup = new QActionGroup(this);
    QActionGroup *t_formatGroup = new QActionGroup(this);
    t_saveGroup->setExclusive(true);
    t_formatGroup->setExclusive(true);

    m_optionMenu = new DMenu(this);

    DFontSizeManager::instance()->bind(m_optionMenu, DFontSizeManager::T8);
    //for test
    QAction *saveTitleAction = new QAction(m_optionMenu);
    QAction *saveToDesktopAction = new QAction(m_optionMenu);
    QAction *saveToPictureAction = new QAction(m_optionMenu);
    QAction *saveToSpecialPath = new QAction(m_optionMenu);
    //QAction *openWithDraw = new QAction(m_optionMenu);
    QAction *formatTitleAction = new QAction(m_optionMenu);
    QAction *pngAction = new QAction(m_optionMenu);
    QAction *jpgAction = new QAction(m_optionMenu);
    QAction *bmpAction = new QAction(m_optionMenu);
    m_clipTitleAction = new QAction(m_optionMenu);
    QAction *saveToClipAction = new QAction(m_optionMenu);
    m_saveCursorAction = new QAction(m_optionMenu);

    Utils::setAccessibility(saveToDesktopAction, "saveToDesktopAction");
    Utils::setAccessibility(saveToPictureAction, "saveToPictureAction");
    Utils::setAccessibility(saveToSpecialPath, "saveToSpecialPath");
    Utils::setAccessibility(saveToClipAction, "saveToClipAction");
    //Utils::setAccessibility(openWithDraw, "openWithDraw");
    Utils::setAccessibility(m_saveCursorAction, "saveCursorAction");
    Utils::setAccessibility(pngAction, "pngAction");
    Utils::setAccessibility(jpgAction, "jpgAction");
    Utils::setAccessibility(bmpAction, "bmpAction");

    saveTitleAction->setDisabled(true);
    saveTitleAction->setText(tr("Save to"));
    saveToDesktopAction->setText(tr("Desktop"));
    saveToDesktopAction->setCheckable(true);
    saveToPictureAction->setText(tr("Pictures"));
    saveToPictureAction->setCheckable(true);
    saveToSpecialPath->setText(tr("Folder"));
    saveToSpecialPath->setCheckable(true);
    saveToClipAction->setText(tr("Clipboard"));
    saveToClipAction->setCheckable(true);
    t_saveGroup->addAction(saveToDesktopAction);
    t_saveGroup->addAction(saveToPictureAction);
    t_saveGroup->addAction(saveToSpecialPath);
    t_saveGroup->addAction(saveToClipAction);

    formatTitleAction->setDisabled(true);
    formatTitleAction->setText(tr("Format"));
    pngAction->setText(tr("PNG"));
    pngAction->setCheckable(true);
    jpgAction->setText(tr("JPG"));
    jpgAction->setCheckable(true);
    bmpAction->setText(tr("BMP"));
    bmpAction->setCheckable(true);

    t_formatGroup->addAction(pngAction);
    t_formatGroup->addAction(jpgAction);
    t_formatGroup->addAction(bmpAction);

    m_clipTitleAction->setDisabled(true);
    m_clipTitleAction->setText(tr("Options"));
    //openWithDraw->setText(tr("Edit in Draw"));
    //openWithDraw->setCheckable(true);
    m_saveCursorAction->setText(tr("Show pointer"));
    m_saveCursorAction->setCheckable(true);

    //保存方式
    m_optionMenu->addAction(saveTitleAction);
    m_optionMenu->addAction(saveToClipAction);
    m_optionMenu->addAction(saveToDesktopAction);
    m_optionMenu->addAction(saveToPictureAction);
    m_optionMenu->addAction(saveToSpecialPath);
    m_optionMenu->addSeparator();

    //保存剪贴板
    m_optionMenu->addAction(m_clipTitleAction);
    m_optionMenu->addAction(m_saveCursorAction);
    // 屏蔽画板打开
    //OptionMenu->addAction(openWithDraw);
    m_optionMenu->addSeparator();

    //保存格式
    m_optionMenu->addAction(formatTitleAction);
    m_optionMenu->addAction(pngAction);
    m_optionMenu->addAction(jpgAction);
    m_optionMenu->addAction(bmpAction);

    m_optionMenu->hide();
    m_shotOptionButton->setMenu(m_optionMenu);

    SaveAction t_saveIndex = ConfigSettings::instance()->value("save", "save_op").value<SaveAction>();

    switch (t_saveIndex) {
    case SaveToDesktop: {
        saveToDesktopAction->setChecked(true);
        break;
    }
    case SaveToImage: {
        saveToPictureAction->setChecked(true);
        break;
    }
    case SaveToSpecificDir: {
        saveToSpecialPath->setChecked(true);
        break;
    }
    default:
        saveToClipAction->setChecked(true);
        break;
    }

    connect(t_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == saveToDesktopAction) {
            qDebug() << "save to desktop";
            ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToDesktop);
            ConfigSettings::instance()->setValue("common", "default_savepath", QStandardPaths::writableLocation(
                                                     QStandardPaths::DesktopLocation));
        } else if (t_act == saveToPictureAction) {
            qDebug() << "save to picture";
            ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToImage);
            ConfigSettings::instance()->setValue("common", "default_savepath", QStandardPaths::writableLocation(
                                                     QStandardPaths::PicturesLocation));
        } else if (t_act == saveToSpecialPath) {
            qDebug() << "save to path";
            ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToSpecificDir);
        } else if (t_act == saveToClipAction) {
            qDebug() << "save to clip";
            ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToClipboard);
        }
    });

    int t_pictureFormat = ConfigSettings::instance()->value("save", "format").toInt();

    switch (t_pictureFormat) {
    case 0:
        pngAction->setChecked(true);
        break;
    case 1:
        jpgAction->setChecked(true);
        break;
    case 2:
        bmpAction->setChecked(true);
        break;
    default:
        break;
    }

    connect(t_formatGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == pngAction) {
            ConfigSettings::instance()->setValue("save", "format", 0);
        } else if (t_act == jpgAction) {
            ConfigSettings::instance()->setValue("save", "format", 1);
        } else if (t_act == bmpAction) {
            ConfigSettings::instance()->setValue("save", "format", 2);
        }
    });

    int t_saveCursor = ConfigSettings::instance()->value("save", "saveCursor").toInt();

    switch (t_saveCursor) {
    case 0:
        m_saveCursorAction->setChecked(false);
        break;
    case 1:
        m_saveCursorAction->setChecked(true);
        break;
    default:
        m_saveCursorAction->setChecked(false);
        break;
    }

    connect(m_saveCursorAction, &QAction::triggered, [ = ] {
        ConfigSettings::instance()->setValue("save", "saveCursor", m_saveCursorAction->isChecked() ? 1 : 0);
    });
    /*
        int t_openWithDraw = ConfigSettings::instance()->value("open", "draw").toInt();

        switch (t_openWithDraw) {
        case 0:
            openWithDraw->setChecked(false);
            break;
        case 1:
            openWithDraw->setChecked(true);
            break;
        default:
            openWithDraw->setChecked(false);
            break;
        }

        connect(openWithDraw, &QAction::triggered, [ = ] {
            if (openWithDraw->isChecked())
            {
                ConfigSettings::instance()->setValue("open", "draw", 1);
            }

            else
            {
                ConfigSettings::instance()->setValue("open", "draw", 0);
            }
        });
    */


    QHBoxLayout *rectLayout = new QHBoxLayout();
    if (Utils::isTabletEnvironment) {
        rectLayout->setSizeConstraint(QLayout::SetMinimumSize);
    } else {
        rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    }
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(3);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(SHOT_BUTTON_SPACING);
    }


    m_shotSubTool->setLayout(rectLayout);
    addWidget(m_shotSubTool);

    connect(m_shotBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        Q_UNUSED(status);
        if (m_pinButton->isChecked()) {
            emit changeShotToolFunc("pinScreenshots");
        }
        if (m_scrollShotButton->isChecked()) {
            emit changeShotToolFunc("scrollShot");
        }
        if (m_ocrButton->isChecked()) {
            emit changeShotToolFunc("ocr");
        }
        //DPalette pa;
        if (m_rectButton->isChecked()) {
            emit changeShotToolFunc("rectangle");
        }
        if (m_circleButton->isChecked()) {
            emit changeShotToolFunc("oval");
        }

        if (m_lineflag == 0) {
            installTipHint(m_lineButton, tr("Line"));
            m_lineButton->setIcon(QIcon::fromTheme("line-normal"));
        } else if (m_lineflag == 1) {
            installTipHint(m_lineButton, tr("Arrow"));
            m_lineButton->setIcon(QIcon::fromTheme("Arrow-normal"));
        }

        if (m_lineButton->isChecked()) {
            emit changeShotToolFunc("arrow");
        }

        if (m_penButton->isChecked()) {
            emit changeShotToolFunc("line");
        }
        if (m_textButton->isChecked()) {
            emit changeShotToolFunc("text");
        }
    });
}
//快捷键或命令行启动滚动截图时，初始化滚动截图工具栏
void SubToolWidget::initScrollLabel()
{
    //分配布局
    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(7);
    rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    rectLayout->addWidget(m_ocrButton);
    rectLayout->addSpacing(SHOT_BUTTON_SPACING);
    rectLayout->addWidget(m_shotOptionButton);
    this->removeWidget(m_recordSubTool);
    this->removeWidget(m_shotSubTool);
    m_scrollShotSubTool = new DLabel(this);
    m_scrollShotSubTool->setLayout(rectLayout);
    addWidget(m_scrollShotSubTool);
}

void SubToolWidget::installTipHint(QWidget *w, const QString &hintstr)
{
    // TODO: parent must be mainframe
    auto hintWidget = new ToolTips("", m_pMainWindow);
    hintWidget->hide();
    hintWidget->setText(hintstr);
    hintWidget->setFixedHeight(32);
    installHint(w, hintWidget);
}

void SubToolWidget::installHint(QWidget *w, QWidget *hint)
{
    w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    w->installEventFilter(hintFilter);
}

void SubToolWidget::hideSomeToolBtn()
{
    //隐藏原工具栏中的不相关按钮
    m_pinButton->hide();
    m_scrollShotButton->hide();
    m_rectButton->hide();
    m_circleButton->hide();
    m_lineButton->hide();
    m_penButton->hide();
    m_textButton->hide();
    //将选择中的显示光标及选项去除
    m_optionMenu->removeAction(m_clipTitleAction);
    m_optionMenu->removeAction(m_saveCursorAction);
    //获取原水平布局，进行重新布局
    QHBoxLayout *rectLayout = static_cast<QHBoxLayout *>(m_shotSubTool->layout());
    //清楚原水平布局中的所有元素
    QLayoutItem *child;
    while ((child = rectLayout->takeAt(0)) != nullptr) {
        //setParent为NULL，防止删除之后界面不消失
        if (child->widget()) {
            child->widget()->setParent(nullptr);
        }
        delete child;
    }
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(7);
    rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    rectLayout->addWidget(m_ocrButton);
    rectLayout->addSpacing(SHOT_BUTTON_SPACING);
    rectLayout->addWidget(m_shotOptionButton);
    this->removeWidget(m_recordSubTool);

}

void SubToolWidget::setScrollShotDisabled(const bool state)
{
    m_scrollShotButton->setDisabled(state);
}

void SubToolWidget::setPinScreenshotsEnable(const bool &state)
{
    m_pinButton->setEnabled(state);
}

void SubToolWidget::switchContent(QString shapeType)
{
    if (shapeType == "record") {
        this->addWidget(m_recordSubTool);
        this->removeWidget(m_shotSubTool);
        setCurrentWidget(m_recordSubTool);
        m_currentType = shapeType;
    }

    if (shapeType == "shot") {
        this->addWidget(m_shotSubTool);
        this->removeWidget(m_recordSubTool);
        setCurrentWidget(m_shotSubTool);
        m_currentType = shapeType;
    }
}
/*
void SubToolWidget::systemAudioActionCheckedSlot(bool checked)
{
    if (AudioUtils().canVirtualCardOutput()) {
        emit systemAudioActionChecked(checked);
    } else {
        m_systemAudioAction->setChecked(false);
    }
}
*/
void SubToolWidget::changeArrowAndLineFromSideBar(int line)
{
    qDebug() << line;
    if (line == 0) {
        m_lineflag = 0;
        installTipHint(m_lineButton, tr("Line"));
        if (m_lineButton->isChecked()) {
            if (Utils::themeType == 1) {
                m_lineButton->setIcon(QIcon(":/newUI/checked/line-checked.svg"));
            } else  {
                m_lineButton->setIcon(QIcon(":/newUI/dark/checked/line-checked.svg"));
            }
        } else {
            if (Utils::themeType == 1) {
                m_lineButton->setIcon(QIcon(":/newUI/normal/line-normal.svg"));
            } else {
                m_lineButton->setIcon(QIcon(":/newUI/dark/normal/line-normal_dark.svg"));
            }
        }
    } else if (line == 1) {
        m_lineflag = 1;
        installTipHint(m_lineButton, tr("Arrow"));
        if (m_lineButton->isChecked()) {
            if (Utils::themeType == 1) {
                m_lineButton->setIcon(QIcon(":/newUI/checked/Arrow-checked.svg"));
            } else {
                m_lineButton->setIcon(QIcon(":/newUI/dark/checked/Arrow-checked.svg"));
            }
        } else {
            if (Utils::themeType == 1) {
                m_lineButton->setIcon(QIcon(":/newUI/normal/Arrow-normal.svg"));
            } else {
                m_lineButton->setIcon(QIcon(":/newUI/dark/normal/Arrow-normal_dark.svg"));
            }
        }
    }
}

void SubToolWidget::setRecordLaunchMode(const unsigned int funType)
{
    if (funType == MainWindow::record) {
        setCurrentWidget(m_recordSubTool);
    } else if (funType == MainWindow::ocr) {
        m_ocrButton->click();
    } else if (funType == MainWindow::scrollshot) {
        m_scrollShotButton->click();
    }
}

void SubToolWidget::setVideoButtonInitFromSub()
{
    if (m_cameraButton->isChecked()) {
        m_cameraButton->click();
    }

    if (m_keyBoardButton->isChecked()) {
        m_keyBoardButton->click();
    }

    if (m_mouseButton->isChecked()) {
        m_mouseButton->click();
    }

}

void SubToolWidget::shapeClickedFromWidget(QString shape)
{
    if (!shape.isEmpty()) {
        if (shape == "pinScreenshots") {
            if (!m_pinButton->isChecked())
                m_pinButton->click();
        } else if (shape == "scrollShot") {
            if (!m_scrollShotButton->isChecked())
                m_scrollShotButton->click();
        } else if (shape == "ocr") {
            if (!m_ocrButton->isChecked())
                m_ocrButton->click();
        } else if (shape == "rect") {
            if (!m_rectButton->isChecked())
                m_rectButton->click();
        } else if (shape == "circ") {
            if (!m_circleButton->isChecked())
                m_circleButton->click();
        } else if (shape == "line") {
            m_lineButton->click();
        } else if (shape == "pen") {
            m_penButton->click();
        } else if (shape == "text") {
            m_textButton->click();
        } else if (shape == "option") {
            if (m_optionMenu->isHidden() && !Utils::isTabletEnvironment) {
                m_shotOptionButton->showMenu();
//                QPoint point = QWidget::mapToGlobal(m_shotOptionButton->pos());
//                m_optionMenu->move(point.x(),point.y()+m_shotOptionButton->size().height());
//                m_optionMenu->show();
            } else {

                m_optionMenu->hide();
            }
        } else if (QString("record_option") == shape) {
            if (m_recordOptionMenu->isHidden() && !Utils::isTabletEnvironment) {
                m_optionButton->showMenu();
//                QPoint point = QWidget::mapToGlobal(m_optionButton->pos());
//                m_recordOptionMenu->move(point.x(),point.y()+m_optionButton->size().height());
//                m_recordOptionMenu->show();
            } else {
                m_recordOptionMenu->hide();
            }
        } else if (shape == "keyBoard") {
            m_keyBoardButton->click();
        } else if (shape == "mouse") {
            //m_mouseButton->click();
            if (m_cursorMenu->isHidden()) {
                m_mouseButton->showMenu();
//                QPoint point = QWidget::mapToGlobal(m_mouseButton->pos());
//                m_cursorMenu->move(point.x(),point.y() + m_mouseButton->size().height());
//                m_cursorMenu->show();
            } else {
                m_cursorMenu->hide();
            }
        } else if (shape == "camera") {
            m_cameraButton->click();
        } else if (shape == "audio") {
            if (m_audioMenu->isHidden()) {
                m_audioButton->showMenu();
//                QPoint point = QWidget::mapToGlobal(m_audioButton->pos());
//                m_audioMenu->move(point.x(),point.y()+m_audioButton->size().height());
//                m_audioMenu->show();
            } else
                m_audioMenu->hide();
        }
    }
}

void SubToolWidget::setMicroPhoneEnable(bool status)
{
    qDebug() << "mic status" << status;
    m_haveMicroPhone = status;
    m_microphoneAction->setEnabled(status);
    m_microphoneAction->setChecked(!status);
    //trigger()函数会改变当前的checked状态
    m_microphoneAction->trigger();
}

void SubToolWidget::setCameraDeviceEnable(bool status)
{
    if (status) {
        if (!m_cameraButton->isEnabled()) {
            m_cameraButton->setChecked(false);
            m_cameraButton->setEnabled(true);

            if (!m_cameraButton->isChecked()) {
                installTipHint(m_cameraButton, tr("Webcam On"));
            }
        }
    }

    else {
        if (m_cameraButton->isEnabled()) {
            m_cameraButton->setChecked(false);
            if (m_cameraButton->isChecked()) {
                installTipHint(m_cameraButton, tr("Webcam Off"));
            } else {
                installTipHint(m_cameraButton, tr("Webcam On"));
            }
            m_cameraButton->setDisabled(true);

        }
    }
}

void SubToolWidget::setSystemAudioEnable(bool status)
{
    m_haveSystemAudio = status;
    m_systemAudioAction->setEnabled(status);
    m_systemAudioAction->setCheckable(status);
    m_systemAudioAction->setChecked(!status);
    m_systemAudioAction->trigger();
}
// 当m_microphoneAction或m_systemAudioAction被点击或者程序主动调用trigg()时，会触发工具栏音频采集图标的改变及发射实际需要录制的音频
void SubToolWidget::onChangeAudioType(bool checked)
{
    Q_UNUSED(checked);
    if (m_microphoneAction->isChecked() && m_systemAudioAction->isChecked()) {
        installTipHint(m_audioButton, tr("Sound On"));
        m_audioButton->setIcon(QIcon::fromTheme("volume_normal"));
        setRecordAudioType(true, true);
    }

    if (m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked()) {
        m_audioButton->setIcon(QIcon::fromTheme("microphone_normal"));
        setRecordAudioType(true, false);
    }

    if (!m_microphoneAction->isChecked() && m_systemAudioAction->isChecked()) {
        m_audioButton->setIcon(QIcon::fromTheme("audio frequency_normal"));
        setRecordAudioType(false, true);
    }

    if (!m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked()) {
        installTipHint(m_audioButton, tr("Sound Off"));
        m_audioButton->setIcon(QIcon::fromTheme("mute_normal"));
        setRecordAudioType(false, false);
    }
}
//此方法为保持切换录屏保存类型时,不改变音频已经设置的选项
//原理：只要是gif就不会录制麦克风和系统音频，如果不是gif会保持之前的情况
void SubToolWidget::changeRecordLaunchMode()
{
    bool isEnabled = false;
    if (t_saveGif) {
        isEnabled = false;
    } else {
        isEnabled = true;
    }
    if (m_haveMicroPhone) {
        m_microphoneAction->setEnabled(isEnabled);
        if (m_microphoneAction->isChecked()) {
            m_microphoneAction->setChecked(false);
        } else {
            m_microphoneAction->setChecked(true);
        }
        m_microphoneAction->trigger();
    }
    if (m_haveSystemAudio) {
        m_systemAudioAction->setEnabled(isEnabled);
        if (m_systemAudioAction->isChecked()) {
            m_systemAudioAction->setChecked(false);
        } else {
            m_systemAudioAction->setChecked(true);
        }
        m_systemAudioAction->trigger();
    }
}

//设置录屏需要采集的声音
void SubToolWidget::setRecordAudioType(bool setMicAudio, bool setSysAudio)
{
    if (t_saveGif == true) {
        emit microphoneActionChecked(false);
        emit systemAudioActionChecked(false);
    } else {
        emit microphoneActionChecked(setMicAudio);
        emit systemAudioActionChecked(setSysAudio);
    }
}
/*
void SubToolWidget::setIsZhaoxinPlatform(bool isZhaoxin)
{
    m_isZhaoxinInSub = isZhaoxin;
}
*/
SubToolWidget::~SubToolWidget()
{
    if (nullptr != hintFilter) {
        delete hintFilter;
        hintFilter = nullptr;
    }
    if (nullptr != m_recordOptionMenu) {
        delete m_recordOptionMenu;
        m_recordOptionMenu = nullptr;
    }
    if (nullptr != m_cursorMenu) {
        delete m_cursorMenu;
        m_cursorMenu = nullptr;
    }
    if (nullptr != m_audioButton) {
        delete m_audioButton;
        m_audioButton = nullptr;
    }
    if (nullptr != m_audioMenu) {
        delete m_audioMenu;
        m_audioMenu = nullptr;
    }
    if (nullptr != m_recordBtnGroup) {
        delete m_recordBtnGroup;
        m_recordBtnGroup = nullptr;
    }
    if (nullptr != m_shotBtnGroup) {
        delete m_shotBtnGroup;
        m_shotBtnGroup = nullptr;
    }
}
