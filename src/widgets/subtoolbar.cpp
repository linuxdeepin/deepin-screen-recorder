// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subtoolbar.h"
#include "toolbutton.h"
#include "colorbutton.h"
#include "textbutton.h"
#include "../utils/baseutils.h"
#include "../utils/configsettings.h"
#include "../utils/saveutils.h"

#include <DSlider>
#include <DLineEdit>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QSlider>
#include <QStyleFactory>
#include <QDebug>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 28;
const int TOOLBAR_WIDTH = 280;
const int BUTTON_SPACING = 1;
const int COLOR_NUM = 16;
const QSize SAVEBTN_SIZE = QSize(33, 26);
}

SubToolBar::SubToolBar(DWidget *parent)
    : DStackedWidget(parent),
      m_lineWidth(1)
{
    initWidget();
}

void SubToolBar::initWidget()
{
//    setObjectName("SubToolBar");
//    setStyleSheet(getFileContent(":/resources/qss/subtoolbar.qss"));
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);

    initRectLabel();
    initArrowLabel();
    initLineLabel();
    initTextLabel();
    initColorLabel();
    initSaveLabel();

    setCurrentWidget(nullptr);
}

void SubToolBar::initRectLabel()
{
    m_rectLabel = new DLabel(this);
    QButtonGroup *rectBtnGroup = new QButtonGroup();
    rectBtnGroup->setExclusive(true);
    QList<ToolButton *> btnList;
    //rectangle, oval...
    ToolButton *fineLine = new ToolButton();
    fineLine->setObjectName("FineLine");
    rectBtnGroup->addButton(fineLine);
    btnList.append(fineLine);
    ToolButton  *mediumLine = new ToolButton();
    mediumLine->setObjectName("MediumLine");
    rectBtnGroup->addButton(mediumLine);
    btnList.append(mediumLine);
    ToolButton *thickLine = new ToolButton();
    thickLine->setObjectName("ThickLine");
    rectBtnGroup->addButton(thickLine);
    btnList.append(thickLine);
    //separator line...
    DLabel *vSeparatorLine = new DLabel();
    vSeparatorLine->setFixedSize(1, 16);
    vSeparatorLine->setObjectName("VerticalSeparatorLine");
    //blur, mosaic...
    ToolButton *blurBtn = new ToolButton();
    blurBtn->setObjectName("BlurBtn");
    ToolButton *mosaicBtn = new ToolButton();
    mosaicBtn->setObjectName("MosaicBtn");
    connect(blurBtn, &ToolButton::clicked, this, [ = ] {
        ConfigSettings::instance()->setValue("effect", "is_blur", blurBtn->isChecked());
        if (blurBtn->isChecked())
        {
            mosaicBtn->setChecked(false);
            ConfigSettings::instance()->setValue("effect", "is_mosaic", false);
        }
    });
    connect(mosaicBtn, &ToolButton::clicked, this, [ = ] {
        ConfigSettings::instance()->setValue("effect", "is_mosaic", mosaicBtn->isChecked());
        if (mosaicBtn->isChecked())
        {
            blurBtn->setChecked(false);
            ConfigSettings::instance()->setValue("effect", "is_blur", false);
        }
    });
    int lineWidthIndex = ConfigSettings::instance()->value("rectangle",
                                                           "linewidth_index").toInt();
    btnList[lineWidthIndex]->setChecked(true);

    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(1);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(BUTTON_SPACING);
        connect(btnList[i], &ToolButton::clicked, this, [ = ] {
            ConfigSettings::instance()->setValue(m_currentType, "linewidth_index", i);
        });
    }
    rectLayout->addSpacing(16);
    rectLayout->addWidget(vSeparatorLine);
    rectLayout->addSpacing(16);
    rectLayout->addWidget(blurBtn);
    rectLayout->addSpacing(BUTTON_SPACING);
    rectLayout->addWidget(mosaicBtn);
    rectLayout->addStretch();
    m_rectLabel->setLayout(rectLayout);
    addWidget(m_rectLabel);

    connect(this, &SubToolBar::shapeChanged, this, [ = ] {
        int lineIndex = ConfigSettings::instance()->value(m_currentType,
                                                          "linewidth_index").toInt();
        btnList[lineIndex]->setChecked(true);
    });
}

void SubToolBar::initArrowLabel()
{
    m_arrowLabel = new DLabel(this);
    //arrow
    QButtonGroup  *arrowBtnGroup = new QButtonGroup();
    arrowBtnGroup->setExclusive(true);
    QList<ToolButton *> arrowBtnList;
    ToolButton *arrowFineLine = new ToolButton();
    arrowFineLine->setObjectName("ArrowFine");
    arrowBtnGroup->addButton(arrowFineLine);
    arrowBtnList.append(arrowFineLine);
    ToolButton  *arrowMediumLine = new ToolButton();
    arrowMediumLine->setObjectName("ArrowMedium");
    arrowBtnGroup->addButton(arrowMediumLine);
    arrowBtnList.append(arrowMediumLine);
    ToolButton *arrowThickLine = new ToolButton();
    arrowThickLine->setObjectName("ArrowThick");
    arrowBtnGroup->addButton(arrowThickLine);
    arrowBtnList.append(arrowThickLine);
    //line
    QButtonGroup  *lineBtnGroup = new QButtonGroup();
    lineBtnGroup->setExclusive(true);
    //rectangle, oval...
    ToolButton *fineLine = new ToolButton();
    fineLine->setObjectName("ArrowFineLine");
    lineBtnGroup->addButton(fineLine);
    arrowBtnList.append(fineLine);
    ToolButton  *mediumLine = new ToolButton();
    mediumLine->setObjectName("ArrowMediumLine");
    lineBtnGroup->addButton(mediumLine);
    arrowBtnList.append(mediumLine);
    ToolButton *thickLine = new ToolButton();
    thickLine->setObjectName("ArrowThickLine");
    lineBtnGroup->addButton(thickLine);
    arrowBtnList.append(thickLine);
    //separator line...
    DLabel *vSeparatorLine = new DLabel();
    vSeparatorLine->setFixedSize(1, 16);
    vSeparatorLine->setObjectName("VerticalSeparatorLine");
    QButtonGroup *styleBtnGroup = new QButtonGroup;
    styleBtnGroup->setExclusive(true);
    ToolButton  *lineBtn = new ToolButton();
    lineBtn->setObjectName("LineBtn");
    styleBtnGroup->addButton(lineBtn, 0);
    ToolButton *arrowBtn = new ToolButton();
    arrowBtn->setObjectName("ArrowBtn");
    arrowBtn->setChecked(true);
    styleBtnGroup->addButton(arrowBtn, 1);
    if (ConfigSettings::instance()->value("arrow", "is_straight").toBool()) {
        lineBtn->setChecked(true);
    } else {
        arrowBtn->setChecked(true);
    }

    int arrowWidthIndex = ConfigSettings::instance()->value("arrow", "arrow_linewidth_index").toInt();
    int sLineWidthIndex = ConfigSettings::instance()->value("arrow", "straightline_linewidth").toInt();
    arrowBtnList[arrowWidthIndex]->setChecked(true);
    arrowBtnList[sLineWidthIndex + 3]->setChecked(true);
    QHBoxLayout *arrowLayout = new QHBoxLayout();
    arrowLayout->setMargin(0);
    arrowLayout->setSpacing(BUTTON_SPACING);
    arrowLayout->addSpacing(1);
    for (int j = 0; j < arrowBtnList.length(); j++) {
        arrowLayout->addWidget(arrowBtnList[j]);
//        arrowLayout->addSpacing(BUTTON_SPACING);
        connect(arrowBtnList[j], &ToolButton::clicked, this, [ = ] {
            if (j < 3)
            {
                ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", j);
            } else
            {
                ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", j - 3);
            }
        });
    }
    arrowLayout->addSpacing(16);
    arrowLayout->addWidget(vSeparatorLine);
    arrowLayout->addSpacing(16);
    arrowLayout->addWidget(lineBtn);
    arrowLayout->addWidget(arrowBtn);
    arrowLayout->addStretch();
    m_arrowLabel->setLayout(arrowLayout);
    addWidget(m_arrowLabel);

    fineLine->hide();
    mediumLine->hide();
    thickLine->hide();

    connect(arrowBtn, &ToolButton::toggled, this, [ = ](bool checked) {
        if (checked) {
            arrowFineLine->show();
            arrowMediumLine->show();
            arrowThickLine->show();
            fineLine->hide();
            mediumLine->hide();
            thickLine->hide();
        } else {
            arrowFineLine->hide();
            arrowMediumLine->hide();
            arrowThickLine->hide();
            fineLine->show();
            mediumLine->show();
            thickLine->show();
        }

        ConfigSettings::instance()->setValue("arrow", "is_straight", !checked);
    });

    connect(this, &SubToolBar::shapeChanged, this, [ = ] {
        if (ConfigSettings::instance()->value("arrow", "is_straight").toBool())
        {
            arrowFineLine->hide();
            arrowMediumLine->hide();
            arrowThickLine->hide();
            fineLine->show();
            mediumLine->show();
            thickLine->show();
        } else
        {
            arrowFineLine->show();
            arrowMediumLine->show();
            arrowThickLine->show();
            fineLine->hide();
            mediumLine->hide();
            thickLine->hide();
        }

        int  arrowLineWidth = ConfigSettings::instance()->value("arrow", "arrow_linewidth_index").toInt();
        int  sLineWidth = ConfigSettings::instance()->value("arrow", "straightline_linewidth_index").toInt();
        arrowBtnList[arrowLineWidth]->setChecked(true);
        arrowBtnList[sLineWidth + 3]->setChecked(true);
    });
}

void SubToolBar::initLineLabel()
{
    m_lineLabel = new DLabel(this);
    //rectangle, oval...
    QButtonGroup *lineBtnGroup = new QButtonGroup();
    lineBtnGroup->setExclusive(true);
    QList<ToolButton *> btnList;

    ToolButton *fineLine = new ToolButton();
    fineLine->setObjectName("FineLine");
    lineBtnGroup->addButton(fineLine);
    btnList.append(fineLine);
    ToolButton  *mediumLine = new ToolButton();
    mediumLine->setObjectName("MediumLine");
    lineBtnGroup->addButton(mediumLine);
    btnList.append(mediumLine);
    ToolButton *thickLine = new ToolButton();
    thickLine->setObjectName("ThickLine");
    lineBtnGroup->addButton(thickLine);
    btnList.append(thickLine);

    int lineWidthIndex = ConfigSettings::instance()->value("line",
                                                           "linewidth_index").toInt();
    btnList[lineWidthIndex]->setChecked(true);

    QHBoxLayout *lineLayout = new QHBoxLayout();
    lineLayout->setMargin(0);
    lineLayout->setSpacing(0);
    lineLayout->addSpacing(1);
    for (int k = 0; k < btnList.length(); k++) {
        lineLayout->addWidget(btnList[k]);
        lineLayout->addSpacing(BUTTON_SPACING);
        connect(btnList[k], &ToolButton::clicked, this, [ = ] {
            ConfigSettings::instance()->setValue("line", "linewidth_index", k);
        });
    }

    lineLayout->addStretch();
    m_lineLabel->setLayout(lineLayout);
    addWidget(m_lineLabel);

    connect(this, &SubToolBar::shapeChanged, this, [ = ] {
        int lineIndex = ConfigSettings::instance()->value(m_currentType,
                                                          "linewidth_index").toInt();
        btnList[lineIndex]->setChecked(true);
    });
}

void SubToolBar::initTextLabel()
{
    //text...
    m_textLabel = new DLabel(this);
//    m_textLabel->setStyleSheet(getFileContent(":/resources/qss/textbutton.qss"));
    QList<int> fontSizeList;
    fontSizeList << 9 << 10 << 12 << 14 << 18 << 24 << 36 << 48 << 64 << 72 << 96;
    QButtonGroup *textBtnGroup = new QButtonGroup(this);

    QList<TextButton *> textButtonList;
    for (int i = 0; i < fontSizeList.length(); i++) {
        TextButton *textButton = new TextButton(fontSizeList[i], m_textLabel);
        textBtnGroup->addButton(textButton);
        textButtonList.append(textButton);
    }
    textBtnGroup->setExclusive(true);
    int defaultFontSize = ConfigSettings::instance()->value("text", "fontsize").toInt();

    QHBoxLayout *textLayout = new QHBoxLayout();
    textLayout->setMargin(0);
    textLayout->setSpacing(0);
    textLayout->addSpacing(1);
    for (int k = 0; k < textButtonList.length(); k++) {
        textLayout->addWidget(textButtonList[k]);
        textLayout->addSpacing(BUTTON_SPACING);
        if (fontSizeList[k] == defaultFontSize) {
            textButtonList[k]->setChecked(true);
        }
    }
    textLayout->addStretch();
    m_textLabel->setLayout(textLayout);
    addWidget(m_textLabel);
}

void SubToolBar::initColorLabel()
{
    m_colorLabel = new DLabel(this);
//    m_colorLabel->setStyleSheet(getFileContent(":/resources/qss/colorbtn.qss"));
    QList<ColorButton *> colorBtnList;
    QButtonGroup *colorBtnGroup = new QButtonGroup(m_colorLabel);
    colorBtnGroup->setExclusive(true);

    for (int i = 0; i < COLOR_NUM; i ++) {
        ColorButton *colorBtn = new ColorButton(colorIndexOf(i));
        colorBtn->setObjectName(QString("ColorBtn%1").arg(i + 1));
        colorBtnList.append(colorBtn);
        colorBtnGroup->addButton(colorBtn);
    }

    QHBoxLayout *colorLayout = new QHBoxLayout();
    colorLayout->setMargin(0);
    colorLayout->setSpacing(0);
    colorLayout->addSpacing(3);
    for (int i = 0; i < colorBtnList.length(); i++) {
        colorLayout->addWidget(colorBtnList[i]);
        colorLayout->addSpacing(1);
        connect(colorBtnList[i], &ColorButton::updatePaintColor,
                this, &SubToolBar::updateColor);
    }
    colorLayout->addStretch();
    m_colorLabel->setLayout(colorLayout);

    addWidget(m_colorLabel);

    connect(this, &SubToolBar::defaultColorIndexChanged, this, [ = ](int index) {
        colorBtnList[index]->setChecked(true);
    });
}

void SubToolBar::updateColor(QColor color)
{
//    qDebug() << "update text color";
    int colNum = colorIndex(color);
    ConfigSettings::instance()->setValue("common", "color_index", colNum);
    emit currentColorChanged(color);
}

void SubToolBar::initSaveLabel()
{
    //save to...
    std::map<SaveAction, ToolButton *> toolBtnMap;
    ToolButton *saveDesktopBtn = new ToolButton();
    saveDesktopBtn->setFixedSize(SAVEBTN_SIZE);
    saveDesktopBtn->setObjectName("SaveToDesktop");
//    saveDesktopBtn->setTips(tr("Save to desktop"));
    toolBtnMap[SaveAction::SaveToDesktop] = saveDesktopBtn;

    ToolButton *savePicBtn = new ToolButton();
    savePicBtn->setFixedSize(SAVEBTN_SIZE);
    savePicBtn->setObjectName("SaveToPictureDir");
//    savePicBtn->setTips(tr("Autosave"));
    toolBtnMap[SaveAction::AutoSave] = savePicBtn;

    ToolButton *saveSpecificDirBtn = new ToolButton();
    saveSpecificDirBtn->setFixedSize(SAVEBTN_SIZE);
    saveSpecificDirBtn->setObjectName("SaveToSpecificDir");
//    saveSpecificDirBtn->setTips(tr("Save to specified folder"));
    toolBtnMap[SaveAction::SaveToSpecificDir] = saveSpecificDirBtn;

    ToolButton *saveClipboardBtn = new ToolButton();
    saveClipboardBtn->setFixedSize(SAVEBTN_SIZE);
    saveClipboardBtn->setObjectName("SaveToClipboard");
//    saveClipboardBtn->setTips(tr("Copy to clipboard"));
    toolBtnMap[SaveAction::SaveToClipboard] = saveClipboardBtn;

    ToolButton *saveAutoClipboardBtn = new ToolButton();
    saveAutoClipboardBtn->setObjectName("SaveToAutoClipboard");
//    saveAutoClipboardBtn->setTips(tr("Autosave and copy to clipboard"));
    toolBtnMap[SaveAction::SaveToAutoClipboard] = saveAutoClipboardBtn;

    DLabel *lowQualityText = new DLabel();
    lowQualityText->setObjectName("LowQualityLabel");
//    lowQualityText->setText(tr("Low"));
    QSlider *saveQualitySlider = new QSlider(Qt::Horizontal);
    saveQualitySlider->setFixedWidth(58);
    saveQualitySlider->setMinimum(50);
    saveQualitySlider->setMaximum(100);
    saveQualitySlider->setPageStep(1);
    saveQualitySlider->setSliderPosition(100);
    setSaveQualityIndex(saveQualitySlider->value());

    connect(saveQualitySlider, &QSlider::valueChanged,
            this, &SubToolBar::setSaveQualityIndex);

    DLabel *highQualityText = new DLabel();
    highQualityText->setObjectName("HighQualityLabel");
//    highQualityText->setText(tr("High"));

    m_saveLabel = new DLabel(this);
    QHBoxLayout *saveLayout = new QHBoxLayout();
    saveLayout->setMargin(0);
    saveLayout->setSpacing(0);
    saveLayout->addSpacing(1);

    SaveAction saveAction = ConfigSettings::instance()->value("save", "save_op").value<SaveAction>();

    for (auto it = toolBtnMap.cbegin(); it != toolBtnMap.cend(); ++it) {
        SaveAction  action = it->first;
        ToolButton *button = it->second;

        saveLayout->addWidget(button);

        connect(button, &ToolButton::clicked, this, [ = ] {
            setSaveOption(action);
        });

        connect(button, &ToolButton::onEnter, this, [ = ] {
            emit showSaveTip(button->getTips());
        });

        connect(button, &ToolButton::onExist, this, &SubToolBar::hideSaveTip);

        if (saveAction == action) {
            button->setChecked(true);
        }
    }

    connect(this, &SubToolBar::saveBtnPressed, this, [ = ](int index) -> void {
        for (auto it = toolBtnMap.cbegin(); it != toolBtnMap.cend(); ++it)
        {
            if (static_cast<SaveAction>(index) == it->first) {
                return it->second->click();
            }
        }
    });

    saveLayout->addStretch();
    saveLayout->addWidget(lowQualityText);
    saveLayout->addWidget(saveQualitySlider);
    saveLayout->addWidget(highQualityText);
    saveLayout->addSpacing(12);

    m_saveLabel->setLayout(saveLayout);
    addWidget(m_saveLabel);
}

void SubToolBar::switchContent(QString shapeType)
{
    if (shapeType == "rectangle" || shapeType == "oval") {
        setCurrentWidget(m_rectLabel);
        m_currentType = shapeType;
        emit shapeChanged();
    }   else if (shapeType == "arrow") {
        setCurrentWidget(m_arrowLabel);
        m_currentType = shapeType;
        emit shapeChanged();
    } else if (shapeType == "line") {
        setCurrentWidget(m_lineLabel);
        m_currentType = shapeType;
        emit shapeChanged();
    } else if (shapeType == "text") {
        setCurrentWidget(m_textLabel);
        m_currentType = shapeType;
        emit shapeChanged();
    } else if (shapeType == "color") {
        int defaultColorIndex = ConfigSettings::instance()->value("common",
                                                                  "color_index").toInt();
        emit defaultColorIndexChanged(defaultColorIndex);
        setCurrentWidget(m_colorLabel);
    } else if (shapeType == "saveList") {
        setCurrentWidget(m_saveLabel);
    }
    qDebug() << "subToolBar shape:" << shapeType;
}

void SubToolBar::setSaveOption(SaveAction action)
{
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
        qDebug() << "Shift key holded: temporary action, will not remember the save_op.";
        ConfigSettings::instance()->setTemporarySaveAction(std::pair<bool, SaveAction>(true, action));
    } else {
        ConfigSettings::instance()->setValue("save", "save_op", action);
    }

    emit saveAction();
}

void SubToolBar::setSaveQualityIndex(int saveQuality)
{
    ConfigSettings::instance()->setValue("save", "save_quality", saveQuality);
}
/*
 * never used
int SubToolBar::getSaveQualityIndex()
{
    return m_saveQuality;
}
*/
SubToolBar::~SubToolBar() {}
