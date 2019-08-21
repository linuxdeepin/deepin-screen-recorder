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

#include <DSlider>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>

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
    microphoneAction->setText("microphone");
    microphoneAction->setCheckable(true);
    microphoneAction->setChecked(true);
    systemAudioAction->setText("systemAudio");
    systemAudioAction->setCheckable(true);
    audioMenu->addAction(microphoneAction);
    audioMenu->addAction(systemAudioAction);
    audioButton->setMenu(audioMenu);
    connect(microphoneAction, SIGNAL(triggered(bool)), this, SIGNAL(microphoneActionChecked(bool)));
    connect(systemAudioAction, SIGNAL(triggered(bool)), this, SIGNAL(systemAudioActionChecked(bool)));

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
    mouseButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(mouseButton);

    ToolButton *formatButton = new ToolButton();
    formatButton->setObjectName("FormatButton");
    formatButton->setText(tr("Format"));
    rectBtnGroup->addButton(formatButton);
    formatButton->setFixedSize(TOOL_BUTTON_SIZE);
    btnList.append(formatButton);

    ToolButton *fpsButton = new ToolButton();
    fpsButton->setObjectName("FpsButton");
    fpsButton->setText(tr("Fps"));
    rectBtnGroup->addButton(fpsButton);
    fpsButton->setFixedSize(MIN_TOOL_BUTTON_SIZE);
    btnList.append(fpsButton);

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
    textButton->setText(tr("text"));
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
        }

        else {
            pa = textButton->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            textButton->setPalette(pa);
        }
    });
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
