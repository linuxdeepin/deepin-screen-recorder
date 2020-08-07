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
    const int BUTTON_SPACING = 4;
    const int SHOT_BUTTON_SPACING = 4;
    const QSize TOOL_ICON_SIZE = QSize(25, 25);
    const QSize MAX_TOOL_ICON_SIZE = QSize(40, 40);
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
    bool t_saveMkv = false;
    int t_frameRate = 0;

    Settings *t_settings = new Settings();
    QVariant t_saveGifVar = t_settings->getOption("save_as_gif");
    QVariant t_saveMkvVar = t_settings->getOption("lossless_recording");
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

    if (t_saveMkvVar.toString() == "true") {
        t_saveMkv = true;
    } else if (t_saveMkvVar.toString() == "false") {
        t_saveMkv = false;
    }


    //保持帧数的配置文件判断
    t_frameRate = t_frameRateVar.toString().toInt();
    //    //添加音频按钮
    ToolButton *audioButton = new ToolButton();

    audioButton->setObjectName("AudioButton");
    audioButton->setText(" ");
    audioButton->setIconSize(TOOL_ICON_SIZE);
    installTipHint(audioButton, tr("Sound On"));
    audioButton->setIcon(QIcon::fromTheme("microphone_normal"));

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
        if (m_microphoneAction->isChecked() && m_systemAudioAction->isChecked()){
            installTipHint(audioButton, tr("Sound On"));
            audioButton->setIcon(QIcon::fromTheme("volume_normal"));
        }

        if (m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked()){
            audioButton->setIcon(QIcon::fromTheme("microphone_normal"));
        }

        if (!m_microphoneAction->isChecked() && m_systemAudioAction->isChecked()){

            audioButton->setIcon(QIcon::fromTheme("audio frequency_normal"));
        }

        if (!m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked()){

            installTipHint(audioButton, tr("Sound Off"));
            audioButton->setIcon(QIcon::fromTheme("mute_normal"));
        }
    });

    connect(m_systemAudioAction, &QAction::triggered, this, [ = ] {
        if (m_microphoneAction->isChecked() && m_systemAudioAction->isChecked()){
            installTipHint(audioButton, tr("Sound On"));
            audioButton->setIcon(QIcon::fromTheme("volume_normal"));
        }

        if (m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked()){
            audioButton->setIcon(QIcon::fromTheme("microphone_normal"));
        }

        if (!m_microphoneAction->isChecked() && m_systemAudioAction->isChecked()){
            audioButton->setIcon(QIcon::fromTheme("audio frequency_normal"));
        }

        if (!m_microphoneAction->isChecked() && !m_systemAudioAction->isChecked()){
            installTipHint(audioButton, tr("Sound Off"));
            audioButton->setIcon(QIcon::fromTheme("mute_normal"));
        }
    });
    m_haveMicroPhone = true;
    m_microphoneAction->setCheckable(true);
    m_microphoneAction->trigger();
    m_systemAudioAction->setCheckable(true);
    m_systemAudioAction->trigger();
    m_haveSystemAudio = true;

    //    m_systemAudioAction->setDisabled(!AudioUtils().canVirtualCardOutput());
    audioMenu->addAction(m_microphoneAction);
    audioMenu->addSeparator();
    audioMenu->addAction(m_systemAudioAction);
    audioButton->setMenu(audioMenu);

    connect(m_microphoneAction, SIGNAL(triggered(bool)), this, SIGNAL(microphoneActionChecked(bool)));
    connect(m_systemAudioAction, SIGNAL(triggered(bool)), this, SIGNAL(systemAudioActionChecked(bool)));

    m_keyBoardButton = new ToolButton();

    m_keyBoardButton->setObjectName("KeyBoardButton");
    m_keyBoardButton->setIconSize(MAX_TOOL_ICON_SIZE);
    installTipHint(m_keyBoardButton, tr("Show Keystroke"));
    m_keyBoardButton->setIcon(QIcon::fromTheme("key_mormal"));


    rectBtnGroup->addButton(m_keyBoardButton);
    m_keyBoardButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_keyBoardButton);

    //发送键盘按键按钮状态信号
    connect(m_keyBoardButton, SIGNAL(clicked(bool)),
            this, SIGNAL(keyBoardButtonClicked(bool)));

    connect(m_keyBoardButton, &DPushButton::clicked, this, [ = ] {
        if (m_keyBoardButton->isChecked()){
            installTipHint(m_keyBoardButton, tr("Hide Keystroke"));
        }

        if (!m_keyBoardButton->isChecked()){
            installTipHint(m_keyBoardButton, tr("Show Keystroke"));
        }
    });

    m_cameraButton = new ToolButton();
    m_cameraButton->setDisabled((QCameraInfo::availableCameras().count() <= 0));

    m_cameraButton->setObjectName("CameraButton");
    m_cameraButton->setIconSize(MAX_TOOL_ICON_SIZE);
    installTipHint(m_cameraButton, tr("Webcam On"));

    m_cameraButton->setIcon(QIcon::fromTheme("webcam_normal"));
    rectBtnGroup->addButton(m_cameraButton);
    m_cameraButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_cameraButton);

    connect(m_cameraButton, &DPushButton::clicked, this, [ = ] {
        if (m_cameraButton->isChecked()){
            installTipHint(m_cameraButton, tr("Webcam Off"));
        }

        if (!m_cameraButton->isChecked()){
            installTipHint(m_cameraButton, tr("Webcam On"));
        }
        emit cameraActionChecked(m_cameraButton->isChecked());
    });
    m_mouseButton = new ToolButton();
    m_mouseButton->setObjectName("MouseButton");
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

    //2019-10-14：新增选项按钮
    m_optionButton = new ToolButton();
    DFontSizeManager::instance()->bind(m_optionButton, DFontSizeManager::T8);
    m_optionButton->setText(tr("Options"));
    m_optionButton->setMinimumSize(QSize(73, 40));
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
    QAction *formatTitleAction = new QAction(OptionMenu);
    QAction *gifAction = new QAction(OptionMenu);
    QAction *mp4Action = new QAction(OptionMenu);
    QAction *mkvAction = new QAction(OptionMenu);
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

    OptionMenu->addAction(formatTitleAction);
    OptionMenu->addAction(gifAction);
    OptionMenu->addAction(mp4Action);

    OptionMenu->addAction(mkvAction);
    OptionMenu->addSeparator();

    OptionMenu->addAction(fpsTitleAction);
    OptionMenu->addAction(fps5Action);
    OptionMenu->addAction(fps10Action);
    OptionMenu->addAction(fps20Action);
    OptionMenu->addAction(fps24Action);
    OptionMenu->addAction(fps30Action);

    m_optionButton->setMenu(OptionMenu);

    // change by hmy

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
        m_systemAudioAction->setEnabled(false);

    } else if (t_saveMkv == true) {
        mkvAction->setChecked(true);
        mkvAction->trigger();
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
        audioButton->setEnabled(true);
        if (m_haveMicroPhone) {
            m_microphoneAction->setEnabled(true);
            m_microphoneAction->setChecked(true);
        }

        if (m_haveSystemAudio) {
            m_systemAudioAction->setEnabled(true);
        }

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
            m_microphoneAction->setChecked(true);
        }

        if (m_haveSystemAudio) {
            m_systemAudioAction->setEnabled(true);
        }

    }


    connect(gifAction, &QAction::triggered, this, [ = ] (bool checked) {
        t_settings->setOption("lossless_recording", false);
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
        m_systemAudioAction->setEnabled(false);
        emit gifActionChecked(checked);
    });

    connect(mp4Action, &QAction::triggered, this, [ = ] (bool checked) {
        t_settings->setOption("lossless_recording", false);
        t_settings->setOption("save_as_gif", false);
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
        audioButton->setEnabled(true);
        if (m_haveMicroPhone) {
            m_microphoneAction->setEnabled(true);
            m_microphoneAction->setChecked(true);
        }
        if (m_haveSystemAudio) {
            m_systemAudioAction->setEnabled(true);
        }
        emit mp4ActionChecked(checked);
    });

    connect(mkvAction, &QAction::triggered, this, [ = ] (bool checked) {
        t_settings->setOption("lossless_recording", true);
        t_settings->setOption("save_as_gif", false);
        fps5Action->setEnabled(true);
        fps10Action->setEnabled(true);
        fps20Action->setEnabled(true);
        fps24Action->setEnabled(true);
        fps30Action->setEnabled(true);
        audioButton->setEnabled(true);
        if (m_haveMicroPhone) {
            m_microphoneAction->setEnabled(true);
            m_microphoneAction->setChecked(true);
        }
        if (m_haveSystemAudio) {
            m_systemAudioAction->setEnabled(true);
        }
        emit mkvActionChecked(checked);
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
    QButtonGroup *rectBtnGroup = new QButtonGroup();
    rectBtnGroup->setExclusive(true);
    QList<ToolButton *> btnList;
    DPalette pa;

    //添加矩形按钮
    m_rectButton = new ToolButton();
    m_rectButton->setIconSize(QSize(35, 35));

    m_rectButton->setIcon(QIcon::fromTheme("rectangle-normal"));
    m_rectButton->setObjectName("RectButton");
    rectBtnGroup->addButton(m_rectButton);
    m_rectButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    installTipHint(m_rectButton, tr("Rectangle"));
    btnList.append(m_rectButton);

    m_circleButton = new ToolButton();
    m_circleButton->setIconSize(QSize(35, 35));
    m_circleButton->setIcon(QIcon::fromTheme("oval-normal"));
    installTipHint(m_circleButton, tr("Ellipse"));
    m_circleButton->setObjectName("CircleButton");

    rectBtnGroup->addButton(m_circleButton);
    m_circleButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_circleButton);

    m_lineButton = new ToolButton();
    m_lineButton->setIconSize(QSize(35, 35));



    bool t_arrowStatus = ConfigSettings::instance()->value("arrow", "is_straight").toBool();
    if (t_arrowStatus) {
        installTipHint(m_lineButton, tr("Line"));
    } else {
        installTipHint(m_lineButton, tr("Arrow"));
    }

    if (t_arrowStatus) {
        m_lineflag = 0;
        m_lineButton->setIcon(QIcon::fromTheme("line-normal"));
    } else {
        m_lineflag = 1;
        m_lineButton->setIcon(QIcon::fromTheme("Arrow-normal"));
    }
    m_lineButton->setObjectName("LineButton");
    rectBtnGroup->addButton(m_lineButton);
    m_lineButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_lineButton);

    m_penButton = new ToolButton();
    m_penButton->setIconSize(QSize(35, 35));
    installTipHint(m_penButton, tr("Pencil"));
    m_penButton->setIcon(QIcon::fromTheme("Combined Shape-normal"));
    m_penButton->setObjectName("PenButton");
    rectBtnGroup->addButton(m_penButton);
    m_penButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_penButton);

    m_textButton = new ToolButton();
    m_textButton->setIconSize(QSize(30, 30));
    m_textButton->setIcon(QIcon::fromTheme("text"));
    //    m_textButton->setToolTip(tr("Text"));
    installTipHint(m_textButton, tr("Text"));
    m_textButton->setObjectName("TextButton");
    rectBtnGroup->addButton(m_textButton);
    m_textButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_textButton);

    //2019-10-15：添加截图选项按钮
    m_shotOptionButton = new ToolButton();
    DFontSizeManager::instance()->bind(m_shotOptionButton, DFontSizeManager::T8);
    m_shotOptionButton->setText(tr("Options"));
    m_shotOptionButton->setMinimumSize(QSize(73, 40));
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
    QAction *saveTitleAction = new QAction(OptionMenu);
    QAction *saveToDesktopAction = new QAction(OptionMenu);
    QAction *saveToPictureAction = new QAction(OptionMenu);
    QAction *saveToSpecialPath = new QAction(OptionMenu);
    QAction *openWithDraw = new QAction(OptionMenu);
    QAction *formatTitleAction = new QAction(OptionMenu);
    QAction *pngAction = new QAction(OptionMenu);
    QAction *jpgAction = new QAction(OptionMenu);
    QAction *bmpAction = new QAction(OptionMenu);
    QAction *clipTitleAction = new QAction(OptionMenu);
    QAction *saveToClipAction = new QAction(OptionMenu);
    QAction *saveCursorAction = new QAction(OptionMenu);

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

    clipTitleAction->setDisabled(true);
    clipTitleAction->setText(tr("Options"));
    openWithDraw->setText(tr("Open With Draw"));
    openWithDraw->setCheckable(true);
    saveCursorAction->setText(tr("Show cursor"));
    saveCursorAction->setCheckable(true);

    //保存方式
    OptionMenu->addAction(saveTitleAction);
    OptionMenu->addAction(saveToClipAction);
    OptionMenu->addAction(saveToDesktopAction);
    OptionMenu->addAction(saveToPictureAction);
    OptionMenu->addAction(saveToSpecialPath);
    OptionMenu->addSeparator();

    //保存剪贴板
    OptionMenu->addAction(clipTitleAction);
    OptionMenu->addAction(saveCursorAction);
    // 屏蔽画板打开
    OptionMenu->addAction(openWithDraw);
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
        else if(t_act == saveToClipAction) {
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
        saveCursorAction->setChecked(false);
        break;
    case 1:
        saveCursorAction->setChecked(true);
        break;
    default:
        saveCursorAction->setChecked(false);
        break;
    }

    connect(saveCursorAction, &QAction::triggered, [ = ] {
        if (saveCursorAction->isChecked())
        {
            ConfigSettings::instance()->setValue("save", "saveCursor", 1);
        }

        else
        {
            ConfigSettings::instance()->setValue("save", "saveCursor", 0);
        }
    });

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



    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setSizeConstraint(QLayout::SetFixedSize);
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(3);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(SHOT_BUTTON_SPACING);
    }

    m_shotSubTool->setLayout(rectLayout);
    addWidget(m_shotSubTool);

    connect(rectBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [ = ](int status) {
        Q_UNUSED(status);
        DPalette pa;
        if (m_rectButton->isChecked()) {
            emit changeShotToolFunc("rectangle");
        }
        if (m_circleButton->isChecked()) {
            emit changeShotToolFunc("oval");
        }
        if (m_lineButton->isChecked()) {
            if (m_lineflag == 0) {
                //                m_lineButton->setToolTip(tr("Line"));
                installTipHint(m_lineButton, tr("Line"));
                m_lineButton->setIcon(QIcon::fromTheme("line-normal"));
            } else if (m_lineflag == 1) {
                installTipHint(m_lineButton, tr("Arrow"));
                m_lineButton->setIcon(QIcon::fromTheme("Arrow-normal"));
            }

            emit changeShotToolFunc("arrow");
        } else {
            if (m_lineflag == 0) {
                installTipHint(m_lineButton, tr("Line"));
                m_lineButton->setIcon(QIcon::fromTheme("line-normal"));
            } else if (m_lineflag == 1) {
                installTipHint(m_lineButton, tr("Arrow"));
                m_lineButton->setIcon(QIcon::fromTheme("Arrow-normal"));
            }
        }

        if (m_penButton->isChecked()) {
            emit changeShotToolFunc("line");
        }
        if (m_textButton->isChecked()) {
            emit changeShotToolFunc("text");
        }

        //        else {
        ////            pa = m_textButton->palette();
        ////            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        ////            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
        ////            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
        ////            m_textButton->setPalette(pa);
        ////            m_textButton->setIcon(QIcon(":/image/newUI/normal/text.svg"));
        //            if (m_themeType == 1) {
        //                m_textButton->setIcon(QIcon(":/image/newUI/normal/text.svg"));
        //            }

        //            else if (m_themeType == 2) {
        //                m_textButton->setIcon(QIcon(":/image/newUI/dark/normal/text_normal_dark.svg"));
        //            }
        //        }
    });
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
            if (m_themeType == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));
            } else if (m_themeType == 2) {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/checked/line-checked.svg"));
            }
        }else {
            if (m_themeType == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
            } else if (m_themeType == 2) {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/normal/line-normal_dark.svg"));
            }
        }
    }else if (line == 1) {
        m_lineflag = 1;
        installTipHint(m_lineButton, tr("Arrow"));
        if (m_lineButton->isChecked()) {
            if (m_themeType == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
            } else if (m_themeType == 2) {
                m_lineButton->setIcon(QIcon(":/image/newUI/dark/checked/Arrow-checked.svg"));
            }
        } else {
            if (m_themeType == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
            } else if (m_themeType == 2) {
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
            if(!m_rectButton->isChecked())
                m_rectButton->click();
        } else if (shape == "circ") {
            if(!m_circleButton->isChecked())
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
                installTipHint(m_cameraButton, tr("Webcam On"));
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
                installTipHint(m_cameraButton, tr("Webcam Off"));
            }

            if (!m_cameraButton->isChecked()) {
                installTipHint(m_cameraButton, tr("Webcam On"));
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
