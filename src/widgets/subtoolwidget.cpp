/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Zheng Youge<youge.zheng@deepin.com>
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
#include <DSlider>
#include <DLineEdit>
#include <DMenu>
#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>
#include <DInputDialog>
#include <DFontSizeManager>
#include <DComboBox>
#include <DListWidget>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include "../settings.h"
#include "tooltips.h"

#include <unistd.h>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 43;
const int TOOLBAR_WIDTH = 350;
const int BUTTON_SPACING = 4;
const int SHOT_BUTTON_SPACING = 4;
const int COLOR_NUM = 16;
const QSize TOOL_ICON_SIZE = QSize(25, 25);
const QSize MAX_TOOL_ICON_SIZE = QSize(40, 40);
const QSize MAX_TOOL_BUTTON_SIZE = QSize(82, 40);
const QSize TOOL_BUTTON_SIZE = QSize(62, 40);
const QSize MEDIUM_TOOL_BUTTON_SIZE = QSize(52, 40);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(42, 40);
}

SubToolWidget::SubToolWidget(DWidget *parent) : DStackedWidget(parent)
{
    initWidget();
}

SubToolWidget::~SubToolWidget()
{

}

void SubToolWidget::initWidget()
{
    hintFilter = new HintFilter;
    m_themeType = 0;
    m_themeType = ConfigSettings::instance()->value("common", "themeType").toInt();

//    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
//    setMinimumSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
//    setFixedHeight(TOOLBAR_HEIGHT);
//    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

//    initVirtualCard();
    initRecordLabel();
    initShotLabel();
    setCurrentWidget(m_shotSubTool);
}

void SubToolWidget::initRecordLabel()
{
    m_recordSubTool = new DLabel(this);
    QButtonGroup *rectBtnGroup = new QButtonGroup();
    rectBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;
    DPalette pa;

    bool t_saveGif = false;
    int t_frameRate = 0;

    Settings *t_settings = new Settings();
    QVariant t_saveGifVar = t_settings->getOption("save_as_gif");
    QVariant t_frameRateVar = t_settings->getOption("mkv_framerate");

    //保持格式的配置文件判断
    if (t_saveGifVar.toString() == "true") {
        t_saveGif = true;
    } else if (t_saveGifVar.toString() == "false") {
        t_saveGif = false;
    } else {
        t_settings->setOption("save_as_gif", true);
        t_saveGif = true;
    }



    //保持帧数的配置文件判断
    t_frameRate = t_frameRateVar.toString().toInt();


//    QString audio_button_style = "DPushButton::menu-indicator{image:url(':/image/newUI/normal/Pulldown-normal.svg');"
//                                 "left:-6px;"
//                                 "subcontrol-origin:padding;"
//                                 "subcontrol-position:right;}";
//    //添加音频按钮
    ToolButton *audioButton = new ToolButton();

//    pa = audioButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    audioButton->setPalette(pa);
//    audioButton->setStyleSheet(audio_button_style);

    audioButton->setObjectName("AudioButton");
    audioButton->setText(tr(" "));
    audioButton->setIconSize(TOOL_ICON_SIZE);
//    audioButton->setToolTip(tr("Sound On"));
    installTipHint(audioButton, tr("Sound On"));
    if (m_themeType == 1) {
        audioButton->setIcon(QIcon(":/image/newUI/normal/microphone_normal.svg"));
    }

    else if (m_themeType == 2) {
        audioButton->setIcon(QIcon(":/image/newUI/dark/normal/microphone_normal.svg"));
    }

    rectBtnGroup->addButton(audioButton);
    audioButton->setFixedSize(MEDIUM_TOOL_BUTTON_SIZE);
    btnList.append(audioButton);

    DMenu *audioMenu = new DMenu();

    DFontSizeManager::instance()->bind(audioMenu, DFontSizeManager::T8);
    m_microphoneAction = new QAction(audioMenu);
    m_systemAudioAction = new QAction(audioMenu);
//    m_m_systemAudioAction = m_systemAudioAction;
    m_microphoneAction->setText(tr("Microphone"));
    m_microphoneAction->setCheckable(true);
    if (m_themeType == 1) {
        m_microphoneAction->setIcon(QIcon(":/image/newUI/normal/microphone.svg"));
    }

    else if (m_themeType == 2) {
        m_microphoneAction->setIcon(QIcon(":/image/newUI/dark/normal/microphone.svg"));
    }



    m_systemAudioAction->setText(tr("System Audio"));
//    m_systemAudioAction->setIcon(QIcon(":/image/newUI/normal/audio frequency.svg"));
    if (m_themeType == 1) {
        m_systemAudioAction->setIcon(QIcon(":/image/newUI/normal/audio frequency.svg"));
    }

    else if (m_themeType == 2) {
        m_systemAudioAction->setIcon(QIcon(":/image/newUI/dark/normal/audio frequency.svg"));
    }


    connect(m_microphoneAction, &QAction::triggered, this, [ = ] {
        if (m_microphoneAction->isChecked() && m_systemAudioAction->isChecked())
        {
//            audioButton->setToolTip(tr("Sound On"));
            installTipHint(audioButton, tr("Sound On"));
//            audioButton->setIcon(QIcon(":/image/newUI/normal/volume_normal.svg"));
            if (m_themeType == 1) {
                audioButton->setIcon(QIcon(":/image/newUI/normal/volume_normal.svg"));
            }

            else if (m_themeType == 2) {
                audioButton->setIcon(QIcon(":/image/newUI/dark/normal/volume_normal.svg"));
            }
        }

        if (m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked())
        {
//            audioButton->setIcon(QIcon(":/image/newUI/normal/microphone_normal.svg"));
            if (m_themeType == 1) {
                audioButton->setIcon(QIcon(":/image/newUI/normal/microphone_normal.svg"));
            }

            else if (m_themeType == 2) {
                audioButton->setIcon(QIcon(":/image/newUI/dark/normal/microphone_normal.svg"));
            }
        }

        if (!m_microphoneAction->isChecked() && m_systemAudioAction->isChecked())
        {
//            audioButton->setIcon(QIcon(":/image/newUI/normal/audio frequency_normal.svg"));
            if (m_themeType == 1) {
                audioButton->setIcon(QIcon(":/image/newUI/normal/audio frequency_normal.svg"));
            }

            else if (m_themeType == 2) {
                audioButton->setIcon(QIcon(":/image/newUI/dark/normal/audio frequency_normal.svg"));
            }
        }

        if (!m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked())
        {
//            audioButton->setToolTip(tr("Sound Off"));
            installTipHint(audioButton, tr("Sound Off"));
//            audioButton->setIcon(QIcon(":/image/newUI/normal/mute_normal.svg"));
            if (m_themeType == 1) {
                audioButton->setIcon(QIcon(":/image/newUI/normal/mute_normal.svg"));
            }

            else if (m_themeType == 2) {
                audioButton->setIcon(QIcon(":/image/newUI/dark/normal/mute_normal.svg"));
            }
        }
    });

    connect(m_systemAudioAction, &QAction::triggered, this, [ = ] {
        if (m_microphoneAction->isChecked() && m_systemAudioAction->isChecked())
        {
//            audioButton->setToolTip(tr("Sound On"));
            installTipHint(audioButton, tr("Sound On"));
//            audioButton->setIcon(QIcon(":/image/newUI/normal/volume_normal.svg"));
            if (m_themeType == 1) {
                audioButton->setIcon(QIcon(":/image/newUI/normal/volume_normal.svg"));
            }

            else if (m_themeType == 2) {
                audioButton->setIcon(QIcon(":/image/newUI/dark/normal/volume_normal.svg"));
            }
        }

        if (m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked())
        {
//            audioButton->setIcon(QIcon(":/image/newUI/normal/microphone_normal.svg"));
            if (m_themeType == 1) {
                audioButton->setIcon(QIcon(":/image/newUI/normal/microphone_normal.svg"));
            }

            else if (m_themeType == 2) {
                audioButton->setIcon(QIcon(":/image/newUI/dark/normal/microphone_normal.svg"));
            }
        }

        if (!m_microphoneAction->isChecked() && m_systemAudioAction->isChecked())
        {
//            audioButton->setIcon(QIcon(":/image/newUI/normal/audio frequency_normal.svg"));
            if (m_themeType == 1) {
                audioButton->setIcon(QIcon(":/image/newUI/normal/audio frequency_normal.svg"));
            }

            else if (m_themeType == 2) {
                audioButton->setIcon(QIcon(":/image/newUI/dark/normal/audio frequency_normal.svg"));
            }
        }

        if (!m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked())
        {
//            audioButton->setToolTip(tr("Sound Off"));
            installTipHint(audioButton, tr("Sound Off"));
//            audioButton->setIcon(QIcon(":/image/newUI/normal/mute_normal.svg"));
            if (m_themeType == 1) {
                audioButton->setIcon(QIcon(":/image/newUI/normal/mute_normal.svg"));
            }

            else if (m_themeType == 2) {
                audioButton->setIcon(QIcon(":/image/newUI/dark/normal/mute_normal.svg"));
            }
        }
    });
    m_haveMicroPhone = true;
    m_microphoneAction->setCheckable(true);
    //        m_microphoneAction->setChecked(true);
    m_microphoneAction->trigger();

//    bool t_haveMicroPhone = false;
//    m_microphoneAction->setIcon(QIcon(":/image/newUI/normal/microphone.svg"));
//    if (AudioUtils().canMicrophoneInput()) {
//        t_haveMicroPhone = true;
//        m_microphoneAction->setCheckable(true);
////        m_microphoneAction->setChecked(true);
//        m_microphoneAction->trigger();
//    } else {
//        t_haveMicroPhone = false;
//        m_microphoneAction->setDisabled(true);

////        if (m_themeType == 1) {
////            audioButton->setIcon(QIcon(":/image/newUI/normal/mute_normal.svg"));
////        }

////        else if (m_themeType == 2) {
////            audioButton->setIcon(QIcon(":/image/newUI/dark/normal/mute_normal.svg"));
////        }
//    }


    if (AudioUtils().canVirtualCardOutput()) {
        m_systemAudioAction->setCheckable(true);
        m_systemAudioAction->trigger();
        m_haveSystemAudio = true;
    } else {
        m_systemAudioAction->setDisabled(true);
        m_haveSystemAudio = false;
    }


//    m_systemAudioAction->setDisabled(!AudioUtils().canVirtualCardOutput());
    audioMenu->addAction(m_microphoneAction);
    audioMenu->addSeparator();
    audioMenu->addAction(m_systemAudioAction);
    audioButton->setMenu(audioMenu);

    connect(m_microphoneAction, SIGNAL(triggered(bool)), this, SIGNAL(microphoneActionChecked(bool)));
    connect(m_systemAudioAction, SIGNAL(triggered(bool)), this, SLOT(systemAudioActionCheckedSlot(bool)));


//    DComboBox *audioButton = new DComboBox();
//    audioButton->addItem(QIcon(":/image/newUI/normal/microphone_normal.svg"), nullptr);
//    audioButton->setStyle(QStyleFactory::create("dlight"));

//    pa = audioButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    audioButton->setPalette(pa);



    m_keyBoardButton = new ToolButton();
//    pa = m_keyBoardButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_keyBoardButton->setPalette(pa);

    m_keyBoardButton->setObjectName("KeyBoardButton");
//    keyBoardButton->setText(tr("Key"));
    m_keyBoardButton->setIconSize(MAX_TOOL_ICON_SIZE);
//    m_keyBoardButton->setToolTip(tr("Show Keystroke"));
    installTipHint(m_keyBoardButton, tr("Show Keystroke"));

    if (m_themeType == 1) {
        m_keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key_mormal.svg"));
    }

    else if (m_themeType == 2) {
        m_keyBoardButton->setIcon(QIcon(":/image/newUI/dark/normal/key_mormal.svg"));
    }

    connect(m_keyBoardButton, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_keyBoardButton->setIcon(QIcon(":/image/newUI/press/key_press.svg"));
        }

        else if (m_themeType == 2)
        {
            m_keyBoardButton->setIcon(QIcon(":/image/newUI/dark/press/key_press.svg"));
        }
    });

//    m_keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key_mormal.svg"));
    rectBtnGroup->addButton(m_keyBoardButton);
    m_keyBoardButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_keyBoardButton);

    //发送键盘按键按钮状态信号
    connect(m_keyBoardButton, SIGNAL(clicked(bool)),
            this, SIGNAL(keyBoardButtonClicked(bool)));

    connect(m_keyBoardButton, &DPushButton::clicked, this, [ = ] {
        if (m_keyBoardButton->isChecked())
        {
//            m_keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key display_mormal.svg"));
//            m_keyBoardButton->setToolTip(tr("Hide Keystroke"));
            installTipHint(m_keyBoardButton, tr("Hide Keystroke"));
            if (m_themeType == 1) {
                m_keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key display_mormal.svg"));
            }

            else if (m_themeType == 2) {
                m_keyBoardButton->setIcon(QIcon(":/image/newUI/dark/normal/key display_mormal.svg"));
            }
        }

        if (!m_keyBoardButton->isChecked())
        {
//            m_keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key_mormal.svg"));
//            m_keyBoardButton->setToolTip(tr("Show Keystroke"));
            installTipHint(m_keyBoardButton, tr("Show Keystroke"));
            if (m_themeType == 1) {
                m_keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key_mormal.svg"));
            }

            else if (m_themeType == 2) {
                m_keyBoardButton->setIcon(QIcon(":/image/newUI/dark/normal/key_mormal.svg"));
            }
        }
    });

    m_cameraButton = new ToolButton();
    m_cameraButton->setDisabled(!CameraProcess::checkCameraAvailability());
//    pa = m_cameraButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_cameraButton->setPalette(pa);

    m_cameraButton->setObjectName("CameraButton");
//    cameraButton->setText(tr("Camera"));
    m_cameraButton->setIconSize(MAX_TOOL_ICON_SIZE);
//    m_cameraButton->setToolTip(tr("Webcam On"));
    installTipHint(m_cameraButton, tr("Webcam On"));

    if (m_themeType == 1) {
        m_cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
    }

    else if (m_themeType == 2) {
        m_cameraButton->setIcon(QIcon(":/image/newUI/dark/normal/webcam_normal.svg"));
    }

    connect(m_cameraButton, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_cameraButton->setIcon(QIcon(":/image/newUI/press/webcam_press.svg"));
        }

        else if (m_themeType == 2)
        {
            m_cameraButton->setIcon(QIcon(":/image/newUI/dark/press/webcam_press.svg"));
        }
    });
//    m_cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
    rectBtnGroup->addButton(m_cameraButton);
    m_cameraButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_cameraButton);

    connect(m_cameraButton, &DPushButton::clicked, this, [ = ] {
        if (m_cameraButton->isChecked())
        {
//            m_cameraButton->setIcon(QIcon(":/image/newUI/checked/webcam_checked.svg"));
//            m_cameraButton->setToolTip(tr("Webcam Off"));
            installTipHint(m_cameraButton, tr("Webcam Off"));
            if (m_themeType == 1) {
                m_cameraButton->setIcon(QIcon(":/image/newUI/checked/webcam_checked.svg"));
            }

            else if (m_themeType == 2) {
                m_cameraButton->setIcon(QIcon(":/image/newUI/dark/checked/webcam_checked.svg"));
            }
        }

        if (!m_cameraButton->isChecked())
        {
//            m_cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
//            m_cameraButton->setToolTip(tr("Webcam On"));
            installTipHint(m_cameraButton, tr("Webcam On"));
            if (m_themeType == 1) {
                m_cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
            }

            else if (m_themeType == 2) {
                m_cameraButton->setIcon(QIcon(":/image/newUI/dark/normal/webcam_normal.svg"));
            }
        }
        emit cameraActionChecked(m_cameraButton->isChecked());
    });
    m_mouseButton = new ToolButton();
//    pa = m_mouseButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_mouseButton->setPalette(pa);

    m_mouseButton->setObjectName("MouseButton");
    m_mouseButton->setIconSize(MAX_TOOL_ICON_SIZE);
//    m_mouseButton->setToolTip(tr("Show Click"));
    installTipHint(m_mouseButton, tr("Show Click"));
    if (m_themeType == 1) {
        m_mouseButton->setIcon(QIcon(":/image/newUI/normal/mouse_mormal.svg"));
    }

    else if (m_themeType == 2) {
        m_mouseButton->setIcon(QIcon(":/image/newUI/dark/normal/mouse_mormal.svg"));
    }

    connect(m_mouseButton, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_mouseButton->setIcon(QIcon(":/image/newUI/press/mouse_press.svg"));
        }

        else if (m_themeType == 2)
        {
            m_mouseButton->setIcon(QIcon(":/image/newUI/dark/press/mouse_press.svg"));
        }
    });

//    m_mouseButton->setIcon(QIcon(":/image/newUI/normal/mouse_mormal.svg"));
//    mouseButton->setText(tr("Mouse"));
    rectBtnGroup->addButton(m_mouseButton);

    m_mouseButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_mouseButton);

    connect(m_mouseButton, &DPushButton::clicked, this, [ = ] {
        if (m_mouseButton->isChecked())
        {
//            m_mouseButton->setIcon(QIcon(":/image/newUI/checked/mouse_checked.svg"));
//            m_mouseButton->setToolTip(tr("Hide Click"));
            installTipHint(m_mouseButton, tr("Hide Click"));
            if (m_themeType == 1) {
                m_mouseButton->setIcon(QIcon(":/image/newUI/checked/mouse_checked.svg"));
            }

            else if (m_themeType == 2) {
                m_mouseButton->setIcon(QIcon(":/image/newUI/dark/checked/mouse_checked.svg"));
            }
        }

        if (!m_mouseButton->isChecked())
        {
//            m_mouseButton->setIcon(QIcon(":/image/newUI/normal/mouse_mormal.svg"));
//            m_mouseButton->setToolTip(tr("Show Click"));
            installTipHint(m_mouseButton, tr("Show Click"));
            if (m_themeType == 1) {
                m_mouseButton->setIcon(QIcon(":/image/newUI/normal/mouse_mormal.svg"));
            }

            else if (m_themeType == 2) {
                m_mouseButton->setIcon(QIcon(":/image/newUI/dark/normal/mouse_mormal.svg"));
            }
        }
    });

    //发送鼠标按键按钮状态信号
    connect(m_mouseButton, SIGNAL(clicked(bool)),
            this, SIGNAL(mouseBoardButtonClicked(bool)));
//    QString button_style = "DPushButton{text-align: left;}";

//    QString format_button_style = "QPushButton{text-align: left;}"
//                                  "QPushButton::menu-indicator{image:url(':/image/newUI/normal/Pulldown-normal.svg');"
//                                  "left:-4px;"
//                                  "subcontrol-origin:padding;"
//                                  "subcontrol-position:right;}";

//    QString format_button_style = "QPushButton{text-align: left;}";


    //2019-10-14：新增选项按钮
    m_optionButton = new ToolButton();
    DFontSizeManager::instance()->bind(m_optionButton, DFontSizeManager::T8);
    m_optionButton->setText(tr("Options"));
//    m_optionButton->setFixedSize(QSize(73, 40));
    m_optionButton->setMinimumSize(QSize(73, 40));
//    m_optionButton->setToolTip(tr("Options"));
    installTipHint(m_optionButton, tr("Options"));
    rectBtnGroup->addButton(m_optionButton);

    btnList.append(m_optionButton);

    QActionGroup *t_formatGroup = new QActionGroup(this);
    QActionGroup *t_fpsGroup = new QActionGroup(this);
    t_formatGroup->setExclusive(true);
    t_fpsGroup->setExclusive(true);

    DMenu *OptionMenu = new DMenu();
    DFontSizeManager::instance()->bind(OptionMenu, DFontSizeManager::T8);
    //for test
//    OptionMenu->setStyle(QStyleFactory::create("dlight"));
//    if (m_themeType == 1) {
//        OptionMenu->setStyle(QStyleFactory::create("dlight"));
//    }

//    else if (m_themeType == 2) {
//        OptionMenu->setStyle(QStyleFactory::create("ddark"));
//    }
    //for test
    QAction *formatTitleAction = new QAction(OptionMenu);
    QAction *gifAction = new QAction(OptionMenu);
    QAction *mp4Action = new QAction(OptionMenu);
    QAction *fpsTitleAction = new QAction(OptionMenu);
    QAction *fps5Action = new QAction(OptionMenu);
    QAction *fps10Action = new QAction(OptionMenu);
    QAction *fps20Action = new QAction(OptionMenu);
    QAction *fps24Action = new QAction(OptionMenu);
    QAction *fps30Action = new QAction(OptionMenu);

    formatTitleAction->setDisabled(true);
    formatTitleAction->setText(tr("Format:"));
    gifAction->setText(tr("GIF"));
    gifAction->setCheckable(true);
    mp4Action->setText(tr("MP4"));
    mp4Action->setCheckable(true);
    t_formatGroup->addAction(gifAction);
    t_formatGroup->addAction(mp4Action);

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

    OptionMenu->addAction(formatTitleAction);
//    OptionMenu->addSeparator();
    OptionMenu->addAction(gifAction);
//    OptionMenu->addSeparator();
    OptionMenu->addAction(mp4Action);
    OptionMenu->addSeparator();

    OptionMenu->addAction(fpsTitleAction);
//    OptionMenu->addSeparator();
    OptionMenu->addAction(fps5Action);
//    OptionMenu->addSeparator();
    OptionMenu->addAction(fps10Action);
//    OptionMenu->addSeparator();
    OptionMenu->addAction(fps20Action);
//    OptionMenu->addSeparator();
    OptionMenu->addAction(fps24Action);
//    OptionMenu->addSeparator();
    OptionMenu->addAction(fps30Action);

    m_optionButton->setMenu(OptionMenu);

    if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxinInSub == false) {
        if (t_saveGif == true) {
            gifAction->setChecked(true);
            gifAction->trigger();
            fps5Action->setEnabled(false);
            fps10Action->setEnabled(false);
            fps20Action->setEnabled(false);
            fps24Action->setEnabled(false);
            fps30Action->setEnabled(false);
            audioButton->setEnabled(false);
            if (m_microphoneAction->isChecked()) {
                m_microphoneAction->trigger();
            }
            m_microphoneAction->setEnabled(false);
//        if (m_systemAudioAction->isChecked()) {
//            m_systemAudioAction->trigger();
//        }
            m_systemAudioAction->setEnabled(false);

        } else {
            mp4Action->setChecked(true);
            mp4Action->trigger();
            fps5Action->setEnabled(true);
            fps10Action->setEnabled(true);
            fps20Action->setEnabled(true);
            fps24Action->setEnabled(true);
            fps30Action->setEnabled(true);
            audioButton->setEnabled(true);
            if (m_haveMicroPhone) {
                m_microphoneAction->setEnabled(true);
            }

            if (m_haveSystemAudio) {
                m_systemAudioAction->setEnabled(true);
            }

        }
    }

//    else {
//        t_saveGif = true;
//        gifAction->setChecked(true);
//        mp4Action->setChecked(false);
//        mp4Action->setEnabled(false);
//        gifAction->trigger();
//        fps5Action->setChecked(false);
//        fps10Action->setChecked(false);
//        fps20Action->setChecked(false);
//        fps24Action->setChecked(false);
//        fps30Action->setChecked(false);
//        fps5Action->setEnabled(false);
//        fps10Action->setEnabled(false);
//        fps20Action->setEnabled(false);
//        fps24Action->setEnabled(false);
//        fps30Action->setEnabled(false);
//        audioButton->setEnabled(false);
//        if (m_microphoneAction->isChecked()) {
//            m_microphoneAction->trigger();
//        }
//        m_microphoneAction->setEnabled(false);
////        if (m_systemAudioAction->isChecked()) {
////            m_systemAudioAction->trigger();
////        }
//        m_systemAudioAction->setEnabled(false);
//    }

    connect(gifAction, &QAction::triggered, this, [ = ] (bool checked) {
        t_settings->setOption("save_as_gif", true);
        fps5Action->setEnabled(false);
        fps10Action->setEnabled(false);
        fps20Action->setEnabled(false);
        fps24Action->setEnabled(false);
        fps30Action->setEnabled(false);
        audioButton->setEnabled(false);
        if (m_microphoneAction->isChecked()) {
            m_microphoneAction->trigger();
        }
        m_microphoneAction->setEnabled(false);
//        if (m_systemAudioAction->isChecked()) {
//            m_systemAudioAction->trigger();
//        }
        m_systemAudioAction->setEnabled(false);
        emit gifActionChecked(checked);
    });

    connect(mp4Action, &QAction::triggered, this, [ = ] (bool checked) {
        t_settings->setOption("save_as_gif", false);
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
        audioButton->setEnabled(true);
        if (m_haveMicroPhone) {
            m_microphoneAction->setEnabled(true);
        }
        if (m_haveSystemAudio) {
            m_systemAudioAction->setEnabled(true);
        }
        emit mp4ActionChecked(checked);
    });

    connect(t_fpsGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        int t_frameRateSelected = 0;
        if (t_act == fps5Action) {
            t_frameRateSelected = 5;
            t_settings->setOption("mkv_framerate", "5");
        } else if (t_act == fps10Action) {
            t_frameRateSelected = 10;
            t_settings->setOption("mkv_framerate", "10");
        } else if (t_act == fps20Action) {
            t_frameRateSelected = 20;
            t_settings->setOption("mkv_framerate", "20");
        } else if (t_act == fps24Action) {
            t_frameRateSelected = 24;
            t_settings->setOption("mkv_framerate", "24");
        } else if (t_act == fps30Action) {
            t_frameRateSelected = 30;
            t_settings->setOption("mkv_framerate", "30");
        }

        emit videoFrameRateChanged(t_frameRateSelected);
    });

    if (QSysInfo::currentCpuArchitecture().startsWith("x86") && m_isZhaoxinInSub == false) {
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
//    rectLayout->addSpacing(35);
//    rectLayout->addSpacing(SHOT_BUTTON_SPACING);
//    rectLayout->addStretch();
//    rectLayout->addSpacing(16);
//    rectLayout->addSpacing(BUTTON_SPACING);
//    rectLayout->addStretch();
    m_recordSubTool->setLayout(rectLayout);
    addWidget(m_recordSubTool);

//    connect(rectBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
//    [ = ](int status) {
//        DPalette pa;
//        if (m_keyBoardButton->isChecked()) {
//            pa = m_keyBoardButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_keyBoardButton->setPalette(pa);
//        }

//        else {
//            pa = m_keyBoardButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_keyBoardButton->setPalette(pa);
//        }

//        if (m_cameraButton->isChecked()) {
//            pa = m_cameraButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_cameraButton->setPalette(pa);
//        }

//        else {
//            pa = m_cameraButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_cameraButton->setPalette(pa);
//        }

//        if (m_mouseButton->isChecked()) {
//            pa = m_mouseButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_mouseButton->setPalette(pa);
//        }

//        else {
//            pa = m_mouseButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_mouseButton->setPalette(pa);
//        }
//    });
}

void SubToolWidget::initShotLabel()
{
    m_shotSubTool = new DLabel(this);
    QButtonGroup *rectBtnGroup = new QButtonGroup();
    rectBtnGroup->setExclusive(true);
    QList<ToolButton *> btnList;
    DPalette pa;

    //添加矩形按钮
    m_rectButton = new ToolButton();

//    pa = m_rectButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_rectButton->setPalette(pa);

    m_rectButton->setIconSize(QSize(35, 35));


    if (m_themeType == 1) {
        m_rectButton->setIcon(QIcon(":/image/newUI/normal/rectangle-normal.svg"));
    }

    else if (m_themeType == 2) {
        m_rectButton->setIcon(QIcon(":/image/newUI/dark/normal/rectangle-normal_dark.svg"));
    }

    connect(m_rectButton, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_rectButton->setIcon(QIcon(":/image/newUI/press/rectangle-press.svg"));
        }

        else if (m_themeType == 2)
        {
            m_rectButton->setIcon(QIcon(":/image/newUI/dark/press/rectangle-press.svg"));
        }
    });


//    m_rectButton->setIcon(QIcon(":/image/newUI/normal/rectangle-normal.svg"));

    m_rectButton->setObjectName("RectButton");
//    rectButton->setText(tr("Rect"));
    rectBtnGroup->addButton(m_rectButton);
    m_rectButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
//    m_rectButton->setToolTip(tr("Rectangle"));
    installTipHint(m_rectButton, tr("Rectangle"));
    btnList.append(m_rectButton);

    m_circleButton = new ToolButton();

//    pa = m_circleButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_circleButton->setPalette(pa);

    m_circleButton->setIconSize(QSize(35, 35));
//    m_circleButton->setToolTip(tr("Ellipse"));
    installTipHint(m_circleButton, tr("Ellipse"));
    if (m_themeType == 1) {
        m_circleButton->setIcon(QIcon(":/image/newUI/normal/oval-normal.svg"));
    }

    else if (m_themeType == 2) {
        m_circleButton->setIcon(QIcon(":/image/newUI/dark/normal/oval-normal_dark.svg"));
    }

    connect(m_circleButton, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_circleButton->setIcon(QIcon(":/image/newUI/press/oval-press.svg"));
        }

        else if (m_themeType == 2)
        {
            m_circleButton->setIcon(QIcon(":/image/newUI/dark/press/oval-press.svg"));
        }
    });

//    m_circleButton->setIcon(QIcon(":/image/newUI/normal/oval-normal.svg"));
    m_circleButton->setObjectName("CircleButton");

//    pa = m_circleButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_circleButton->setPalette(pa);


//    circleButton->setText(tr("Circ"));
    rectBtnGroup->addButton(m_circleButton);
    m_circleButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_circleButton);

    m_lineButton = new ToolButton();
//    pa = m_lineButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));

    m_lineButton->setIconSize(QSize(35, 35));



    bool t_arrowStatus = ConfigSettings::instance()->value("arrow", "is_straight").toBool();
    if (t_arrowStatus) {
//        m_lineButton->setToolTip(tr("Line"));
        installTipHint(m_lineButton, tr("Line"));
    }

    else {
//        m_lineButton->setToolTip(tr("Arrow"));
        installTipHint(m_lineButton, tr("Arrow"));
    }

    connect(m_lineButton, &ToolButton::onPress, this, [ = ] {
        bool t_arrowStatustemp = ConfigSettings::instance()->value("arrow", "is_straight").toBool();
        if (m_themeType == 1)
        {

            if (t_arrowStatustemp) {
                m_lineButton->setIcon(QIcon(":/image/newUI/press/line-press.svg"));
            }

            else {
                m_lineButton->setIcon(QIcon(":/image/newUI/press/Arrow-press.svg"));
            }
        }

        else if (m_themeType == 2)
        {

            if (t_arrowStatustemp) {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/press/line-press.svg"));
            }

            else {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/press/Arrow-press.svg"));
            }
        }
    });


    if (t_arrowStatus) {
        m_lineflag = 0;
//        m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
        if (m_themeType == 1) {
            m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
        }

        else if (m_themeType == 2) {
            m_lineButton->setIcon(QIcon(":/image/newUI/dark/normal/line-normal_dark.svg"));
        }
    }

    else {
        m_lineflag = 1;
//        m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
        if (m_themeType == 1) {
            m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
        }

        else if (m_themeType == 2) {
            m_lineButton->setIcon(QIcon(":/image/newUI/dark/normal/Arrow-normal_dark.svg"));
        }
    }


//    m_lineButton->setPalette(pa);

    m_lineButton->setObjectName("LineButton");
//    lineButton->setText(tr("Line"));
    rectBtnGroup->addButton(m_lineButton);
    m_lineButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_lineButton);

    m_penButton = new ToolButton();
//    pa = m_penButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
    m_penButton->setIconSize(QSize(35, 35));
//    m_penButton->setToolTip(tr("Pencil"));
    installTipHint(m_penButton, tr("Pencil"));
//    m_penButton->setIcon(QIcon(":/image/newUI/normal/Combined Shape-normal.svg"));

    if (m_themeType == 1) {
        m_penButton->setIcon(QIcon(":/image/newUI/normal/Combined Shape-normal.svg"));
    }

    else if (m_themeType == 2) {
        m_penButton->setIcon(QIcon(":/image/newUI/dark/normal/Combined Shape-normal_dark.svg"));
    }

    connect(m_penButton, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_penButton->setIcon(QIcon(":/image/newUI/press/Combined Shape-press.svg"));

        }

        else if (m_themeType == 2)
        {

            m_penButton->setIcon(QIcon(":/image/newUI/dark/press/Combined Shape-press.svg"));

        }
    });

//    m_penButton->setPalette(pa);

    m_penButton->setObjectName("PenButton");
//    penButton->setText(tr("Pen"));
    rectBtnGroup->addButton(m_penButton);
    m_penButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_penButton);

    m_textButton = new ToolButton();
//    pa = m_textButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
    m_textButton->setIconSize(QSize(30, 30));
//    m_textButton->setToolTip(tr("Text"));
    installTipHint(m_textButton, tr("Text"));
//    m_textButton->setIcon(QIcon(":/image/newUI/normal/text.svg"));
    if (m_themeType == 1) {
        m_textButton->setIcon(QIcon(":/image/newUI/normal/text.svg"));
    }

    else if (m_themeType == 2) {
        m_textButton->setIcon(QIcon(":/image/newUI/dark/normal/text_normal_dark.svg"));
    }

    connect(m_textButton, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_textButton->setIcon(QIcon(":/image/newUI/press/text_press.svg"));

        }

        else if (m_themeType == 2)
        {

            m_textButton->setIcon(QIcon(":/image/newUI/dark/press/text_press.svg"));

        }
    });

//    m_textButton->setPalette(pa);

    m_textButton->setObjectName("TextButton");
//    textButton->setText(tr("Text"));
    rectBtnGroup->addButton(m_textButton);
    m_textButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_textButton);

    //2019-10-15：添加截图选项按钮
    m_shotOptionButton = new ToolButton();
    DFontSizeManager::instance()->bind(m_shotOptionButton, DFontSizeManager::T8);
    m_shotOptionButton->setText(tr("Options"));
//    m_shotOptionButton->setFixedSize(QSize(73, 40));
    m_shotOptionButton->setMinimumSize(QSize(73, 40));
//    m_shotOptionButton->setToolTip(tr("Options"));
    installTipHint(m_shotOptionButton, tr("Options"));
    rectBtnGroup->addButton(m_shotOptionButton);

    btnList.append(m_shotOptionButton);

    QActionGroup *t_saveGroup = new QActionGroup(this);
    QActionGroup *t_formatGroup = new QActionGroup(this);
    t_saveGroup->setExclusive(true);
    t_formatGroup->setExclusive(true);

    DMenu *OptionMenu = new DMenu();

    DFontSizeManager::instance()->bind(OptionMenu, DFontSizeManager::T8);
    //for test
//    OptionMenu->setStyle(QStyleFactory::create("dlight"));
//    if (m_themeType == 1) {
//        OptionMenu->setStyle(QStyleFactory::create("dlight"));
//    }

//    else if (m_themeType == 2) {
//        OptionMenu->setStyle(QStyleFactory::create("ddark"));
//    }
    //for test
    QAction *saveTitleAction = new QAction(OptionMenu);
    QAction *saveToDesktopAction = new QAction(OptionMenu);
    QAction *saveToPictureAction = new QAction(OptionMenu);
    QAction *saveToSpecialPath = new QAction(OptionMenu);
    QAction *formatTitleAction = new QAction(OptionMenu);
    QAction *pngAction = new QAction(OptionMenu);
    QAction *jpgAction = new QAction(OptionMenu);
    QAction *bmpAction = new QAction(OptionMenu);
    QAction *clipTitleAction = new QAction(OptionMenu);
    QAction *clipAction = new QAction(OptionMenu);

    saveTitleAction->setDisabled(true);
    saveTitleAction->setText(tr("Save to"));
    saveToDesktopAction->setText(tr("Desktop"));
    saveToDesktopAction->setCheckable(true);
    saveToPictureAction->setText(tr("Pictures"));
    saveToPictureAction->setCheckable(true);
    saveToSpecialPath->setText(tr("Folder"));
    saveToSpecialPath->setCheckable(true);
    t_saveGroup->addAction(saveToDesktopAction);
    t_saveGroup->addAction(saveToPictureAction);
    t_saveGroup->addAction(saveToSpecialPath);

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

    clipTitleAction->setDisabled(true);
    clipTitleAction->setText(tr("Options"));
    clipAction->setText(tr("Copy to clipboard"));
    clipAction->setCheckable(true);

    //保存方式
    OptionMenu->addAction(saveTitleAction);
    OptionMenu->addAction(saveToDesktopAction);
    OptionMenu->addAction(saveToPictureAction);
    OptionMenu->addAction(saveToSpecialPath);
    OptionMenu->addSeparator();

    //保存剪贴板
    OptionMenu->addAction(clipTitleAction);
    OptionMenu->addAction(clipAction);
    OptionMenu->addSeparator();

    //保存格式
    OptionMenu->addAction(formatTitleAction);
    OptionMenu->addAction(pngAction);
    OptionMenu->addAction(jpgAction);
    OptionMenu->addAction(bmpAction);


    m_shotOptionButton->setMenu(OptionMenu);

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
        saveToDesktopAction->setChecked(true);
        break;
    }

    connect(t_saveGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        if (t_act == saveToDesktopAction) {
            qDebug() << "save to desktop";
            ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToDesktop);
            ConfigSettings::instance()->setValue("common", "default_savepath", QStandardPaths::writableLocation(
                                                     QStandardPaths::DesktopLocation));
        }

        else if (t_act == saveToPictureAction) {
            qDebug() << "save to picture";
            ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToImage);
            ConfigSettings::instance()->setValue("common", "default_savepath", QStandardPaths::writableLocation(
                                                     QStandardPaths::PicturesLocation));
        }

        else if (t_act == saveToSpecialPath) {
            qDebug() << "save to path";
            ConfigSettings::instance()->setValue("save", "save_op", SaveAction::SaveToSpecificDir);
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

    int t_saveToClipBoard = ConfigSettings::instance()->value("save", "saveClip").toInt();

    switch (t_saveToClipBoard) {
    case 0:
        clipAction->setChecked(false);
        break;
    case 1:
        clipAction->setChecked(true);
        break;
    default:
        clipAction->setChecked(true);
        break;
    }

    connect(clipAction, &QAction::triggered, [ = ] {
        if (clipAction->isChecked())
        {
            ConfigSettings::instance()->setValue("save", "saveClip", 1);
        }

        else
        {
            ConfigSettings::instance()->setValue("save", "saveClip", 0);
        }
    });


    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(3);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(SHOT_BUTTON_SPACING);
    }
//    rectLayout->addSpacing(16);
//    rectLayout->addSpacing(16);
//    rectLayout->addSpacing(SHOT_BUTTON_SPACING);
//    rectLayout->addStretch();
    m_shotSubTool->setLayout(rectLayout);
    addWidget(m_shotSubTool);

    connect(rectBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        DPalette pa;
        if (m_rectButton->isChecked()) {
//            pa = m_rectButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_rectButton->setPalette(pa);
//            m_rectButton->setIcon(QIcon(":/image/newUI/checked/rectangle-checked.svg"));

            if (m_themeType == 1) {
                m_rectButton->setIcon(QIcon(":/image/newUI/checked/rectangle-checked.svg"));
            }

            else if (m_themeType == 2) {
                m_rectButton->setIcon(QIcon(":/image/newUI/dark/checked/rectangle-checked.svg"));
            }

            emit changeShotToolFunc("rectangle");
        }

        else {
//            pa = m_rectButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_rectButton->setPalette(pa);
//            m_rectButton->setIcon(QIcon(":/image/newUI/normal/rectangle-normal.svg"));
            if (m_themeType == 1) {
                m_rectButton->setIcon(QIcon(":/image/newUI/normal/rectangle-normal.svg"));
            }

            else if (m_themeType == 2) {
                m_rectButton->setIcon(QIcon(":/image/newUI/dark/normal/rectangle-normal_dark.svg"));
            }
        }

        if (m_circleButton->isChecked()) {
//            pa = m_circleButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_circleButton->setPalette(pa);

            emit changeShotToolFunc("oval");
//            m_circleButton->setIcon(QIcon(":/image/newUI/checked/oval-checked.svg"));
            if (m_themeType == 1) {
                m_circleButton->setIcon(QIcon(":/image/newUI/checked/oval-checked.svg"));
            }

            else if (m_themeType == 2) {
                m_circleButton->setIcon(QIcon(":/image/newUI/dark/checked/oval-checked.svg"));
            }
        }

        else {
//            pa = m_circleButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_circleButton->setPalette(pa);
//            m_circleButton->setIcon(QIcon(":/image/newUI/normal/oval-normal.svg"));
            if (m_themeType == 1) {
                m_circleButton->setIcon(QIcon(":/image/newUI/normal/oval-normal.svg"));
            }

            else if (m_themeType == 2) {
                m_circleButton->setIcon(QIcon(":/image/newUI/dark/normal/oval-normal_dark.svg"));
            }
        }

        if (m_lineButton->isChecked()) {
//            pa = m_lineButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_lineButton->setPalette(pa);
            if (m_lineflag == 0) {
//                m_lineButton->setToolTip(tr("Line"));
                installTipHint(m_lineButton, tr("Line"));
//                m_lineButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));

                if (m_themeType == 1) {
                    m_lineButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));
                }

                else if (m_themeType == 2) {
                    m_lineButton->setIcon(QIcon(":/image/newUI/dark/checked/line-checked.svg"));
                }
            }

            else if (m_lineflag == 1) {
//                m_lineButton->setToolTip(tr("Arrow"));
                installTipHint(m_lineButton, tr("Arrow"));
//                m_lineButton->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
                if (m_themeType == 1) {
                    m_lineButton->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
                }

                else if (m_themeType == 2) {
                    m_lineButton->setIcon(QIcon(":/image/newUI/dark/checked/Arrow-checked.svg"));
                }
            }

            emit changeShotToolFunc("arrow");
        }

        else {
//            pa = m_lineButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_lineButton->setPalette(pa);
            if (m_lineflag == 0) {
//                m_lineButton->setToolTip(tr("Line"));
                installTipHint(m_lineButton, tr("Line"));
//                m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
                if (m_themeType == 1) {
                    m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
                }

                else if (m_themeType == 2) {
                    m_lineButton->setIcon(QIcon(":/image/newUI/dark/normal/line-normal_dark.svg"));
                }
            }

            else if (m_lineflag == 1) {
//                m_lineButton->setToolTip(tr("Arrow"));
                installTipHint(m_lineButton, tr("Arrow"));
//                m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
                if (m_themeType == 1) {
                    m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
                }

                else if (m_themeType == 2) {
                    m_lineButton->setIcon(QIcon(":/image/newUI/dark/normal/Arrow-normal_dark.svg"));
                }
            }
        }

        if (m_penButton->isChecked()) {
//            pa = m_penButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_penButton->setPalette(pa);
//            m_penButton->setIcon(QIcon(":/image/newUI/checked/Combined Shape-checked.svg"));
            if (m_themeType == 1) {
                m_penButton->setIcon(QIcon(":/image/newUI/checked/Combined Shape-checked.svg"));
            }

            else if (m_themeType == 2) {
                m_penButton->setIcon(QIcon(":/image/newUI/dark/checked/Combined Shape-checked.svg"));
            }

            emit changeShotToolFunc("line");
        }

        else {
//            pa = m_penButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_penButton->setPalette(pa);
//            m_penButton->setIcon(QIcon(":/image/newUI/normal/Combined Shape-normal.svg"));
            if (m_themeType == 1) {
                m_penButton->setIcon(QIcon(":/image/newUI/normal/Combined Shape-normal.svg"));
            }

            else if (m_themeType == 2) {
                m_penButton->setIcon(QIcon(":/image/newUI/dark/normal/Combined Shape-normal_dark.svg"));
            }
        }

        if (m_textButton->isChecked()) {
//            pa = m_textButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_textButton->setPalette(pa);
//            m_textButton->setIcon(QIcon(":/image/newUI/checked/text_checked.svg"));
            if (m_themeType == 1) {
                m_textButton->setIcon(QIcon(":/image/newUI/checked/text_checked.svg"));
            }

            else if (m_themeType == 2) {
                m_textButton->setIcon(QIcon(":/image/newUI/dark/checked/text_checked.svg"));
            }

            emit changeShotToolFunc("text");
        }

        else {
//            pa = m_textButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_textButton->setPalette(pa);
//            m_textButton->setIcon(QIcon(":/image/newUI/normal/text.svg"));
            if (m_themeType == 1) {
                m_textButton->setIcon(QIcon(":/image/newUI/normal/text.svg"));
            }

            else if (m_themeType == 2) {
                m_textButton->setIcon(QIcon(":/image/newUI/dark/normal/text_normal_dark.svg"));
            }
        }
    });
//    rectButton->click();
}

void SubToolWidget::initVirtualCard()
{
    if (AudioUtils().canVirtualCardOutput()) {
        return;
    }
    bool isOk;
    QString text = DInputDialog::getText(this, tr("Need authorization"), tr("Please enter your sudo password to be authorized"),
                                         QLineEdit::Password, "", &isOk);
    if (isOk) {

        QProcess p(this);
        QStringList arguments;
        arguments << QString("-c");
        arguments << QString("echo %1 | sudo -S modprobe snd-aloop pcm_substreams=1 ; sudo sed -i '$ a snd_aloop' /etc/modules").arg(text);
        qDebug() << arguments;
        p.start("/bin/bash", arguments);
        p.waitForFinished();
        p.waitForReadyRead();
        p.close();

        sleep(1);
    }
}

void SubToolWidget::installTipHint(QWidget *w, const QString &hintstr)
{
    // TODO: parent must be mainframe
    auto hintWidget = new ToolTips("", this->parentWidget()->parentWidget()->parentWidget());
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

void SubToolWidget::switchContent(QString shapeType)
{
    if (shapeType == "record") {
        setCurrentWidget(m_recordSubTool);
        m_currentType = shapeType;
    }

    if (shapeType == "shot") {
        setCurrentWidget(m_shotSubTool);
        m_currentType = shapeType;
    }
}

void SubToolWidget::systemAudioActionCheckedSlot(bool checked)
{
    if (AudioUtils().canVirtualCardOutput()) {
        emit systemAudioActionChecked(checked);
    } else {
        m_systemAudioAction->setChecked(false);
    }
}

void SubToolWidget::changeArrowAndLineFromSideBar(int line)
{
    if (line == 0) {
        m_lineflag = 0;
        if (m_lineButton->isChecked()) {
//            m_lineButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));
            if (m_themeType == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));
            }

            else if (m_themeType == 2) {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/checked/line-checked.svg"));
            }
        }

        else {
//            m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
            if (m_themeType == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
            }

            else if (m_themeType == 2) {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/normal/line-normal_dark.svg"));
            }
        }
    }

    else if (line == 1) {
        m_lineflag = 1;
        if (m_lineButton->isChecked()) {
//            m_lineButton->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
            if (m_themeType == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
            }

            else if (m_themeType == 2) {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/checked/Arrow-checked.svg"));
            }
        }

        else {
//            m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
            if (m_themeType == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
            }

            else if (m_themeType == 2) {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/normal/Arrow-normal_dark.svg"));
            }
        }
    }
}

void SubToolWidget::setRecordLaunchMode(bool recordLaunch)
{
    if (recordLaunch == true) {
        setCurrentWidget(m_recordSubTool);
    }
    //    qDebug() << "sub record mode";
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
        if (shape == "rect") {
            m_rectButton->click();
        } else if (shape == "circ") {
            m_circleButton->click();
        } else if (shape == "line") {
            m_lineButton->click();
        } else if (shape == "pen") {
            m_penButton->click();
        } else if (shape == "text") {
            m_textButton->click();
        } else if (shape == "option") {
            m_shotOptionButton->click();
        }
    }
}

void SubToolWidget::setMicroPhoneEnable(bool status)
{
    qDebug() << "mic status" << status;
    if (status) {
        m_haveMicroPhone = true;

        if (m_microphoneAction->isEnabled()) {
            return;
        } else {
            m_microphoneAction->setEnabled(true);
            if (m_microphoneAction->isChecked()) {
                return;
            } else {
                m_microphoneAction->trigger();
            }
        }
    } else {
        m_haveMicroPhone = false;
        if (!m_microphoneAction->isEnabled()) {
            return;
        } else {
            if (!m_microphoneAction->isChecked()) {
                m_microphoneAction->setEnabled(false);
                return;
            } else {
                m_microphoneAction->trigger();
                m_microphoneAction->setEnabled(false);
            }
        }
    }
    update();
}

void SubToolWidget::setCameraDeviceEnable(bool status)
{
    if (status) {
        if (!m_cameraButton->isEnabled()) {
            m_cameraButton->setChecked(false);
            m_cameraButton->setEnabled(true);

            if (!m_cameraButton->isChecked()) {
//                m_cameraButton->setToolTip(tr("Webcam On"));
                installTipHint(m_cameraButton, tr("Webcam On"));
                if (m_themeType == 1) {
                    m_cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
                }

                else if (m_themeType == 2) {
                    m_cameraButton->setIcon(QIcon(":/image/newUI/dark/normal/webcam_normal.svg"));
                }
            }
        }

        else {
            return;
        }
    }

    else {
        if (m_cameraButton->isEnabled()) {
            m_cameraButton->setChecked(false);
            if (m_cameraButton->isChecked()) {
//                m_cameraButton->setToolTip(tr("Webcam Off"));
                installTipHint(m_cameraButton, tr("Webcam Off"));
                if (m_themeType == 1) {
                    m_cameraButton->setIcon(QIcon(":/image/newUI/checked/webcam_checked.svg"));
                }

                else if (m_themeType == 2) {
                    m_cameraButton->setIcon(QIcon(":/image/newUI/dark/checked/webcam_checked.svg"));
                }
            }

            if (!m_cameraButton->isChecked()) {
                //            m_cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
//                m_cameraButton->setToolTip(tr("Webcam On"));
                installTipHint(m_cameraButton, tr("Webcam On"));
                if (m_themeType == 1) {
                    m_cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
                }

                else if (m_themeType == 2) {
                    m_cameraButton->setIcon(QIcon(":/image/newUI/dark/normal/webcam_normal.svg"));
                }
            }
            m_cameraButton->setDisabled(true);

        } else {
            return;
        }
    }
}

void SubToolWidget::setSystemAudioEnable(bool status)
{
    if (status) {
        m_systemAudioAction->setEnabled(true);
        m_systemAudioAction->setCheckable(true);
        m_systemAudioAction->trigger();
        m_haveSystemAudio = true;
    }

}

void SubToolWidget::setIsZhaoxinPlatform(bool isZhaoxin)
{
    m_isZhaoxinInSub = isZhaoxin;
}
