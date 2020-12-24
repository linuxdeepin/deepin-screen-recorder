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
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QLine>
#include <QDebug>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
    const int TOOLBAR_HEIGHT = 223;
    const int TOOLBAR_WIDTH = 40;
    const int BUTTON_SPACING = 3;
    //const int SHOT_BUTTON_SPACING = 3;
    //const int COLOR_NUM = 16;
    const QSize TOOL_ICON_SIZE = QSize(30, 30);
    const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
    const QSize TOOL_SLIDERBlUR_SIZE = QSize(35, 190);
    const QSize TOOL_SLIDER_SIZE = QSize(40, 180);
    const QSize SPLITTER_SIZE = QSize(30, 1);
    //const QSize MIN_TOOL_BUTTON_SIZE = QSize(35, 30);
}

ShotToolWidget::ShotToolWidget(DWidget *parent) : DStackedWidget(parent)
{
    initWidget();
}

ShotToolWidget::~ShotToolWidget()
{

}

void ShotToolWidget::initWidget()
{
    m_themeType = 0;
    m_themeType = ConfigSettings::instance()->value("common", "themeType").toInt();
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
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    QButtonGroup *t_funcBtnGroup = new QButtonGroup();
    t_funcBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();
    //thickOneBtn->setObjectName("thickOneBtn");
    Utils::setAccessibility(thickOneBtn, AC_SHOTTOOLWIDGET_THICK_ONE_RECT);
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();
    //thickTwoBtn->setObjectName("thickTwoBtn");
    Utils::setAccessibility(thickTwoBtn, AC_SHOTTOOLWIDGET_THICK_TWO_RECT);
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();
    //thickThreeBtn->setObjectName("thickThreeBtn");
    Utils::setAccessibility(thickThreeBtn, AC_SHOTTOOLWIDGET_THICK_THREE_RECT);
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);


    QVBoxLayout *rectLayout = new QVBoxLayout();
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
            if(m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if(m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }

        if (thickTwoBtn->isChecked()) {
            thickTwoBtn->update();
            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 1);
            if(m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if(m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }

        if (thickThreeBtn->isChecked()) {
            thickThreeBtn->update();
            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 2);
            if(m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if(m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }
    });

    thickOneBtn->click();
    ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 0);

    rectLayout->addSpacing(2);
    ToolButton *t_seperator = new ToolButton();
    pa = t_seperator->palette();
    pa.setColor(DPalette::Light, QColor("#1C1C1C"));
    pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
    t_seperator->setDisabled(true);
    t_seperator->setPalette(pa);
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
    m_mosaicRectButton = new ToolButton();
    m_mosaicRectButton->setToolTip(tr("Mosaic"));
    //m_mosaicRectButton->setObjectName("mosaicBtn");
    Utils::setAccessibility(m_mosaicRectButton, AC_SHOTTOOLWIDGET_MOSAIC_RECT);
    m_mosaicRectButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_mosaicRectButton->setIconSize(TOOL_ICON_SIZE);
    m_mosaicRectButton->setIcon(QIcon::fromTheme("Mosaic_normal"));
    btnList.append(m_mosaicRectButton);

    connect(m_blurRectButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_blurRectButton->isChecked();
        if (t_status == true){
            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());

            m_mosaicRectButton->setChecked(false);
            m_mosaicRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_mosaic", m_mosaicRectButton->isChecked());
        } else {
            m_blurRectButton->setChecked(false);
            m_blurRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());
        }
    });

    connect(m_mosaicRectButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_mosaicRectButton->isChecked();
        if (t_status == true){
            ConfigSettings::instance()->setValue("rectangle", "is_mosaic", m_mosaicRectButton->isChecked());
            m_arrowFlag = true;
            m_blurRectButton->setChecked(false);
            m_blurRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());
        } else {
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
    ToolButton *t_seperator1 = new ToolButton();
    pa = t_seperator1->palette();
    pa.setColor(DPalette::Light, QColor("#1C1C1C"));
    pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
    t_seperator1->setDisabled(true);
    t_seperator1->setPalette(pa);
    t_seperator1->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator1, 0, Qt::AlignHCenter);
    m_rectSubTool->setLayout(rectLayout);
    addWidget(m_rectSubTool);
}

void ShotToolWidget::initCircLabel()
{
    m_circSubTool = new DLabel(this);
    DPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    QButtonGroup *t_funcBtnGroup = new QButtonGroup();
    t_funcBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();
    thickOneBtn->setObjectName("thickOneBtn");
    Utils::setAccessibility(thickOneBtn, AC_SHOTTOOLWIDGET_THICK_ONE_CIRC);
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();
    //thickTwoBtn->setObjectName("thickTwoBtn");
    Utils::setAccessibility(thickTwoBtn, AC_SHOTTOOLWIDGET_THICK_TWO_CIRC);
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();
    //thickThreeBtn->setObjectName("thickThreeBtn");
    Utils::setAccessibility(thickThreeBtn, AC_SHOTTOOLWIDGET_THICK_THREE_CIRC);
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout();
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
            ConfigSettings::instance()->setValue("oval", "linewidth_index", 0);
            if(m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if(m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        } else {
            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
            thickTwoBtn->update();
            ConfigSettings::instance()->setValue("oval", "linewidth_index", 1);
            if(m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if(m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        } else {
            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
            thickThreeBtn->update();
            ConfigSettings::instance()->setValue("oval", "linewidth_index", 2);
            if(m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if(m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        } else {
            thickThreeBtn->update();
        }
    });
    thickOneBtn->click();
    ConfigSettings::instance()->setValue("oval", "linewidth_index", 0);

    rectLayout->addSpacing(2);
    ToolButton *t_seperator = new ToolButton();
    pa = t_seperator->palette();
    pa.setColor(DPalette::Light, QColor("#1C1C1C"));
    pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
    t_seperator->setDisabled(true);
    t_seperator->setPalette(pa);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(BUTTON_SPACING);

    btnList.clear();

    ConfigSettings::instance()->setValue("oval", "is_blur", false);
    ConfigSettings::instance()->setValue("oval", "is_mosaic", false);

    //模糊按钮
    m_blurCircButton = new ToolButton();
    m_blurCircButton->setToolTip(tr("Blur"));
    //m_blurCircButton->setObjectName("blurButton");
    Utils::setAccessibility(m_blurCircButton, AC_SHOTTOOLWIDGET_BLUR_CIRC);
    m_blurCircButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_blurCircButton->setIconSize(TOOL_ICON_SIZE);
    m_blurCircButton->setIcon(QIcon::fromTheme("vague_normal"));
    btnList.append(m_blurCircButton);

    //马赛克按钮
    m_mosaicCircButton = new ToolButton();
    m_mosaicCircButton->setToolTip(tr("Mosaic"));
    //m_mosaicCircButton->setObjectName("mosaicBtn");
    Utils::setAccessibility(m_mosaicCircButton, AC_SHOTTOOLWIDGET_MOSAIC_CIRC);
    m_mosaicCircButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_mosaicCircButton->setIconSize(TOOL_ICON_SIZE);
    m_mosaicCircButton->setIcon(QIcon::fromTheme("Mosaic_normal"));
    btnList.append(m_mosaicCircButton);

    connect(m_blurCircButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_blurCircButton->isChecked();

        if (t_status == true){

            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());

            m_mosaicCircButton->setChecked(false);
            m_mosaicCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_mosaic", m_mosaicCircButton->isChecked());

        } else {
            m_blurCircButton->setChecked(false);
            m_blurCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());
        }
    });

    connect(m_mosaicCircButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_mosaicCircButton->isChecked();

        if (t_status == true){
            ConfigSettings::instance()->setValue("oval", "is_mosaic", m_mosaicCircButton->isChecked());
            m_arrowFlag = true;
            m_blurCircButton->setChecked(false);
            m_blurCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());
        } else {
            m_mosaicCircButton->setChecked(false);
            m_mosaicCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_mosaic", m_mosaicCircButton->isChecked());
        }

    });



    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
        rectLayout->addSpacing(BUTTON_SPACING);
    }

    rectLayout->addSpacing(2);
    ToolButton *t_seperator1 = new ToolButton();
    pa = t_seperator1->palette();
    pa.setColor(DPalette::Light, QColor("#1C1C1C"));
    pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
    t_seperator1->setDisabled(true);
    t_seperator1->setPalette(pa);
    t_seperator1->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator1, 0, Qt::AlignHCenter);

    m_circSubTool->setLayout(rectLayout);
    addWidget(m_circSubTool);
}

void ShotToolWidget::initLineLabel()
{
    m_lineSubTool = new DLabel(this);
    DPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    QButtonGroup *t_funcBtnGroup = new QButtonGroup();
    t_funcBtnGroup->setExclusive(true);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();
    //thickOneBtn->setObjectName("thickOneBtn");
    Utils::setAccessibility(thickOneBtn, AC_SHOTTOOLWIDGET_THICK_ONE_LINE);
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();
    //thickTwoBtn->setObjectName("thickTwoBtn");
    Utils::setAccessibility(thickTwoBtn, AC_SHOTTOOLWIDGET_THICK_TWO_LINE);
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();
    //thickThreeBtn->setObjectName("thickThreeBtn");
    Utils::setAccessibility(thickThreeBtn, AC_SHOTTOOLWIDGET_THICK_THREE_LINE);
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout();
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
    thickOneBtn->click();

    ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", 0);
    ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", 0);

    rectLayout->addSpacing(2);
    ToolButton *t_seperator = new ToolButton();
    pa = t_seperator->palette();
    pa.setColor(DPalette::Light, QColor("#1C1C1C"));
    pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
    t_seperator->setDisabled(true);
    t_seperator->setPalette(pa);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(BUTTON_SPACING);

    btnList.clear();

    //直线按钮
    ToolButton *blurButton = new ToolButton();
    //blurButton->setObjectName("blurButton");
    Utils::setAccessibility(blurButton, AC_SHOTTOOLWIDGET_LINE_BUTTON);
    blurButton->setFixedSize(TOOL_BUTTON_SIZE);
    blurButton->setIconSize(TOOL_ICON_SIZE);
    blurButton->setIcon(QIcon::fromTheme("line-normal"));
    btnList.append(blurButton);

    //箭头按钮
    ToolButton *mosaicBtn = new ToolButton();
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
        if (blurButton->isChecked()){
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
        if (mosaicBtn->isChecked()){
            blurButton->setChecked(false);
            blurButton->update();
        }
    });
    bool t_arrowStatus = ConfigSettings::instance()->value("arrow", "is_straight").toBool();

    if (t_arrowStatus) {
        blurButton->click();
        m_arrowFlag = true;
    }else {
        mosaicBtn->click();
        m_arrowFlag = false;
    }

    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
        rectLayout->addSpacing(BUTTON_SPACING);
        t_funcBtnGroup->addButton(btnList[j]);
    }

    rectLayout->addSpacing(2);
    ToolButton *t_seperator1 = new ToolButton();
    pa = t_seperator1->palette();
    pa.setColor(DPalette::Light, QColor("#1C1C1C"));
    pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
    t_seperator1->setDisabled(true);
    t_seperator1->setPalette(pa);
    t_seperator1->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator1, 0, Qt::AlignHCenter);

    m_lineSubTool->setLayout(rectLayout);
    addWidget(m_lineSubTool);
}

void ShotToolWidget::initPenLabel()
{
    m_penSubTool = new DLabel(this);
    DPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();
    //thickOneBtn->setObjectName("thickOneBtn");
    Utils::setAccessibility(thickOneBtn, AC_SHOTTOOLWIDGET_THICK_ONE_PEN);
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();
    //thickTwoBtn->setObjectName("thickTwoBtn");
    Utils::setAccessibility(thickTwoBtn, AC_SHOTTOOLWIDGET_THICK_TWO_PEN);
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();
    //thickThreeBtn->setObjectName("thickThreeBtn");
    Utils::setAccessibility(thickThreeBtn, AC_SHOTTOOLWIDGET_THICK_THREE_PEN);
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout();
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
        }else {
            thickThreeBtn->update();
        }
    });
    thickOneBtn->click();
    ConfigSettings::instance()->setValue("line", "linewidth_index", 0);


    rectLayout->addSpacing(70);
    ToolButton *t_seperator = new ToolButton();
    pa = t_seperator->palette();
    pa.setColor(DPalette::Light, QColor("#1C1C1C"));
    pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
    t_seperator->setDisabled(true);
    t_seperator->setPalette(pa);
    t_seperator->setFixedSize(SPLITTER_SIZE);
    rectLayout->addWidget(t_seperator, 0, Qt::AlignHCenter);
    rectLayout->addSpacing(70);

    btnList.clear();

    m_penSubTool->setLayout(rectLayout);
    addWidget(m_penSubTool);
}

void ShotToolWidget::initTextLabel()
{
    m_textSubTool = new DLabel(this);

    DBlurEffectWidget *t_blurArea = new DBlurEffectWidget();
    t_blurArea->setBlurRectXRadius(7);
    t_blurArea->setBlurRectYRadius(7);
    t_blurArea->setRadius(15);
    t_blurArea->setMode(DBlurEffectWidget::GaussianBlur);
    t_blurArea->setBlurEnabled(true);
    t_blurArea->setBlendMode(DBlurEffectWidget::InWindowBlend);
    t_blurArea->setMaskColor(QColor(255, 255, 255, 0));
    t_blurArea->setFixedSize(TOOL_SLIDERBlUR_SIZE);

    QVBoxLayout *t_blurAreaLayout = new QVBoxLayout();

    DSlider *t_textFontSize = new DSlider(Qt::Vertical);

    t_textFontSize->slider()->setTickInterval(1);
    t_textFontSize->setFixedSize(TOOL_SLIDER_SIZE);
    t_textFontSize->setMinimum(0);
    t_textFontSize->setMaximum(10);
    t_textFontSize->setRightIcon(QIcon(":/image/newUI/normal/Aa small_normal.svg"));
    t_textFontSize->setLeftIcon(QIcon(":/image/newUI/normal/Aa big_normal.svg"));
    t_textFontSize->setIconSize(TOOL_ICON_SIZE);

    static const int indexTofontsize[11] = {9, 10, 12, 14, 18, 24, 36, 48, 64, 72, 96};
    int t_fontSize = ConfigSettings::instance()->value("text", "fontsize").toInt();
    for(int i = 0; i < 11; ++i) {
        if(indexTofontsize[i] == t_fontSize){
            t_textFontSize->setValue(i);
            break;
        }
    }

    /*
    switch (t_fontSize) {
    case 9:
        t_textFontSize->setValue(0);
        break;
    case 10:
        t_textFontSize->setValue(1);
        break;
    case 12:
        t_textFontSize->setValue(2);
        break;
    case 14:
        t_textFontSize->setValue(3);
        break;
    case 18:
        t_textFontSize->setValue(4);
        break;
    case 24:
        t_textFontSize->setValue(5);
        break;
    case 36:
        t_textFontSize->setValue(6);
        break;
    case 48:
        t_textFontSize->setValue(7);
        break;
    case 64:
        t_textFontSize->setValue(8);
        break;
    case 72:
        t_textFontSize->setValue(9);
        break;
    case 96:
        t_textFontSize->setValue(10);
        break;
    default:
        break;
    }
    */
    t_blurAreaLayout->setContentsMargins(0, 0, 0, 0);
    t_blurAreaLayout->addWidget(t_textFontSize, Qt::AlignVCenter);
    t_blurArea->setLayout(t_blurAreaLayout);

    connect(t_textFontSize, &DSlider::valueChanged, this, [ = ] {
        int t_value = t_textFontSize->value();
        ConfigSettings::instance()->setValue("text", "fontsize", indexTofontsize[t_value]);
        /*
        switch (t_value)
        {
        case 0:
            ConfigSettings::instance()->setValue("text", "fontsize", 9);
            break;
        case 1:
            ConfigSettings::instance()->setValue("text", "fontsize", 10);
            break;
        case 2:
            ConfigSettings::instance()->setValue("text", "fontsize", 12);
            break;
        case 3:
            ConfigSettings::instance()->setValue("text", "fontsize", 14);
            break;
        case 4:
            ConfigSettings::instance()->setValue("text", "fontsize", 18);
            break;
        case 5:
            ConfigSettings::instance()->setValue("text", "fontsize", 24);
            break;
        case 6:
            ConfigSettings::instance()->setValue("text", "fontsize", 36);
            break;
        case 7:
            ConfigSettings::instance()->setValue("text", "fontsize", 48);
            break;
        case 8:
            ConfigSettings::instance()->setValue("text", "fontsize", 64);
            break;
        case 9:
            ConfigSettings::instance()->setValue("text", "fontsize", 72);
            break;
        case 10:
            ConfigSettings::instance()->setValue("text", "fontsize", 96);
            break;
        default:
            break;
        }
        */
    });

    QVBoxLayout *rectLayout = new QVBoxLayout();
    rectLayout->setMargin(3);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(3);

    rectLayout->addWidget(t_blurArea, Qt::AlignTop);

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
            }else {
                setCurrentWidget(m_circSubTool);
            }
        }

        if (shapeType == "arrow") {
            //            setCurrentWidget(m_lineSubTool);
            if (m_lineInitFlag == false) {
                initLineLabel();
                setCurrentWidget(m_lineSubTool);
                m_lineInitFlag = true;
            }else {
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
            }else {
                setCurrentWidget(m_textSubTool);
            }
        }
    }
}
void ShotToolWidget::colorChecked(QString colorType)
{
    Q_UNUSED(colorType);
    //if(m_currentType == "rectangle"){
    if(m_blurRectButton && m_blurRectButton->isChecked())
        m_blurRectButton->click();
    if(m_mosaicRectButton && m_mosaicRectButton->isChecked())
        m_mosaicRectButton->click();
    //}else if(m_currentType == "oval"){
    if(m_blurCircButton && m_blurCircButton->isChecked())
        m_blurCircButton->click();
    if(m_mosaicCircButton && m_mosaicCircButton->isChecked())
        m_mosaicCircButton->click();
    // }
}
