// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shottoolwidget.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"

#include <DSlider>
#include <DLineEdit>
#include <DMenu>
#include <DFrame>
#include <DBlurEffectWidget>
#include <DPalette>

#include <QAction>
#include <QButtonGroup>
#include <QStyleFactory>
#include <QLine>
#include <QDebug>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 223;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 3;
const QSize TOOL_ICON_SIZE = QSize(30, 30);
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize TOOL_SLIDERBlUR_SIZE = QSize(35, 190);
const QSize TOOL_SLIDER_SIZE = QSize(40, 180);
const QSize SPLITTER_SIZE = QSize(30, 1);
}

ShotToolWidget::ShotToolWidget(DWidget *parent) :
    DStackedWidget(parent),
    m_buttonGroup(nullptr),
    m_thicknessBtnGroup(nullptr),
    m_funcBtnGroup(nullptr),
    m_rectLayout(nullptr)
{
    initWidget();
}



void ShotToolWidget::initWidget()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    m_arrowFlag = false;

    m_rectInitFlag = false;
    m_circInitFlag = false;
    m_lineInitFlag = false;
    m_penInitFlag = false;
    m_textInitFlag = false;
    m_blurRectButton = nullptr;
    m_mosaicRectButton = nullptr;
    m_blurCircButton = nullptr;
    m_mosaicCircButton = nullptr;
}

void ShotToolWidget::initRectLabel()
{
    m_rectSubTool = new DLabel(this);
    DPalette pa;
    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup(this);
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(false);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton(this);
    //thickOneBtn->setObjectName("thickOneBtn");
    Utils::setAccessibility(thickOneBtn, AC_SHOTTOOLWIDGET_THICK_ONE_RECT);
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton(this);
    //thickTwoBtn->setObjectName("thickTwoBtn");
    Utils::setAccessibility(thickTwoBtn, AC_SHOTTOOLWIDGET_THICK_TWO_RECT);
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton(this);
    //thickThreeBtn->setObjectName("thickThreeBtn");
    Utils::setAccessibility(thickThreeBtn, AC_SHOTTOOLWIDGET_THICK_THREE_RECT);
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);


    QVBoxLayout *rectLayout = new QVBoxLayout(this);
    rectLayout->setContentsMargins(1, 4, 0, 0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(BUTTON_SPACING);
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

    connect(t_thicknessBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        Q_UNUSED(status);
        //DPalette pa;
        if (thickOneBtn->isChecked()) {
            thickOneBtn->update();
            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 0);
            if (m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if (m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }

        if (thickTwoBtn->isChecked()) {
            thickTwoBtn->update();
            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 1);
            if (m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if (m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }

        if (thickThreeBtn->isChecked()) {
            thickThreeBtn->update();
            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 2);
            if (m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if (m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }
    });


    int linewidth_index = ConfigSettings::instance()->value("rectangle", "linewidth_index").toInt();
    if (linewidth_index == 0) {
        thickOneBtn->click();
    } else if (linewidth_index == 1) {
        thickTwoBtn->click();
    } else {
        thickThreeBtn->click();
    }

    rectLayout->addSpacing(2);
    //分割线
    ToolButton *t_seperator = new ToolButton(this);
    if (Utils::themeType == 1) {
        //t_seperator->setStyleSheet("border:1px groove rgba(0, 0, 0, 77);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(0, 0, 0, 77);border-radius:0px;background-color:rgba(0, 0, 0, 77)");
    } else {
        //t_seperator->setStyleSheet("border:1px groove rgba(255, 255, 255, 26);border-radius:0px;");
        t_seperator->setStyleSheet("border:0px solid rgba(255, 255, 255, 26);border-radius:0px;background-color:rgba(255, 255, 255, 26)");
    }

    t_seperator->setDisabled(true);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(BUTTON_SPACING);


    btnList.clear();
    ConfigSettings::instance()->setValue("rectangle", "is_blur", false);
    ConfigSettings::instance()->setValue("rectangle", "is_mosaic", false);
    //模糊按钮
    m_blurRectButton = new ToolButton(this);

    pa = m_blurRectButton->palette();
    m_blurRectButton->setToolTip(tr("Blur"));
    //m_blurRectButton->setObjectName("blurButton");
    Utils::setAccessibility(m_blurRectButton, AC_SHOTTOOLWIDGET_BLUR_RECT);
    m_blurRectButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_blurRectButton->setIconSize(TOOL_ICON_SIZE);
    m_blurRectButton->setIcon(QIcon::fromTheme("vague_normal"));
    btnList.append(m_blurRectButton);
    //马赛克按钮
    m_mosaicRectButton = new ToolButton(this);
    m_mosaicRectButton->setToolTip(tr("Mosaic"));
    //m_mosaicRectButton->setObjectName("mosaicBtn");
    Utils::setAccessibility(m_mosaicRectButton, AC_SHOTTOOLWIDGET_MOSAIC_RECT);
    m_mosaicRectButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_mosaicRectButton->setIconSize(TOOL_ICON_SIZE);
    m_mosaicRectButton->setIcon(QIcon::fromTheme("Mosaic_normal"));
    btnList.append(m_mosaicRectButton);

    connect(m_blurRectButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_blurRectButton->isChecked();
        if (t_status == true)
        {
            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());

            m_mosaicRectButton->setChecked(false);
            m_mosaicRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_mosaic", m_mosaicRectButton->isChecked());
        } else
        {
            m_blurRectButton->setChecked(false);
            m_blurRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());
        }
    });

    connect(m_mosaicRectButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_mosaicRectButton->isChecked();
        if (t_status == true)
        {
            ConfigSettings::instance()->setValue("rectangle", "is_mosaic", m_mosaicRectButton->isChecked());
            m_arrowFlag = true;
            m_blurRectButton->setChecked(false);
            m_blurRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());
        } else
        {
            m_mosaicRectButton->setChecked(false);
            m_mosaicRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_mosaic", m_mosaicRectButton->isChecked());
        }

    });

    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
        rectLayout->addSpacing(BUTTON_SPACING);
    }

    rectLayout->addSpacing(2);
    ToolButton *t_seperator1 = new ToolButton(this);
    if (Utils::themeType == 1) {
        //t_seperator1->setStyleSheet("border:1px groove rgba(0, 0, 0, 77);border-radius:0px");
        t_seperator1->setStyleSheet("border:0px solid rgba(0, 0, 0, 77);border-radius:0px;background-color:rgba(0, 0, 0, 77)");
    } else {
        //t_seperator1->setStyleSheet("border:1px groove rgba(255, 255, 255, 26);border-radius:0px");
        t_seperator1->setStyleSheet("border:0px solid rgba(255, 255, 255, 26);border-radius:0px;background-color:rgba(255, 255, 255, 26)");
    }
    qDebug() << "test";
    t_seperator1->setDisabled(true);
    t_seperator1->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator1, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(2);
    m_rectSubTool->setLayout(rectLayout);
    addWidget(m_rectSubTool);
}

void ShotToolWidget::initCircLabel()
{
    m_circSubTool = new DLabel(this);
    DPalette pa;

    //选择粗细按钮组
    m_thicknessBtnGroup = new QButtonGroup(this);
    m_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    m_funcBtnGroup = new QButtonGroup(this);
    m_funcBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton(this);
    thickOneBtn->setObjectName("thickOneBtn");
    Utils::setAccessibility(thickOneBtn, AC_SHOTTOOLWIDGET_THICK_ONE_CIRC);
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton(this);
    //thickTwoBtn->setObjectName("thickTwoBtn");
    Utils::setAccessibility(thickTwoBtn, AC_SHOTTOOLWIDGET_THICK_TWO_CIRC);
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton(this);
    //thickThreeBtn->setObjectName("thickThreeBtn");
    Utils::setAccessibility(thickThreeBtn, AC_SHOTTOOLWIDGET_THICK_THREE_CIRC);
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);

    m_rectLayout = new QVBoxLayout(this);
    m_rectLayout->setContentsMargins(1, 4, 0, 0);
    m_rectLayout->setSpacing(0);
    m_rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        m_rectLayout->addWidget(btnList[i]);
        m_rectLayout->addSpacing(BUTTON_SPACING);
        m_thicknessBtnGroup->addButton(btnList[i]);
    }

    connect(m_thicknessBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        Q_UNUSED(status);
        //DPalette pa;
        if (thickOneBtn->isChecked()) {
            thickOneBtn->update();
            ConfigSettings::instance()->setValue("oval", "linewidth_index", 0);
            if (m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if (m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        } else {
            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
            thickTwoBtn->update();
            ConfigSettings::instance()->setValue("oval", "linewidth_index", 1);
            if (m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if (m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        } else {
            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
            thickThreeBtn->update();
            ConfigSettings::instance()->setValue("oval", "linewidth_index", 2);
            if (m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if (m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        } else {
            thickThreeBtn->update();
        }
    });

    int linewidth_index = ConfigSettings::instance()->value("oval", "linewidth_index").toInt();
    if (linewidth_index == 0) {
        thickOneBtn->click();
    } else if (linewidth_index == 1) {
        thickTwoBtn->click();
    } else {
        thickThreeBtn->click();
    }


    m_rectLayout->addSpacing(2);
    ToolButton *t_seperator = new ToolButton(this);
    if (Utils::themeType == 1) {
        //t_seperator->setStyleSheet("border:1px groove rgba(0, 0, 0, 77);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(0, 0, 0, 77);border-radius:0px;background-color:rgba(0, 0, 0, 77)");
    } else {
        //t_seperator->setStyleSheet("border:1px groove rgba(255, 255, 255, 26);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(255, 255, 255, 26);border-radius:0px;background-color:rgba(255, 255, 255, 26)");
    }
    t_seperator->setDisabled(true);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    m_rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    m_rectLayout->addSpacing(BUTTON_SPACING);

    btnList.clear();

    ConfigSettings::instance()->setValue("oval", "is_blur", false);
    ConfigSettings::instance()->setValue("oval", "is_mosaic", false);

    //模糊按钮
    m_blurCircButton = new ToolButton(this);
    m_blurCircButton->setToolTip(tr("Blur"));
    //m_blurCircButton->setObjectName("blurButton");
    Utils::setAccessibility(m_blurCircButton, AC_SHOTTOOLWIDGET_BLUR_CIRC);
    m_blurCircButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_blurCircButton->setIconSize(TOOL_ICON_SIZE);
    m_blurCircButton->setIcon(QIcon::fromTheme("vague_normal"));
    btnList.append(m_blurCircButton);

    //马赛克按钮
    m_mosaicCircButton = new ToolButton(this);
    m_mosaicCircButton->setToolTip(tr("Mosaic"));
    //m_mosaicCircButton->setObjectName("mosaicBtn");
    Utils::setAccessibility(m_mosaicCircButton, AC_SHOTTOOLWIDGET_MOSAIC_CIRC);
    m_mosaicCircButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_mosaicCircButton->setIconSize(TOOL_ICON_SIZE);
    m_mosaicCircButton->setIcon(QIcon::fromTheme("Mosaic_normal"));
    btnList.append(m_mosaicCircButton);

    connect(m_blurCircButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_blurCircButton->isChecked();

        if (t_status == true)
        {

            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());

            m_mosaicCircButton->setChecked(false);
            m_mosaicCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_mosaic", m_mosaicCircButton->isChecked());

        } else
        {
            m_blurCircButton->setChecked(false);
            m_blurCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());
        }
    });

    connect(m_mosaicCircButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_mosaicCircButton->isChecked();

        if (t_status == true)
        {
            ConfigSettings::instance()->setValue("oval", "is_mosaic", m_mosaicCircButton->isChecked());
            m_arrowFlag = true;
            m_blurCircButton->setChecked(false);
            m_blurCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());
        } else
        {
            m_mosaicCircButton->setChecked(false);
            m_mosaicCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_mosaic", m_mosaicCircButton->isChecked());
        }

    });



    for (int j = 0; j < btnList.length(); j++) {
        m_rectLayout->addWidget(btnList[j]);
        m_rectLayout->addSpacing(BUTTON_SPACING);
    }

    m_rectLayout->addSpacing(2);
    ToolButton *t_seperator1 = new ToolButton(this);
    if (Utils::themeType == 1) {
        //t_seperator1->setStyleSheet("border:1px groove rgba(0, 0, 0, 77);border-radius:0px");
        t_seperator1->setStyleSheet("border:0px solid rgba(0, 0, 0, 77);border-radius:0px;background-color:rgba(0, 0, 0, 77)");
    } else {
        //t_seperator1->setStyleSheet("border:1px groove rgba(255, 255, 255, 26);border-radius:0px");
        t_seperator1->setStyleSheet("border:0px solid rgba(255, 255, 255, 26);border-radius:0px;background-color:rgba(255, 255, 255, 26)");
    }
    t_seperator1->setDisabled(true);
    t_seperator1->setFixedSize(SPLITTER_SIZE);
    m_rectLayout->addWidget(t_seperator1, 0, Qt::AlignHCenter);

    m_rectLayout->addSpacing(2);
    m_circSubTool->setLayout(m_rectLayout);
    addWidget(m_circSubTool);
}

void ShotToolWidget::initLineLabel()
{
    m_lineSubTool = new DLabel(this);
    DPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup(this);
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    QButtonGroup *t_funcBtnGroup = new QButtonGroup(this);
    t_funcBtnGroup->setExclusive(true);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton(this);
    //thickOneBtn->setObjectName("thickOneBtn");
    Utils::setAccessibility(thickOneBtn, AC_SHOTTOOLWIDGET_THICK_ONE_LINE);
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton(this);
    //thickTwoBtn->setObjectName("thickTwoBtn");
    Utils::setAccessibility(thickTwoBtn, AC_SHOTTOOLWIDGET_THICK_TWO_LINE);
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton(this);
    //thickThreeBtn->setObjectName("thickThreeBtn");
    Utils::setAccessibility(thickThreeBtn, AC_SHOTTOOLWIDGET_THICK_THREE_LINE);
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout(this);
    rectLayout->setContentsMargins(1, 4, 0, 0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(BUTTON_SPACING);
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

    connect(t_thicknessBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        Q_UNUSED(status);
        if (thickOneBtn->isChecked()) {
            thickOneBtn->update();
            ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", 0);
            ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", 0);
        } else {
            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
            thickTwoBtn->update();
            ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", 1);
            ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", 1);
        } else {
            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
            thickThreeBtn->update();
            ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", 2);
            ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", 2);
        } else {
            thickThreeBtn->update();
        }
    });

    int linewidth_index = ConfigSettings::instance()->value("arrow", "arrow_linewidth_index").toInt();
    if (linewidth_index == 0) {
        thickOneBtn->click();
    } else if (linewidth_index == 1) {
        thickTwoBtn->click();
    } else {
        thickThreeBtn->click();
    }

    rectLayout->addSpacing(2);
    ToolButton *t_seperator = new ToolButton(this);
    if (Utils::themeType == 1) {
        //t_seperator->setStyleSheet("border:1px groove rgba(0, 0, 0, 77);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(0, 0, 0, 77);border-radius:0px;background-color:rgba(0, 0, 0, 77)");
    } else {
        //t_seperator->setStyleSheet("border:1px groove rgba(255, 255, 255, 26);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(255, 255, 255, 26);border-radius:0px;background-color:rgba(255, 255, 255, 26)");
    }
    t_seperator->setDisabled(true);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(BUTTON_SPACING);

    btnList.clear();

    //直线按钮
    ToolButton *blurButton = new ToolButton(this);
    //blurButton->setObjectName("blurButton");
    Utils::setAccessibility(blurButton, AC_SHOTTOOLWIDGET_LINE_BUTTON);
    blurButton->setFixedSize(TOOL_BUTTON_SIZE);
    blurButton->setIconSize(TOOL_ICON_SIZE);
    blurButton->setIcon(QIcon::fromTheme("line-normal"));
    btnList.append(blurButton);

    //箭头按钮
    ToolButton *mosaicBtn = new ToolButton(this);
    //mosaicBtn->setObjectName("mosaicBtn");
    Utils::setAccessibility(mosaicBtn, AC_SHOTTOOLWIDGET_ARROW_BUTTON);
    mosaicBtn->setFixedSize(TOOL_BUTTON_SIZE);
    mosaicBtn->setIconSize(TOOL_ICON_SIZE);
    mosaicBtn->setIcon(QIcon::fromTheme("Arrow-normal"));
    btnList.append(mosaicBtn);

    connect(blurButton, &ToolButton::clicked, this, [ = ] {
        blurButton->setIcon(QIcon::fromTheme("line-normal"));
        blurButton->update();
        emit changeArrowAndLine(0);
        ConfigSettings::instance()->setValue("arrow", "is_straight", true);
        m_arrowFlag = false;
        if (blurButton->isChecked())
        {
            mosaicBtn->setChecked(false);
            mosaicBtn->setIcon(QIcon::fromTheme("Arrow-normal"));
            mosaicBtn->update();
        }
    });
    connect(mosaicBtn, &ToolButton::clicked, this, [ = ] {
        mosaicBtn->update();
        emit changeArrowAndLine(1);
        ConfigSettings::instance()->setValue("arrow", "is_straight", false);
        m_arrowFlag = true;
        if (mosaicBtn->isChecked())
        {
            blurButton->setChecked(false);
            blurButton->update();
        }
    });
    bool t_arrowStatus = ConfigSettings::instance()->value("arrow", "is_straight").toBool();

    if (t_arrowStatus) {
        blurButton->click();
        m_arrowFlag = true;
    } else {
        mosaicBtn->click();
        m_arrowFlag = false;
    }

    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
        rectLayout->addSpacing(BUTTON_SPACING);
        t_funcBtnGroup->addButton(btnList[j]);
    }

    rectLayout->addSpacing(2);
    ToolButton *t_seperator1 = new ToolButton(this);
    if (Utils::themeType == 1) {
        //t_seperator1->setStyleSheet("border:1px groove rgba(0, 0, 0, 77);border-radius:0px");
        t_seperator1->setStyleSheet("border:0px solid rgba(0, 0, 0, 77);border-radius:0px;background-color:rgba(0, 0, 0, 77)");
    } else {
        //t_seperator1->setStyleSheet("border:1px groove rgba(255, 255, 255, 26);border-radius:0px");
        t_seperator1->setStyleSheet("border:0px solid rgba(255, 255, 255, 26);border-radius:0px;background-color:rgba(255, 255, 255, 26)");
    }

    t_seperator1->setDisabled(true);
    t_seperator1->setPalette(pa);
    t_seperator1->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator1, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(2);

    m_lineSubTool->setLayout(rectLayout);
    addWidget(m_lineSubTool);
}

void ShotToolWidget::initPenLabel()
{
    m_penSubTool = new DLabel(this);
    DPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup(this);
    t_thicknessBtnGroup->setExclusive(true);

    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton(this);
    //thickOneBtn->setObjectName("thickOneBtn");
    Utils::setAccessibility(thickOneBtn, AC_SHOTTOOLWIDGET_THICK_ONE_PEN);
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton(this);
    //thickTwoBtn->setObjectName("thickTwoBtn");
    Utils::setAccessibility(thickTwoBtn, AC_SHOTTOOLWIDGET_THICK_TWO_PEN);
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton(this);
    //thickThreeBtn->setObjectName("thickThreeBtn");
    Utils::setAccessibility(thickThreeBtn, AC_SHOTTOOLWIDGET_THICK_THREE_PEN);
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout(this);
    rectLayout->setContentsMargins(1, 4, 0, 0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        rectLayout->addSpacing(BUTTON_SPACING);
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

    connect(t_thicknessBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        Q_UNUSED(status);
        //DPalette pa;
        if (thickOneBtn->isChecked()) {
            thickOneBtn->update();
            ConfigSettings::instance()->setValue("line", "linewidth_index", 0);
        } else {
            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
            thickTwoBtn->update();
            ConfigSettings::instance()->setValue("line", "linewidth_index", 1);
        } else {
            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
            thickThreeBtn->update();
            ConfigSettings::instance()->setValue("line", "linewidth_index", 2);
        } else {
            thickThreeBtn->update();
        }
    });

    int linewidth_index = ConfigSettings::instance()->value("line", "linewidth_index").toInt();
    if (linewidth_index == 0) {
        thickOneBtn->click();
    } else if (linewidth_index == 1) {
        thickTwoBtn->click();
    } else {
        thickThreeBtn->click();
    }


    //rectLayout->addSpacing(70);
    rectLayout->addSpacing(40);
    ToolButton *t_seperator = new ToolButton(this);
    if (Utils::themeType == 1) {
        //t_seperator->setStyleSheet("border:1px groove rgba(0, 0, 0, 77);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(0, 0, 0, 77);border-radius:0px;background-color:rgba(0, 0, 0, 77)");
    } else {
        //t_seperator->setStyleSheet("border:1px groove rgba(255, 255, 255, 26);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(255, 255, 255, 26);border-radius:0px;background-color:rgba(255, 255, 255, 26)");
    }
    t_seperator->setDisabled(true);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    //rectLayout->addSpacing(70);
    rectLayout->addSpacing(40);
    btnList.clear();

    m_penSubTool->setLayout(rectLayout);
    addWidget(m_penSubTool);
}

void ShotToolWidget::initTextLabel()
{
    m_textSubTool = new DLabel(this);

    DBlurEffectWidget *t_blurArea = new DBlurEffectWidget(this);
    t_blurArea->setBlurRectXRadius(7);
    t_blurArea->setBlurRectYRadius(7);
    t_blurArea->setRadius(15);
    t_blurArea->setMode(DBlurEffectWidget::GaussianBlur);
    t_blurArea->setBlurEnabled(true);
    t_blurArea->setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    t_blurArea->setMaskColor(QColor(255, 255, 255, 0));
    t_blurArea->setFixedSize(TOOL_SLIDERBlUR_SIZE);

    QVBoxLayout *t_blurAreaLayout = new QVBoxLayout(this);

    DSlider *t_textFontSize = new DSlider(Qt::Vertical, this);

    t_textFontSize->slider()->setTickInterval(1);
    t_textFontSize->setFixedSize(TOOL_SLIDER_SIZE);
    t_textFontSize->setMinimum(0);
    t_textFontSize->setMaximum(10);
    t_textFontSize->setRightIcon(QIcon(":/newUI/normal/Aa small_normal.svg"));
    t_textFontSize->setLeftIcon(QIcon(":/newUI/normal/Aa big_normal.svg"));
    t_textFontSize->setIconSize(TOOL_ICON_SIZE);

    static const int indexTofontsize[11] = {9, 10, 12, 14, 18, 24, 36, 48, 64, 72, 96};
    int t_fontSize = ConfigSettings::instance()->value("text", "fontsize").toInt();
    for (int i = 0; i < 11; ++i) {
        if (indexTofontsize[i] == t_fontSize) {
            t_textFontSize->setValue(i);
            break;
        }
    }
    t_blurAreaLayout->setContentsMargins(0, 0, 0, 0);
    t_blurAreaLayout->addWidget(t_textFontSize, Qt::AlignVCenter);
    t_blurArea->setLayout(t_blurAreaLayout);

    connect(t_textFontSize, &DSlider::valueChanged, this, [ = ] {
        int t_value = t_textFontSize->value();
        ConfigSettings::instance()->setValue("text", "fontsize", indexTofontsize[t_value]);
    });

    ToolButton *t_seperator = new ToolButton(this);
    if (Utils::themeType == 1) {
        //t_seperator->setStyleSheet("border:1px groove rgba(0, 0, 0, 77);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(0, 0, 0, 77);border-radius:0px;background-color:rgba(0, 0, 0, 77)");
    } else {
        //t_seperator->setStyleSheet("border:1px groove rgba(255, 255, 255, 26);border-radius:0px");
        t_seperator->setStyleSheet("border:0px solid rgba(255, 255, 255, 26);border-radius:0px;background-color:rgba(255, 255, 255, 26)");
    }
    t_seperator->setDisabled(true);
    t_seperator->setFixedSize(SPLITTER_SIZE);

    QVBoxLayout *rectLayout = new QVBoxLayout(this);
    rectLayout->setMargin(2);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(0);

    rectLayout->addWidget(t_blurArea, Qt::AlignTop);
    rectLayout->addSpacing(10);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);

    m_textSubTool->setLayout(rectLayout);
    addWidget(m_textSubTool);
}

void ShotToolWidget::switchContent(QString shapeType)
{
    QString t_blurVal;
    QString t_mosaicVal;
    if (!shapeType.isEmpty()) {
        if (shapeType == "rectangle") {
            if (m_rectInitFlag == false) {
                initRectLabel();
                setCurrentWidget(m_rectSubTool);
                m_rectInitFlag = true;
            } else {
                setCurrentWidget(m_rectSubTool);
            }

        }

        if (shapeType == "oval") {
            if (m_circInitFlag == false) {
                initCircLabel();
                setCurrentWidget(m_circSubTool);
                m_circInitFlag = true;
            } else {
                setCurrentWidget(m_circSubTool);
            }
        }

        if (shapeType == "arrow") {
            //            setCurrentWidget(m_lineSubTool);
            if (m_lineInitFlag == false) {
                initLineLabel();
                setCurrentWidget(m_lineSubTool);
                m_lineInitFlag = true;
            } else {
                setCurrentWidget(m_lineSubTool);
            }
        }

        if (shapeType == "line") {
            //            setCurrentWidget(m_penSubTool);
            if (m_penInitFlag == false) {
                initPenLabel();
                setCurrentWidget(m_penSubTool);
                m_penInitFlag = true;
            } else {
                setCurrentWidget(m_penSubTool);
            }
        }

        if (shapeType == "text") {
            //            setCurrentWidget(m_textSubTool);
            if (m_textInitFlag == false) {
                initTextLabel();
                setCurrentWidget(m_textSubTool);
                m_textInitFlag = true;
            } else {
                setCurrentWidget(m_textSubTool);
            }
        }
    }
}
void ShotToolWidget::colorChecked(QString colorType)
{
    Q_UNUSED(colorType);
    //if(m_currentType == "rectangle"){
    if (m_blurRectButton && m_blurRectButton->isChecked())
        m_blurRectButton->click();
    if (m_mosaicRectButton && m_mosaicRectButton->isChecked())
        m_mosaicRectButton->click();
    //}else if(m_currentType == "oval"){
    if (m_blurCircButton && m_blurCircButton->isChecked())
        m_blurCircButton->click();
    if (m_mosaicCircButton && m_mosaicCircButton->isChecked())
        m_mosaicCircButton->click();
    // }
}
ShotToolWidget::~ShotToolWidget()
{
//指定父指针，利用qt对象树自动析构机制释放资源
//    if(nullptr != m_buttonGroup){
//        m_buttonGroup->deleteLater();
//        m_buttonGroup = nullptr;
//    }
//    if(nullptr != m_thicknessBtnGroup){
//        m_thicknessBtnGroup->deleteLater();
//        m_thicknessBtnGroup = nullptr;
//    }
//    if(nullptr != m_funcBtnGroup){
//        m_funcBtnGroup->deleteLater();
//        m_funcBtnGroup = nullptr;
//    }
//    if(nullptr != m_rectLayout){
//        m_rectLayout->deleteLater();
//        m_rectLayout = nullptr;
//    }
}
