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
#include "toolbutton.h"
#include "../utils/audioutils.h"
#include <DSlider>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>
#include <QInputDialog>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 43;
const int TOOLBAR_WIDTH = 450;
const int BUTTON_SPACING = 4;
const int SHOT_BUTTON_SPACING = 4;
const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(65, 40);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(50, 40);
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

    initRecordLabel();
    initShotLabel();
    setCurrentWidget(m_recordSubTool);
    if (AudioUtils().canVirtualCardOutput()) {
    } else {
       initVirtualCard();
    }
}

void SubToolWidget::initRecordLabel()
{
    m_recordSubTool = new DLabel(this);
    QButtonGroup *rectBtnGroup = new QButtonGroup();
    rectBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;

    //添加音频按钮
    ToolButton *audioButton = new ToolButton();
    audioButton->setObjectName("AudioButton");
    audioButton->setText(tr("Audio"));
    rectBtnGroup->addButton(audioButton);
    audioButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(audioButton);

    QMenu *audioMenu = new QMenu();
    QAction *microphoneAction = new QAction(audioMenu);
    QAction *systemAudioAction = new QAction(audioMenu);
    m_systemAudioAction = systemAudioAction;
    microphoneAction->setText(tr("Microphone"));
    microphoneAction->setCheckable(true);
    microphoneAction->setChecked(true);
    systemAudioAction->setText(tr("SystemAudio"));
    systemAudioAction->setCheckable(true);
    audioMenu->addAction(microphoneAction);
    audioMenu->addSeparator();
    audioMenu->addAction(systemAudioAction);
    audioButton->setMenu(audioMenu);

    connect(microphoneAction, SIGNAL(triggered(bool)), this, SIGNAL(microphoneActionChecked(bool)));
    connect(systemAudioAction, SIGNAL(triggered(bool)), this,SLOT(systemAudioActionCheckedSlot(bool)));

    ToolButton *keyBoardButton = new ToolButton();
    keyBoardButton->setObjectName("KeyBoardButton");
    keyBoardButton->setText(tr("Key"));
    rectBtnGroup->addButton(keyBoardButton);
    keyBoardButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(keyBoardButton);

    //发送键盘按键按钮状态信号
    connect(keyBoardButton, SIGNAL(clicked(bool)),
            this, SIGNAL(keyBoardButtonClicked(bool)));

    ToolButton *cameraButton = new ToolButton();
    cameraButton->setObjectName("CameraButton");
    cameraButton->setText(tr("Camera"));
    rectBtnGroup->addButton(cameraButton);
    cameraButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(cameraButton);

    ToolButton *mouseButton = new ToolButton();
    mouseButton->setObjectName("MouseButton");
    mouseButton->setText(tr("Mouse"));
    rectBtnGroup->addButton(mouseButton);
    mouseButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(mouseButton);

    //发送鼠标按键按钮状态信号
    connect(mouseButton, SIGNAL(clicked(bool)),
            this, SIGNAL(mouseBoardButtonClicked(bool)));

    ToolButton *formatButton = new ToolButton();
    formatButton->setObjectName("FormatButton");
    formatButton->setText(tr("Format"));
    rectBtnGroup->addButton(formatButton);
    formatButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(formatButton);

    QActionGroup *t_formatActionGroup = new QActionGroup(this);
    t_formatActionGroup->setExclusive(true);
    QMenu *formatMenu = new QMenu();
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

    ToolButton *fpsButton = new ToolButton();
    fpsButton->setObjectName("FpsButton");
    fpsButton->setText(tr("Fps"));
    rectBtnGroup->addButton(fpsButton);
    fpsButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(fpsButton);

    //添加帧率选择下拉列表
    QActionGroup *t_fpsActionGroup = new QActionGroup(this);
    t_fpsActionGroup->setExclusive(true);
    QMenu *fpsMenu = new QMenu();
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
        } else if (t_act == frame10Action) {
            t_frameRateSelected = 10;
        } else if (t_act == frame20Action) {
            t_frameRateSelected = 20;
        } else if (t_act == frame24Action) {
            t_frameRateSelected = 24;
        } else if (t_act == frame30Action) {
            t_frameRateSelected = 30;
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
    rectLayout->addSpacing(16);
    rectLayout->addSpacing(16);
    rectLayout->addSpacing(BUTTON_SPACING);
    rectLayout->addStretch();
    m_recordSubTool->setLayout(rectLayout);
    addWidget(m_recordSubTool);

    connect(rectBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        QPalette pa;
        if (keyBoardButton->isChecked()) {
            pa = keyBoardButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            keyBoardButton->setPalette(pa);
        }

        else {
            pa = keyBoardButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            keyBoardButton->setPalette(pa);
        }

        if (cameraButton->isChecked()) {
            pa = cameraButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            cameraButton->setPalette(pa);
        }

        else {
            pa = cameraButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            cameraButton->setPalette(pa);
        }

        if (mouseButton->isChecked()) {
            pa = mouseButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            mouseButton->setPalette(pa);
        }

        else {
            pa = mouseButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
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

    //添加音频按钮
    ToolButton *rectButton = new ToolButton();
    rectButton->setObjectName("RectButton");
    rectButton->setText(tr("Rect"));
    rectBtnGroup->addButton(rectButton);
    rectButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(rectButton);

    ToolButton *circleButton = new ToolButton();
    circleButton->setObjectName("CircleButton");
    circleButton->setText(tr("Circ"));
    rectBtnGroup->addButton(circleButton);
    circleButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(circleButton);

    ToolButton *lineButton = new ToolButton();
    lineButton->setObjectName("LineButton");
    lineButton->setText(tr("Line"));
    rectBtnGroup->addButton(lineButton);
    lineButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(lineButton);

    ToolButton *penButton = new ToolButton();
    penButton->setObjectName("PenButton");
    penButton->setText(tr("Pen"));
    rectBtnGroup->addButton(penButton);
    penButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(penButton);

    ToolButton *textButton = new ToolButton();
    textButton->setObjectName("TextButton");
    textButton->setText(tr("Text"));
    rectBtnGroup->addButton(textButton);
    textButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(textButton);

    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(20);
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
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            rectButton->setPalette(pa);

            emit changeShotToolFunc("rect");
        }

        else {
            pa = rectButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            rectButton->setPalette(pa);
        }

        if (circleButton->isChecked()) {
            pa = circleButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            circleButton->setPalette(pa);

            emit changeShotToolFunc("circ");
        }

        else {
            pa = circleButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            circleButton->setPalette(pa);
        }

        if (lineButton->isChecked()) {
            pa = lineButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            lineButton->setPalette(pa);

            emit changeShotToolFunc("line");
        }

        else {
            pa = lineButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            lineButton->setPalette(pa);
        }

        if (penButton->isChecked()) {
            pa = penButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            penButton->setPalette(pa);

            emit changeShotToolFunc("pen");
        }

        else {
            pa = penButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            penButton->setPalette(pa);
        }

        if (textButton->isChecked()) {
            pa = textButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            textButton->setPalette(pa);

            emit changeShotToolFunc("text");
        }

        else {
            pa = textButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            textButton->setPalette(pa);
        }
    });
    rectButton->click();
}

void SubToolWidget::initVirtualCard()
{
    bool isOk;
    QString text = QInputDialog::getText(this,"Need authorization","Please enter your sudo password to be authorized",QLineEdit::PasswordEchoOnEdit,"",&isOk);
    if (isOk) {

        QProcess p(this);
        QStringList arguments;
        arguments << QString("-c");
        arguments << QString("echo %1 | sudo -S modprobe snd-aloop pcm_substreams=1").arg(text);
        qDebug()<<arguments;
        p.start("/bin/bash",arguments);
        p.waitForFinished();
        p.waitForReadyRead();
        p.close();
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
