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

#include <unistd.h>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 43;
const int TOOLBAR_WIDTH = 375;
const int BUTTON_SPACING = 1;
const int SHOT_BUTTON_SPACING = 2;
const int COLOR_NUM = 16;
const QSize TOOL_ICON_SIZE = QSize(20, 20);
const QSize MAX_TOOL_ICON_SIZE = QSize(35, 35);
const QSize MAX_TOOL_BUTTON_SIZE = QSize(82, 40);
const QSize TOOL_BUTTON_SIZE = QSize(62, 40);
const QSize MEDIUM_TOOL_BUTTON_SIZE = QSize(52, 40);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(42, 40);
}

SubToolWidget::SubToolWidget(QWidget *parent) : DStackedWidget(parent)
{
    initWidget();
}

SubToolWidget::~SubToolWidget()
{

}

void SubToolWidget::initWidget()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    m_lineflag = 0;
    initVirtualCard();
    initRecordLabel();
    initShotLabel();
    setCurrentWidget(m_recordSubTool);
}

void SubToolWidget::initRecordLabel()
{
    m_recordSubTool = new DLabel(this);
    QButtonGroup *rectBtnGroup = new QButtonGroup();
    rectBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;
    QPalette pa;


    QString audio_button_style = "QPushButton::menu-indicator{image:url(':/image/newUI/normal/Pulldown-normal.svg');"
                                 "left:-6px;"
                                 "subcontrol-origin:padding;"
                                 "subcontrol-position:right;}";
    //添加音频按钮
    ToolButton *audioButton = new ToolButton();

    pa = audioButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    audioButton->setPalette(pa);
    audioButton->setStyleSheet(audio_button_style);

    audioButton->setObjectName("AudioButton");
    audioButton->setText(tr(" "));
    audioButton->setIconSize(TOOL_ICON_SIZE);
    audioButton->setIcon(QIcon(":/image/newUI/normal/microphone_normal.svg"));
    rectBtnGroup->addButton(audioButton);
    audioButton->setFixedSize(MEDIUM_TOOL_BUTTON_SIZE);
    btnList.append(audioButton);

    DMenu *audioMenu = new DMenu();
    //for test
    audioMenu->setStyle(QStyleFactory::create("dlight"));
    //for test
    pa = audioMenu->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    audioMenu->setPalette(pa);

    DFontSizeManager::instance()->bind(audioMenu, DFontSizeManager::T8);
    QAction *microphoneAction = new QAction(audioMenu);
    QAction *systemAudioAction = new QAction(audioMenu);
    m_systemAudioAction = systemAudioAction;
    microphoneAction->setText(tr("Microphone"));
    microphoneAction->setIcon(QIcon(":/image/newUI/normal/microphone.svg"));
    microphoneAction->setCheckable(true);
    microphoneAction->setChecked(true);
    systemAudioAction->setText(tr("SystemAudio"));
    systemAudioAction->setIcon(QIcon(":/image/newUI/normal/audio frequency.svg"));

    connect(microphoneAction, &QAction::triggered, this, [ = ] {
        if (microphoneAction->isChecked() && systemAudioAction->isChecked())
        {
            audioButton->setIcon(QIcon(":/image/newUI/normal/volume_normal.svg"));
        }

        if (microphoneAction->isChecked() && !systemAudioAction->isChecked())
        {
            audioButton->setIcon(QIcon(":/image/newUI/normal/microphone_normal.svg"));
        }

        if (!microphoneAction->isChecked() && systemAudioAction->isChecked())
        {
            audioButton->setIcon(QIcon(":/image/newUI/normal/audio frequency_normal.svg"));
        }

        if (!microphoneAction->isChecked() && !systemAudioAction->isChecked())
        {
            audioButton->setIcon(QIcon(":/image/newUI/normal/mute_normal.svg"));
        }
    });

    connect(systemAudioAction, &QAction::triggered, this, [ = ] {
        if (microphoneAction->isChecked() && systemAudioAction->isChecked())
        {
            audioButton->setIcon(QIcon(":/image/newUI/normal/volume_normal.svg"));
        }

        if (microphoneAction->isChecked() && !systemAudioAction->isChecked())
        {
            audioButton->setIcon(QIcon(":/image/newUI/normal/microphone_normal.svg"));
        }

        if (!microphoneAction->isChecked() && systemAudioAction->isChecked())
        {
            audioButton->setIcon(QIcon(":/image/newUI/normal/audio frequency_normal.svg"));
        }

        if (!microphoneAction->isChecked() && !systemAudioAction->isChecked())
        {
            audioButton->setIcon(QIcon(":/image/newUI/normal/mute_normal.svg"));
        }
    });

    if (AudioUtils().canVirtualCardOutput()) {
        systemAudioAction->setCheckable(true);
    } else {
        systemAudioAction->setDisabled(true);
    }

//    systemAudioAction->setDisabled(!AudioUtils().canVirtualCardOutput());
    audioMenu->addAction(microphoneAction);
    audioMenu->addSeparator();
    audioMenu->addAction(systemAudioAction);
    audioButton->setMenu(audioMenu);

    connect(microphoneAction, SIGNAL(triggered(bool)), this, SIGNAL(microphoneActionChecked(bool)));
    connect(systemAudioAction, SIGNAL(triggered(bool)), this, SLOT(systemAudioActionCheckedSlot(bool)));

    ToolButton *keyBoardButton = new ToolButton();
    pa = keyBoardButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    keyBoardButton->setPalette(pa);

    keyBoardButton->setObjectName("KeyBoardButton");
//    keyBoardButton->setText(tr("Key"));
    keyBoardButton->setIconSize(MAX_TOOL_ICON_SIZE);
    keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key_mormal.svg"));
    rectBtnGroup->addButton(keyBoardButton);
    keyBoardButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(keyBoardButton);

    //发送键盘按键按钮状态信号
    connect(keyBoardButton, SIGNAL(clicked(bool)),
            this, SIGNAL(keyBoardButtonClicked(bool)));

    connect(keyBoardButton, &DPushButton::clicked, this, [ = ] {
        if (keyBoardButton->isChecked())
        {
            keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key display_mormal.svg"));
        }

        if (!keyBoardButton->isChecked())
        {
            keyBoardButton->setIcon(QIcon(":/image/newUI/normal/key_mormal.svg"));
        }
    });

    ToolButton *cameraButton = new ToolButton();
    cameraButton->setDisabled(CameraProcess::checkCameraAvailability());
    pa = cameraButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    cameraButton->setPalette(pa);

    cameraButton->setObjectName("CameraButton");
//    cameraButton->setText(tr("Camera"));
    cameraButton->setIconSize(MAX_TOOL_ICON_SIZE);
    cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
    rectBtnGroup->addButton(cameraButton);
    cameraButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(cameraButton);

    connect(cameraButton, &DPushButton::clicked, this, [ = ] {
        if (cameraButton->isChecked())
        {
            cameraButton->setIcon(QIcon(":/image/newUI/checked/webcam_checked.svg"));
            emit cameraActionChecked(cameraButton->isChecked());
        }

        if (!cameraButton->isChecked())
        {
            cameraButton->setIcon(QIcon(":/image/newUI/normal/webcam_normal.svg"));
        }
    });
    ToolButton *mouseButton = new ToolButton();
    pa = mouseButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    mouseButton->setPalette(pa);

    mouseButton->setObjectName("MouseButton");
    mouseButton->setIconSize(MAX_TOOL_ICON_SIZE);
    mouseButton->setIcon(QIcon(":/image/newUI/normal/mouse_mormal.svg"));
//    mouseButton->setText(tr("Mouse"));
    rectBtnGroup->addButton(mouseButton);

    mouseButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(mouseButton);

    connect(mouseButton, &DPushButton::clicked, this, [ = ] {
        if (mouseButton->isChecked())
        {
            mouseButton->setIcon(QIcon(":/image/newUI/checked/mouse_checked.svg"));
        }

        if (!mouseButton->isChecked())
        {
            mouseButton->setIcon(QIcon(":/image/newUI/normal/mouse_mormal.svg"));
        }
    });

    //发送鼠标按键按钮状态信号
    connect(mouseButton, SIGNAL(clicked(bool)),
            this, SIGNAL(mouseBoardButtonClicked(bool)));
//    QString button_style = "QPushButton{text-align: left;}";

    QString format_button_style = "QPushButton{text-align: left;}"
                                  "QPushButton::menu-indicator{image:url(':/image/newUI/normal/Pulldown-normal.svg');"
                                  "left:-4px;"
                                  "subcontrol-origin:padding;"
                                  "subcontrol-position:right;}";

    ToolButton *formatButton = new ToolButton();
    pa = formatButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    formatButton->setPalette(pa);

    formatButton->setFixedSize(MAX_TOOL_BUTTON_SIZE);
    DFontSizeManager::instance()->bind(formatButton, DFontSizeManager::T9);
    formatButton->setObjectName("FormatButton");
    formatButton->setText(tr("Webm"));
    formatButton->setIconSize(QSize(24, 24));
    formatButton->setIcon(QIcon(":/image/newUI/normal/format_normal.svg"));
    formatButton->setStyleSheet(format_button_style);
    rectBtnGroup->addButton(formatButton);

    btnList.append(formatButton);


    QActionGroup *t_formatActionGroup = new QActionGroup(this);
    t_formatActionGroup->setExclusive(true);
    DMenu *formatMenu = new DMenu();
    DFontSizeManager::instance()->bind(formatMenu, DFontSizeManager::T8);
    //for test
    formatMenu->setStyle(QStyleFactory::create("dlight"));
    //for test
    QAction *gifAction = new QAction(formatMenu);
    QAction *mp4Action = new QAction(formatMenu);
    gifAction->setText(tr("GIF"));
    gifAction->setCheckable(true);
    gifAction->setChecked(true);
    mp4Action->setText(tr("MP4"));
    mp4Action->setCheckable(true);
    formatMenu->addAction(gifAction);
    formatMenu->addSeparator();
    formatMenu->addAction(mp4Action);
    formatButton->setMenu(formatMenu);
    t_formatActionGroup->addAction(gifAction);
    t_formatActionGroup->addAction(mp4Action);

    connect(gifAction, SIGNAL(triggered(bool)), this, SIGNAL(gifActionChecked(bool)));
    connect(mp4Action, SIGNAL(triggered(bool)), this, SIGNAL(mp4ActionChecked(bool)));

    QString fps_button_style = "QPushButton{text-align: left;}"
                               "QPushButton::menu-indicator{image:url(':/image/newUI/normal/Pulldown-normal.svg');"
                               "left:-4px;"
                               "subcontrol-origin:padding;"
                               "subcontrol-position:right;}";

    ToolButton *fpsButton = new ToolButton();
    DFontSizeManager::instance()->bind(fpsButton, DFontSizeManager::T9);
    pa = fpsButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    fpsButton->setPalette(pa);

    fpsButton->setObjectName("FpsButton");
    fpsButton->setText(tr("24"));
    fpsButton->setIconSize(QSize(24, 24));
    fpsButton->setIcon(QIcon(":/image/newUI/normal/Resolving power_normal.svg"));
    rectBtnGroup->addButton(fpsButton);
    fpsButton->setFixedSize(TOOL_BUTTON_SIZE);
    fpsButton->setStyleSheet(fps_button_style);
    btnList.append(fpsButton);

    //添加帧率选择下拉列表
    QActionGroup *t_fpsActionGroup = new QActionGroup(this);
    t_fpsActionGroup->setExclusive(true);
    DMenu *fpsMenu = new DMenu();
    DFontSizeManager::instance()->bind(fpsMenu, DFontSizeManager::T8);
    //for test
    fpsMenu->setStyle(QStyleFactory::create("dlight"));
    //for test
    pa = fpsMenu->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    fpsMenu->setPalette(pa);

    QAction *frame5Action = new QAction(fpsMenu);
    QAction *frame10Action = new QAction(fpsMenu);
    QAction *frame20Action = new QAction(fpsMenu);
    QAction *frame24Action = new QAction(fpsMenu);
    QAction *frame30Action = new QAction(fpsMenu);

    frame5Action->setText(tr("5 fps"));
    frame5Action->setCheckable(true);

    frame10Action->setText(tr("10 fps"));
    frame10Action->setCheckable(true);

    frame20Action->setText(tr("20 fps"));
    frame20Action->setCheckable(true);

    frame24Action->setText(tr("24 fps"));
    frame24Action->setCheckable(true);
    frame24Action->setChecked(true);

    frame30Action->setText(tr("30 fps"));
    frame30Action->setCheckable(true);

    fpsMenu->addAction(frame5Action);
    fpsMenu->addSeparator();
    fpsMenu->addAction(frame10Action);
    fpsMenu->addSeparator();
    fpsMenu->addAction(frame20Action);
    fpsMenu->addSeparator();
    fpsMenu->addAction(frame24Action);
    fpsMenu->addSeparator();
    fpsMenu->addAction(frame30Action);

    fpsButton->setMenu(fpsMenu);
    t_fpsActionGroup->addAction(frame5Action);
    t_fpsActionGroup->addAction(frame10Action);
    t_fpsActionGroup->addAction(frame20Action);
    t_fpsActionGroup->addAction(frame24Action);
    t_fpsActionGroup->addAction(frame30Action);

    connect(t_fpsActionGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
    [ = ](QAction * t_act) {
        int t_frameRateSelected = 0;
        if (t_act == frame5Action) {
            t_frameRateSelected = 5;
            fpsButton->setText(tr("5"));
        } else if (t_act == frame10Action) {
            t_frameRateSelected = 10;
            fpsButton->setText(tr("10"));
        } else if (t_act == frame20Action) {
            t_frameRateSelected = 20;
            fpsButton->setText(tr("20"));
        } else if (t_act == frame24Action) {
            t_frameRateSelected = 24;
            fpsButton->setText(tr("24"));
        } else if (t_act == frame30Action) {
            t_frameRateSelected = 30;
            fpsButton->setText(tr("30"));
        }

        emit videoFrameRateChanged(t_frameRateSelected);
    });

    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(BUTTON_SPACING);
    }
    rectLayout->addSpacing(35);
//    rectLayout->addSpacing(16);
//    rectLayout->addSpacing(BUTTON_SPACING);
//    rectLayout->addStretch();
    m_recordSubTool->setLayout(rectLayout);
    addWidget(m_recordSubTool);

    connect(rectBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        QPalette pa;
        if (keyBoardButton->isChecked()) {
            pa = keyBoardButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            keyBoardButton->setPalette(pa);
        }

        else {
            pa = keyBoardButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            keyBoardButton->setPalette(pa);
        }

        if (cameraButton->isChecked()) {
            pa = cameraButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            cameraButton->setPalette(pa);
        }

        else {
            pa = cameraButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            cameraButton->setPalette(pa);
        }

        if (mouseButton->isChecked()) {
            pa = mouseButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            mouseButton->setPalette(pa);
        }

        else {
            pa = mouseButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            mouseButton->setPalette(pa);
        }
    });
}

void SubToolWidget::initShotLabel()
{
    m_shotSubTool = new DLabel(this);
    QButtonGroup *rectBtnGroup = new QButtonGroup();
    rectBtnGroup->setExclusive(true);
    QList<ToolButton *> btnList;
    QPalette pa;

    //添加矩形按钮
    ToolButton *rectButton = new ToolButton();

    pa = rectButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    rectButton->setPalette(pa);

    rectButton->setIconSize(QSize(30, 30));
    rectButton->setIcon(QIcon(":/image/newUI/normal/rectangle-normal.svg"));

    rectButton->setObjectName("RectButton");
//    rectButton->setText(tr("Rect"));
    rectBtnGroup->addButton(rectButton);
    rectButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(rectButton);

    ToolButton *circleButton = new ToolButton();

    pa = circleButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    circleButton->setPalette(pa);

    circleButton->setIconSize(QSize(30, 30));
    circleButton->setIcon(QIcon(":/image/newUI/normal/oval-normal.svg"));
    circleButton->setObjectName("CircleButton");

    pa = circleButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    circleButton->setPalette(pa);


//    circleButton->setText(tr("Circ"));
    rectBtnGroup->addButton(circleButton);
    circleButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(circleButton);

    m_lineButton = new ToolButton();
    pa = m_lineButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));

    m_lineButton->setIconSize(QSize(30, 30));
    m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
    m_lineButton->setPalette(pa);

    m_lineButton->setObjectName("LineButton");
//    lineButton->setText(tr("Line"));
    rectBtnGroup->addButton(m_lineButton);
    m_lineButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(m_lineButton);

    ToolButton *penButton = new ToolButton();
    pa = penButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    penButton->setIconSize(QSize(30, 30));
    penButton->setIcon(QIcon(":/image/newUI/normal/Combined Shape-normal.svg"));

    penButton->setPalette(pa);

    penButton->setObjectName("PenButton");
//    penButton->setText(tr("Pen"));
    rectBtnGroup->addButton(penButton);
    penButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(penButton);

    ToolButton *textButton = new ToolButton();
    pa = textButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    textButton->setIconSize(QSize(30, 30));
    textButton->setIcon(QIcon(":/image/newUI/normal/text.svg"));

    textButton->setPalette(pa);

    textButton->setObjectName("TextButton");
//    textButton->setText(tr("Text"));
    rectBtnGroup->addButton(textButton);
    textButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(textButton);

    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(60);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(SHOT_BUTTON_SPACING);
    }
    rectLayout->addSpacing(16);
    rectLayout->addSpacing(16);
    rectLayout->addSpacing(SHOT_BUTTON_SPACING);
    rectLayout->addStretch();
    m_shotSubTool->setLayout(rectLayout);
    addWidget(m_shotSubTool);

    connect(rectBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        QPalette pa;
        if (rectButton->isChecked()) {
            pa = rectButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            rectButton->setPalette(pa);
            rectButton->setIcon(QIcon(":/image/newUI/checked/rectangle-checked.svg"));

            emit changeShotToolFunc("rectangle");
        }

        else {
            pa = rectButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            rectButton->setPalette(pa);
            rectButton->setIcon(QIcon(":/image/newUI/normal/rectangle-normal.svg"));
        }

        if (circleButton->isChecked()) {
            pa = circleButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            circleButton->setPalette(pa);

            emit changeShotToolFunc("oval");
            circleButton->setIcon(QIcon(":/image/newUI/checked/oval-checked.svg"));
        }

        else {
            pa = circleButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            circleButton->setPalette(pa);
            circleButton->setIcon(QIcon(":/image/newUI/normal/oval-normal.svg"));
        }

        if (m_lineButton->isChecked()) {
            pa = m_lineButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            m_lineButton->setPalette(pa);
            if (m_lineflag == 0) {
                m_lineButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));
            }

            else if (m_lineflag == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
            }

            emit changeShotToolFunc("arrow");
        }

        else {
            pa = m_lineButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            m_lineButton->setPalette(pa);
            if (m_lineflag == 0) {
                m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
            }

            else if (m_lineflag == 1) {
                m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
            }
        }

        if (penButton->isChecked()) {
            pa = penButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            penButton->setPalette(pa);
            penButton->setIcon(QIcon(":/image/newUI/checked/Combined Shape-checked.svg"));

            emit changeShotToolFunc("line");
        }

        else {
            pa = penButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            penButton->setPalette(pa);
            penButton->setIcon(QIcon(":/image/newUI/normal/Combined Shape-normal.svg"));
        }

        if (textButton->isChecked()) {
            pa = textButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            textButton->setPalette(pa);
            textButton->setIcon(QIcon(":/image/newUI/checked/text_checked.svg"));

            emit changeShotToolFunc("text");
        }

        else {
            pa = textButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            textButton->setPalette(pa);
            textButton->setIcon(QIcon(":/image/newUI/normal/text.svg"));
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
                                         DLineEdit::Password, "", &isOk);
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
            m_lineButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));
        }

        else {
            m_lineButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
        }
    }

    else if (line == 1) {
        m_lineflag = 1;
        if (m_lineButton->isChecked()) {
            m_lineButton->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
        }

        else {
            m_lineButton->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
        }
    }
}
