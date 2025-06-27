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
#include "../utils/log.h"

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
    qCDebug(dsrApp) << "SubToolBar constructor called.";
    initWidget();
}

void SubToolBar::initWidget()
{
    qCDebug(dsrApp) << "SubToolBar::initWidget called.";
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
    qCDebug(dsrApp) << "SubToolBar::initRectLabel called.";
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
        qCDebug(dsrApp) << "Blur button clicked.";
        ConfigSettings::instance()->setValue("effect", "is_blur", blurBtn->isChecked());
        if (blurBtn->isChecked())
        {
            qCDebug(dsrApp) << "Blur button checked, unchecking mosaic.";
            mosaicBtn->setChecked(false);
            ConfigSettings::instance()->setValue("effect", "is_mosaic", false);
        }
    });
    connect(mosaicBtn, &ToolButton::clicked, this, [ = ] {
        qCDebug(dsrApp) << "Mosaic button clicked.";
        ConfigSettings::instance()->setValue("effect", "is_mosaic", mosaicBtn->isChecked());
        if (mosaicBtn->isChecked())
        {
            qCDebug(dsrApp) << "Mosaic button checked, unchecking blur.";
            blurBtn->setChecked(false);
            ConfigSettings::instance()->setValue("effect", "is_blur", false);
        }
    });
    int lineWidthIndex = ConfigSettings::instance()->value("rectangle",
                                                           "linewidth_index").toInt();
    btnList[lineWidthIndex]->setChecked(true);
    qCDebug(dsrApp) << "Initial line width index for rectangle:" << lineWidthIndex;

    QHBoxLayout *rectLayout = new QHBoxLayout();
    rectLayout->setContentsMargins(0, 0, 0, 0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(1);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(BUTTON_SPACING);
        connect(btnList[i], &ToolButton::clicked, this, [ = ] {
            qCDebug(dsrApp) << "Line thickness button clicked, index:" << i;
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
        qCDebug(dsrApp) << "Shape changed signal received in initRectLabel.";
        int lineIndex = ConfigSettings::instance()->value(m_currentType,
                                                          "linewidth_index").toInt();
        btnList[lineIndex]->setChecked(true);
        qCDebug(dsrApp) << "Setting line width based on shape change to index:" << lineIndex;
    });
}

void SubToolBar::initArrowLabel()
{
    qCDebug(dsrApp) << "SubToolBar::initArrowLabel called.";
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
        qCDebug(dsrApp) << "Initial state: straight line button checked.";
        lineBtn->setChecked(true);
    } else {
        qCDebug(dsrApp) << "Initial state: arrow button checked.";
        arrowBtn->setChecked(true);
    }

    int arrowWidthIndex = ConfigSettings::instance()->value("arrow", "arrow_linewidth_index").toInt();
    int sLineWidthIndex = ConfigSettings::instance()->value("arrow", "straightline_linewidth").toInt();
    qCDebug(dsrApp) << "Initial arrow width index:" << arrowWidthIndex << ", straight line width index:" << sLineWidthIndex;
    arrowBtnList[arrowWidthIndex]->setChecked(true);
    arrowBtnList[sLineWidthIndex + 3]->setChecked(true);
    QHBoxLayout *arrowLayout = new QHBoxLayout();
    arrowLayout->setContentsMargins(0, 0, 0, 0);
    arrowLayout->setSpacing(BUTTON_SPACING);
    arrowLayout->addSpacing(1);
    for (int j = 0; j < arrowBtnList.length(); j++) {
        arrowLayout->addWidget(arrowBtnList[j]);
//        arrowLayout->addSpacing(BUTTON_SPACING);
        connect(arrowBtnList[j], &ToolButton::clicked, this, [ = ] {
            qCDebug(dsrApp) << "Arrow/line thickness button clicked, index:" << j;
            if (j < 3)
            {
                qCDebug(dsrApp) << "Setting arrow line width index:" << j;
                ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", j);
            } else
            {
                qCDebug(dsrApp) << "Setting straight line width index:" << j - 3;
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
        qCDebug(dsrApp) << "Arrow button toggled, checked:" << checked;
        if (checked) {
            qCDebug(dsrApp) << "Arrow button checked, showing arrow line options.";
            arrowFineLine->show();
            arrowMediumLine->show();
            arrowThickLine->show();
            fineLine->hide();
            mediumLine->hide();
            thickLine->hide();
        } else {
            qCDebug(dsrApp) << "Arrow button unchecked, showing straight line options.";
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
        qCDebug(dsrApp) << "Shape changed signal received in initArrowLabel.";
        if (ConfigSettings::instance()->value("arrow", "is_straight").toBool())
        {
            qCDebug(dsrApp) << "Shape changed to straight line, showing straight line options.";
            arrowFineLine->hide();
            arrowMediumLine->hide();
            arrowThickLine->hide();
            fineLine->show();
            mediumLine->show();
            thickLine->show();
        } else
        {
            qCDebug(dsrApp) << "Shape changed to arrow, showing arrow line options.";
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
    qCDebug(dsrApp) << "SubToolBar::initLineLabel called.";
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
    qCDebug(dsrApp) << "Initial line width index for line:" << lineWidthIndex;

    QHBoxLayout *lineLayout = new QHBoxLayout();
    lineLayout->setContentsMargins(0, 0, 0, 0);
    lineLayout->setSpacing(0);
    lineLayout->addSpacing(1);
    for (int k = 0; k < btnList.length(); k++) {
        lineLayout->addWidget(btnList[k]);
        lineLayout->addSpacing(BUTTON_SPACING);
        connect(btnList[k], &ToolButton::clicked, this, [ = ] {
            qCDebug(dsrApp) << "Line thickness button clicked for line, index:" << k;
            ConfigSettings::instance()->setValue("line", "linewidth_index", k);
        });
    }

    lineLayout->addStretch();
    m_lineLabel->setLayout(lineLayout);
    addWidget(m_lineLabel);

    connect(this, &SubToolBar::shapeChanged, this, [ = ] {
        qCDebug(dsrApp) << "Shape changed signal received in initLineLabel.";
        int lineIndex = ConfigSettings::instance()->value(m_currentType,
                                                          "linewidth_index").toInt();
        btnList[lineIndex]->setChecked(true);
        qCDebug(dsrApp) << "Setting line width based on shape change for line to index:" << lineIndex;
    });
}

void SubToolBar::initTextLabel()
{
    qCDebug(dsrApp) << "SubToolBar::initTextLabel called.";
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
    qCDebug(dsrApp) << "Default font size:" << defaultFontSize;

    QHBoxLayout *textLayout = new QHBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(0);
    textLayout->addSpacing(1);
    for (int k = 0; k < textButtonList.length(); k++) {
        textLayout->addWidget(textButtonList[k]);
        textLayout->addSpacing(BUTTON_SPACING);
        if (fontSizeList[k] == defaultFontSize) {
            qCDebug(dsrApp) << "Setting text button checked for font size:" << fontSizeList[k];
            textButtonList[k]->setChecked(true);
        }
    }
    textLayout->addStretch();
    m_textLabel->setLayout(textLayout);
    addWidget(m_textLabel);
}

void SubToolBar::initColorLabel()
{
    qCDebug(dsrApp) << "SubToolBar::initColorLabel called.";
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
        qCDebug(dsrApp) << "Adding color button for index:" << i;
    }

    QHBoxLayout *colorLayout = new QHBoxLayout();
    colorLayout->setContentsMargins(0, 0, 0, 0);
    colorLayout->setSpacing(0);
    colorLayout->addSpacing(3);
    for (int i = 0; i < colorBtnList.length(); i++) {
        colorLayout->addWidget(colorBtnList[i]);
        colorLayout->addSpacing(1);
        connect(colorBtnList[i], &ColorButton::updatePaintColor,
                this, &SubToolBar::updateColor);
        qCDebug(dsrApp) << "Connecting updatePaintColor for color button:" << i;
    }
    colorLayout->addStretch();
    m_colorLabel->setLayout(colorLayout);

    addWidget(m_colorLabel);

    connect(this, &SubToolBar::defaultColorIndexChanged, this, [ = ](int index) {
        qCDebug(dsrApp) << "Default color index changed to:" << index;
        colorBtnList[index]->setChecked(true);
    });
}

void SubToolBar::updateColor(QColor color)
{
    qCDebug(dsrApp) << "SubToolBar::updateColor called with color:" << color;
//    qDebug() << "update text color";
    int colNum = colorIndex(color);
    qCDebug(dsrApp) << "Calculated color number:" << colNum;
    ConfigSettings::instance()->setValue("common", "color_index", colNum);
    emit currentColorChanged(color);
}

void SubToolBar::initSaveLabel()
{
    qCDebug(dsrApp) << "SubToolBar::initSaveLabel called.";
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
    saveLayout->setContentsMargins(0, 0, 0, 0);
    saveLayout->setSpacing(0);
    saveLayout->addSpacing(1);

    SaveAction saveAction = ConfigSettings::instance()->value("save", "save_op").value<SaveAction>();
    qCDebug(dsrApp) << "Initial save action:" << static_cast<int>(saveAction);

    for (auto it = toolBtnMap.cbegin(); it != toolBtnMap.cend(); ++it) {
        SaveAction  action = it->first;
        ToolButton *button = it->second;

        saveLayout->addWidget(button);

        connect(button, &ToolButton::clicked, this, [ = ] {
            qCDebug(dsrApp) << "Save option button clicked, action:" << static_cast<int>(action);
            setSaveOption(action);
        });

        connect(button, &ToolButton::onEnter, this, [ = ] {
            qCDebug(dsrApp) << "Show save tip for button:" << button->objectName();
            emit showSaveTip(button->getTips());
        });

        connect(button, &ToolButton::onExist, this, &SubToolBar::hideSaveTip);

        if (saveAction == action) {
            qCDebug(dsrApp) << "Setting save action button checked for action:" << static_cast<int>(action);
            button->setChecked(true);
        }
    }

    connect(this, &SubToolBar::saveBtnPressed, this, [ = ](int index) -> void {
        qCDebug(dsrApp) << "Save button pressed with index:" << index;
        for (auto it = toolBtnMap.cbegin(); it != toolBtnMap.cend(); ++it)
        {
            if (static_cast<SaveAction>(index) == it->first) {
                qCDebug(dsrApp) << "Clicking save button for action:" << static_cast<int>(it->first);
                return it->second->click();
            }
        }
        qCDebug(dsrApp) << "No matching save action found for index:" << index;
    });

    saveLayout->addStretch();
    saveLayout->addWidget(lowQualityText);
    saveLayout->addWidget(saveQualitySlider);
    saveLayout->addWidget(highQualityText);
    saveLayout->addSpacing(12);

    m_saveLabel->setLayout(saveLayout);
    addWidget(m_saveLabel);
    qDebug() << "SubToolBar::initSaveLabel called.";
}

void SubToolBar::switchContent(QString shapeType)
{
    qDebug() << "SubToolBar::switchContent called with shapeType:" << shapeType;
    if (shapeType == "rectangle" || shapeType == "oval") {
        qDebug() << "Switching to rectangle or oval content.";
        setCurrentWidget(m_rectLabel);
        m_currentType = shapeType;
        emit shapeChanged();
    }   else if (shapeType == "arrow") {
        qDebug() << "Switching to arrow content.";
        setCurrentWidget(m_arrowLabel);
        m_currentType = shapeType;
        emit shapeChanged();
    } else if (shapeType == "line") {
        qDebug() << "Switching to line content.";
        setCurrentWidget(m_lineLabel);
        m_currentType = shapeType;
        emit shapeChanged();
    } else if (shapeType == "text") {
        qDebug() << "Switching to text content.";
        setCurrentWidget(m_textLabel);
        m_currentType = shapeType;
        emit shapeChanged();
    } else if (shapeType == "color") {
        qDebug() << "Switching to color content.";
        int defaultColorIndex = ConfigSettings::instance()->value("common",
                                                                  "color_index").toInt();
        emit defaultColorIndexChanged(defaultColorIndex);
        setCurrentWidget(m_colorLabel);
    } else if (shapeType == "saveList") {
        qDebug() << "Switching to saveList content.";
        setCurrentWidget(m_saveLabel);
    }
    qDebug() << "subToolBar shape:" << shapeType;
}

void SubToolBar::setSaveOption(SaveAction action)
{
    qCDebug(dsrApp) << "SubToolBar::setSaveOption called with action:" << static_cast<int>(action);
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
    qCDebug(dsrApp) << "SubToolBar::setSaveQualityIndex called with quality:" << saveQuality;
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
