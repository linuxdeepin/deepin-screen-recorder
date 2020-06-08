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

#include <DSlider>
#include <DLineEdit>
#include <DMenu>
#include <QAction>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QLine>
#include <DPalette>
#include <QDebug>
#include <DFrame>
#include <DBlurEffectWidget>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 223;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 3;
const int SHOT_BUTTON_SPACING = 3;
const int COLOR_NUM = 16;
const QSize TOOL_ICON_SIZE = QSize(30, 30);
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize TOOL_SLIDERBlUR_SIZE = QSize(35, 190);
const QSize TOOL_SLIDER_SIZE = QSize(40, 180);
const QSize SPLITTER_SIZE = QSize(30, 1);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(35, 30);
}

ShotToolWidget::ShotToolWidget(DWidget *parent) : DStackedWidget(parent)
{
    initWidget();
}

ShotToolWidget::~ShotToolWidget()
{
//    delete m_rectSubTool;
//    delete m_circSubTool;
//    delete m_lineSubTool;
//    delete m_penSubTool;
//    delete m_textSubTool;
//    delete m_blurRectButton;
//    delete m_mosaicRectButton;
//    delete m_blurCircButton;
//    delete m_mosaicCircButton;
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
//    initRectLabel();
//    initCircLabel();
//    initLineLabel();
//    initPenLabel();
//    initTextLabel();
//    setCurrentWidget(m_rectSubTool);
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

//    pa = thickOneBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickOneBtn->setPalette(pa);

//    thickOneBtn->setToolTip(tr("fine"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
//    thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//    if (m_themeType == 1) {
//        thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickOneBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush small_normal.svg"));
//    }

    btnList.append(thickOneBtn);

//    connect(thickOneBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickOneBtn->setIcon(QIcon(":/image/newUI/press/brush small_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickOneBtn->setIcon(QIcon(":/image/newUI/dark/press/brush small_press.svg"));

//        }
//    });
    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();

//    pa = thickTwoBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickTwoBtn->setPalette(pa);

//    thickTwoBtn->setToolTip(tr("medium"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
//    thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//    if (m_themeType == 1) {
//        thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush medium_normal.svg"));
//    }
    btnList.append(thickTwoBtn);

//    connect(thickTwoBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/press/brush medium_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/press/brush medium_press.svg"));

//        }
//    });
    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();

//    pa = thickThreeBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickThreeBtn->setPalette(pa);

//    thickThreeBtn->setToolTip(tr("thick"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
//    thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//    if (m_themeType == 1) {
//        thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush big_normal.svg"));
//    }
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
    btnList.append(thickThreeBtn);

//    connect(thickThreeBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/press/brush big_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/press/brush big_press.svg"));

//        }
//    });
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

        DPalette pa;
        if (thickOneBtn->isChecked()) {
//            pa = thickOneBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickOneBtn->setPalette(pa);
//            thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));
//            if (m_themeType == 1) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush small_checked.svg"));
//            }

            thickOneBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 0);
            if(m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if(m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }

//        else {
////            pa = thickOneBtn->palette();
////            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
////            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
////            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
////            thickOneBtn->setPalette(pa);
////            thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//            if (m_themeType == 1) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush small_normal.svg"));
//            }

//            thickOneBtn->update();
//        }

        if (thickTwoBtn->isChecked()) {
//            pa = thickTwoBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickTwoBtn->setPalette(pa);
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));
//            if (m_themeType == 1) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush medium_checked.svg"));
//            }

            thickTwoBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 1);
            if(m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if(m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }

//        else {
////            pa = thickTwoBtn->palette();
////            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
////            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
////            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
////            thickTwoBtn->setPalette(pa);
////            thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//            if (m_themeType == 1) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush medium_normal.svg"));
//            }

//            thickTwoBtn->update();
//        }

        if (thickThreeBtn->isChecked()) {
//            pa = thickThreeBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickThreeBtn->setPalette(pa);
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));
//            if (m_themeType == 1) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush big_checked.svg"));
//            }

            thickThreeBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 2);
            if(m_blurRectButton && m_blurRectButton->isChecked())
                m_blurRectButton->click();
            if(m_mosaicRectButton && m_mosaicRectButton->isChecked())
                m_mosaicRectButton->click();
        }

//        else {
////            pa = thickThreeBtn->palette();
////            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
////            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
////            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
////            thickThreeBtn->setPalette(pa);
////            thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//            if (m_themeType == 1) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush big_normal.svg"));
//            }

//            thickThreeBtn->update();
//        }
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
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_blurRectButton->setPalette(pa);

    m_blurRectButton->setToolTip(tr("Blur"));
    m_blurRectButton->setObjectName("blurButton");
    m_blurRectButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_blurRectButton->setIconSize(TOOL_ICON_SIZE);
    m_blurRectButton->setIcon(QIcon::fromTheme("vague_normal"));
//    m_blurRectButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//    if (m_themeType == 1) {
//        m_blurRectButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        m_blurRectButton->setIcon(QIcon(":/image/newUI/dark/normal/vague_normal.svg"));
//    }

//    connect(m_blurRectButton, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            m_blurRectButton->setIcon(QIcon(":/image/newUI/press/vague_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            m_blurRectButton->setIcon(QIcon(":/image/newUI/dark/press/vague_press.svg"));

//        }
//    });

    btnList.append(m_blurRectButton);
    //马赛克按钮
    m_mosaicRectButton = new ToolButton();

//    pa = m_mosaicRectButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_mosaicRectButton->setPalette(pa);

    m_mosaicRectButton->setToolTip(tr("Mosaic"));
    m_mosaicRectButton->setObjectName("mosaicBtn");
    m_mosaicRectButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_mosaicRectButton->setIconSize(TOOL_ICON_SIZE);
    m_mosaicRectButton->setIcon(QIcon::fromTheme("Mosaic_normal"));
//    m_mosaicRectButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//    if (m_themeType == 1) {
//        m_mosaicRectButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        m_mosaicRectButton->setIcon(QIcon(":/image/newUI/dark/normal/Mosaic_normal.svg"));
//    }

//    connect(m_mosaicRectButton, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            m_mosaicRectButton->setIcon(QIcon(":/image/newUI/press/Mosaic_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            m_mosaicRectButton->setIcon(QIcon(":/image/newUI/dark/press/Mosaic_press.svg"));

//        }
//    });

    btnList.append(m_mosaicRectButton);

    connect(m_blurRectButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_blurRectButton->isChecked();

        if (t_status == true)
        {
            DPalette pa;
//            pa = m_blurRectButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_blurRectButton->setPalette(pa);
//            m_blurRectButton->setIcon(QIcon(":/image/newUI/checked/vague_checked.svg"));
//            if (m_themeType == 1) {
//                m_blurRectButton->setIcon(QIcon(":/image/newUI/checked/vague_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_blurRectButton->setIcon(QIcon(":/image/newUI/dark/checked/vague_checked.svg"));
//            }

            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());

            m_mosaicRectButton->setChecked(false);
//            pa = m_mosaicRectButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_mosaicRectButton->setPalette(pa);
//            m_mosaicRectButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//            if (m_themeType == 1) {
//                m_mosaicRectButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_mosaicRectButton->setIcon(QIcon(":/image/newUI/dark/normal/Mosaic_normal.svg"));
//            }

            m_mosaicRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_mosaic", m_mosaicRectButton->isChecked());

        }

        else
        {
            DPalette pa;
            m_blurRectButton->setChecked(false);
//            pa = m_blurRectButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_blurRectButton->setPalette(pa);
//            m_blurRectButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//            if (m_themeType == 1) {
//                m_blurRectButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_blurRectButton->setIcon(QIcon(":/image/newUI/dark/normal/vague_normal.svg"));
//            }

            m_blurRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());
        }
    });

    connect(m_mosaicRectButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_mosaicRectButton->isChecked();

        if (t_status == true)
        {
            DPalette pa;
//            pa = m_mosaicRectButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_mosaicRectButton->setPalette(pa);
//            m_mosaicRectButton->setIcon(QIcon(":/image/newUI/checked/Mosaic_checked.svg"));
//            if (m_themeType == 1) {
//                m_mosaicRectButton->setIcon(QIcon(":/image/newUI/checked/Mosaic_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_mosaicRectButton->setIcon(QIcon(":/image/newUI/dark/checked/Mosaic_checked.svg"));
//            }

            ConfigSettings::instance()->setValue("rectangle", "is_mosaic", m_mosaicRectButton->isChecked());
            m_arrowFlag = true;
            m_blurRectButton->setChecked(false);
//            pa = m_blurRectButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_blurRectButton->setPalette(pa);
//            m_blurRectButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//            if (m_themeType == 1) {
//                m_blurRectButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_blurRectButton->setIcon(QIcon(":/image/newUI/dark/normal/vague_normal.svg"));
//            }

            m_blurRectButton->update();
            ConfigSettings::instance()->setValue("rectangle", "is_blur", m_blurRectButton->isChecked());
        }

        else
        {
            DPalette pa;
            m_mosaicRectButton->setChecked(false);
//            pa = m_mosaicRectButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_mosaicRectButton->setPalette(pa);
//            m_mosaicRectButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));

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
//    rectLayout->addSpacing(BUTTON_SPACING);
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

//    pa = thickOneBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickOneBtn->setPalette(pa);

//    thickOneBtn->setToolTip(tr("fine"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
//    thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//    if (m_themeType == 1) {
//        thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickOneBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush small_normal.svg"));
//    }
    btnList.append(thickOneBtn);

//    connect(thickOneBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickOneBtn->setIcon(QIcon(":/image/newUI/press/brush small_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickOneBtn->setIcon(QIcon(":/image/newUI/dark/press/brush small_press.svg"));

//        }
//    });

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();

//    pa = thickTwoBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickTwoBtn->setPalette(pa);

//    thickTwoBtn->setToolTip(tr("medium"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
//    thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//    if (m_themeType == 1) {
//        thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush medium_normal.svg"));
//    }

//    connect(thickTwoBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/press/brush medium_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/press/brush medium_press.svg"));

//        }
//    });
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();

//    pa = thickThreeBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickThreeBtn->setPalette(pa);

//    thickThreeBtn->setToolTip(tr("thick"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
//    thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//    if (m_themeType == 1) {
//        thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush big_normal.svg"));
//    }

//    connect(thickThreeBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/press/brush big_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/press/brush big_press.svg"));

//        }
//    });
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

        DPalette pa;
        if (thickOneBtn->isChecked()) {
//            pa = thickOneBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickOneBtn->setPalette(pa);
//            thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));
//            if (m_themeType == 1) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush small_checked.svg"));
//            }

            thickOneBtn->update();

            ConfigSettings::instance()->setValue("oval", "linewidth_index", 0);
            if(m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if(m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        }

        else {
//            pa = thickOneBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickOneBtn->setPalette(pa);
//            thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//            if (m_themeType == 1) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush small_normal.svg"));
//            }

            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
//            pa = thickTwoBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickTwoBtn->setPalette(pa);
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));
//            if (m_themeType == 1) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush medium_checked.svg"));
//            }

            thickTwoBtn->update();

            ConfigSettings::instance()->setValue("oval", "linewidth_index", 1);
            if(m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if(m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        }

        else {
//            pa = thickTwoBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickTwoBtn->setPalette(pa);
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//            if (m_themeType == 1) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush medium_normal.svg"));
//            }

            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
//            pa = thickThreeBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickThreeBtn->setPalette(pa);
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));
//            if (m_themeType == 1) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush big_checked.svg"));
//            }

            thickThreeBtn->update();

            ConfigSettings::instance()->setValue("oval", "linewidth_index", 2);
            if(m_blurCircButton && m_blurCircButton->isChecked())
                m_blurCircButton->click();
            if(m_mosaicCircButton && m_mosaicCircButton->isChecked())
                m_mosaicCircButton->click();
        }

        else {
//            pa = thickThreeBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickThreeBtn->setPalette(pa);
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//            if (m_themeType == 1) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush big_normal.svg"));
//            }

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

//    pa = m_blurCircButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_blurCircButton->setPalette(pa);

    m_blurCircButton->setToolTip(tr("Blur"));
    m_blurCircButton->setObjectName("blurButton");
    m_blurCircButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_blurCircButton->setIconSize(TOOL_ICON_SIZE);
    m_blurCircButton->setIcon(QIcon::fromTheme("vague_normal"));
//    m_blurCircButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//    if (m_themeType == 1) {
//        m_blurCircButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        m_blurCircButton->setIcon(QIcon(":/image/newUI/dark/normal/vague_normal.svg"));
//    }

//    connect(m_blurCircButton, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            m_blurCircButton->setIcon(QIcon(":/image/newUI/press/vague_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            m_blurCircButton->setIcon(QIcon(":/image/newUI/dark/press/vague_press.svg"));

//        }
//    });
    btnList.append(m_blurCircButton);

    //马赛克按钮
    m_mosaicCircButton = new ToolButton();

//    pa = m_mosaicCircButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    m_mosaicCircButton->setPalette(pa);

    m_mosaicCircButton->setToolTip(tr("Mosaic"));
    m_mosaicCircButton->setObjectName("mosaicBtn");
    m_mosaicCircButton->setFixedSize(TOOL_BUTTON_SIZE);
    m_mosaicCircButton->setIconSize(TOOL_ICON_SIZE);
    m_mosaicCircButton->setIcon(QIcon::fromTheme("Mosaic_normal"));
//    m_mosaicCircButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//    if (m_themeType == 1) {
//        m_mosaicCircButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        m_mosaicCircButton->setIcon(QIcon(":/image/newUI/dark/normal/Mosaic_normal.svg"));
//    }

//    connect(m_mosaicCircButton, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            m_mosaicCircButton->setIcon(QIcon(":/image/newUI/press/Mosaic_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            m_mosaicCircButton->setIcon(QIcon(":/image/newUI/dark/press/Mosaic_press.svg"));

//        }
//    });

    btnList.append(m_mosaicCircButton);

    connect(m_blurCircButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_blurCircButton->isChecked();

        if (t_status == true)
        {
            DPalette pa;
//            pa = m_blurCircButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_blurCircButton->setPalette(pa);
//            m_blurCircButton->setIcon(QIcon(":/image/newUI/checked/vague_checked.svg"));
//            if (m_themeType == 1) {
//                m_blurCircButton->setIcon(QIcon(":/image/newUI/checked/vague_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_blurCircButton->setIcon(QIcon(":/image/newUI/dark/checked/vague_checked.svg"));
//            }

            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());

            m_mosaicCircButton->setChecked(false);
//            pa = m_mosaicCircButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_mosaicCircButton->setPalette(pa);
//            m_mosaicCircButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//            if (m_themeType == 1) {
//                m_mosaicCircButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_mosaicCircButton->setIcon(QIcon(":/image/newUI/dark/normal/Mosaic_normal.svg"));
//            }

            m_mosaicCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_mosaic", m_mosaicCircButton->isChecked());

        }

        else
        {
            DPalette pa;
            m_blurCircButton->setChecked(false);
//            pa = m_blurCircButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_blurCircButton->setPalette(pa);
//            m_blurCircButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//            if (m_themeType == 1) {
//                m_blurCircButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_blurCircButton->setIcon(QIcon(":/image/newUI/dark/normal/vague_normal.svg"));
//            }

            m_blurCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());
        }
    });

    connect(m_mosaicCircButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = m_mosaicCircButton->isChecked();

        if (t_status == true)
        {
            DPalette pa;
//            pa = m_mosaicCircButton->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            m_mosaicCircButton->setPalette(pa);
//            m_mosaicCircButton->setIcon(QIcon(":/image/newUI/checked/Mosaic_checked.svg"));
//            if (m_themeType == 1) {
//                m_mosaicCircButton->setIcon(QIcon(":/image/newUI/checked/Mosaic_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_mosaicCircButton->setIcon(QIcon(":/image/newUI/dark/checked/Mosaic_checked.svg"));
//            }


            ConfigSettings::instance()->setValue("oval", "is_mosaic", m_mosaicCircButton->isChecked());
            m_arrowFlag = true;
            m_blurCircButton->setChecked(false);
//            pa = m_blurCircButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_blurCircButton->setPalette(pa);
//            m_blurCircButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//            if (m_themeType == 1) {
//                m_blurCircButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_blurCircButton->setIcon(QIcon(":/image/newUI/dark/normal/vague_normal.svg"));
//            }

            m_blurCircButton->update();
            ConfigSettings::instance()->setValue("oval", "is_blur", m_blurCircButton->isChecked());
        }

        else
        {
            DPalette pa;
            m_mosaicCircButton->setChecked(false);
//            pa = m_mosaicCircButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_mosaicCircButton->setPalette(pa);
//            m_mosaicCircButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//            if (m_themeType == 1) {
//                m_mosaicCircButton->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                m_mosaicCircButton->setIcon(QIcon(":/image/newUI/dark/normal/Mosaic_normal.svg"));
//            }

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

//    pa = thickOneBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickOneBtn->setPalette(pa);

//    thickOneBtn->setToolTip(tr("fine"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
//    thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//    if (m_themeType == 1) {
//        thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickOneBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush small_normal.svg"));
//    }

//    connect(thickOneBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickOneBtn->setIcon(QIcon(":/image/newUI/press/brush small_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickOneBtn->setIcon(QIcon(":/image/newUI/dark/press/brush small_press.svg"));

//        }
//    });

    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();

//    pa = thickTwoBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickTwoBtn->setPalette(pa);

//    thickTwoBtn->setToolTip(tr("medium"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
//    thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//    if (m_themeType == 1) {
//        thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush medium_normal.svg"));
//    }

//    connect(thickTwoBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/press/brush medium_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/press/brush medium_press.svg"));

//        }
//    });

    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();

//    pa = thickThreeBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickThreeBtn->setPalette(pa);

//    thickThreeBtn->setToolTip(tr("thick"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
//    thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//    if (m_themeType == 1) {
//        thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush big_normal.svg"));
//    }

//    connect(thickThreeBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/press/brush big_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/press/brush big_press.svg"));

//        }
//    });

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

        DPalette pa;
        if (thickOneBtn->isChecked()) {
//            pa = thickOneBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickOneBtn->setPalette(pa);
//            thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));
//            if (m_themeType == 1) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush small_checked.svg"));
//            }

            thickOneBtn->update();

            ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", 0);
            ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", 0);
        }

        else {
//            pa = thickOneBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickOneBtn->setPalette(pa);
//            thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//            if (m_themeType == 1) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush small_normal.svg"));
//            }

            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
//            pa = thickTwoBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickTwoBtn->setPalette(pa);
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));
//            if (m_themeType == 1) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush medium_checked.svg"));
//            }

            thickTwoBtn->update();

            ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", 1);
            ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", 1);
        }

        else {
//            pa = thickTwoBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickTwoBtn->setPalette(pa);
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//            if (m_themeType == 1) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush medium_normal.svg"));
//            }


            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
//            pa = thickThreeBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickThreeBtn->setPalette(pa);
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));
//            if (m_themeType == 1) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush big_checked.svg"));
//            }

            thickThreeBtn->update();

            ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", 2);
            ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", 2);
        }

        else {
//            pa = thickThreeBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickThreeBtn->setPalette(pa);
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//            if (m_themeType == 1) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush big_normal.svg"));
//            }

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
//    pa = blurButton->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    blurButton->setPalette(pa);

//    blurButton->setToolTip(tr("line"));
    blurButton->setObjectName("blurButton");
    blurButton->setFixedSize(TOOL_BUTTON_SIZE);
    blurButton->setIconSize(TOOL_ICON_SIZE);
    blurButton->setIcon(QIcon::fromTheme("line-normal"));
//    blurButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
//    if (m_themeType == 1) {
//        blurButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        blurButton->setIcon(QIcon(":/image/newUI/dark/normal/line-normal_dark.svg"));
//    }

//    connect(blurButton, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            blurButton->setIcon(QIcon(":/image/newUI/press/line-press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            blurButton->setIcon(QIcon(":/image/newUI/dark/press/line-press.svg"));

//        }
//    });

    btnList.append(blurButton);

    //箭头按钮
    ToolButton *mosaicBtn = new ToolButton();

//    pa = mosaicBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    mosaicBtn->setPalette(pa);

//    mosaicBtn->setToolTip(tr("arrow"));
    mosaicBtn->setObjectName("mosaicBtn");
    mosaicBtn->setFixedSize(TOOL_BUTTON_SIZE);
    mosaicBtn->setIconSize(TOOL_ICON_SIZE);
//    mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
//    if (m_themeType == 1) {
//        mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        mosaicBtn->setIcon(QIcon(":/image/newUI/dark/normal/Arrow-normal_dark.svg"));
//    }

    mosaicBtn->setIcon(QIcon::fromTheme("Arrow-normal"));

//    connect(mosaicBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            mosaicBtn->setIcon(QIcon(":/image/newUI/press/Arrow-press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            mosaicBtn->setIcon(QIcon(":/image/newUI/dark/press/Arrow-press.svg"));

//        }
//    });

    btnList.append(mosaicBtn);

    connect(blurButton, &ToolButton::clicked, this, [ = ] {
        DPalette pa;
//        pa = blurButton->palette();
//        pa.setColor(DPalette::ButtonText, Qt::white);
//        pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//        pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//        blurButton->setPalette(pa);
//        blurButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));
//        if (m_themeType == 1)
//        {
//            blurButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));
//        }

//        else if (m_themeType == 2)
//        {
//            blurButton->setIcon(QIcon(":/image/newUI/dark/checked/line-checked.svg"));
//        }
        blurButton->setIcon(QIcon::fromTheme("line-normal"));

        blurButton->update();

        emit changeArrowAndLine(0);
        ConfigSettings::instance()->setValue("arrow", "is_straight", true);
        m_arrowFlag = false;

        if (blurButton->isChecked())
        {
            mosaicBtn->setChecked(false);
//            pa = mosaicBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            mosaicBtn->setPalette(pa);
//            mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
//            if (m_themeType == 1) {
//                mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                mosaicBtn->setIcon(QIcon(":/image/newUI/dark/normal/Arrow-normal_dark.svg"));
//            }
            mosaicBtn->setIcon(QIcon::fromTheme("Arrow-normal"));

            mosaicBtn->update();
        }
    });
    connect(mosaicBtn, &ToolButton::clicked, this, [ = ] {
        DPalette pa;
//        pa = mosaicBtn->palette();
//        pa.setColor(DPalette::ButtonText, Qt::white);
//        pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//        pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//        mosaicBtn->setPalette(pa);
//        mosaicBtn->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
//        if (m_themeType == 1)
//        {
//            mosaicBtn->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));
//        }

//        else if (m_themeType == 2)
//        {
//            mosaicBtn->setIcon(QIcon(":/image/newUI/dark/checked/Arrow-checked.svg"));
//        }

        mosaicBtn->update();

        emit changeArrowAndLine(1);
        ConfigSettings::instance()->setValue("arrow", "is_straight", false);
        m_arrowFlag = true;
        if (mosaicBtn->isChecked())
        {
            blurButton->setChecked(false);
//            pa = blurButton->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            blurButton->setPalette(pa);
//            blurButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
//            if (m_themeType == 1) {
//                blurButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                blurButton->setIcon(QIcon(":/image/newUI/dark/normal/line-normal_dark.svg"));
//            }

            blurButton->update();
        }
    });

//    mosaicBtn->click();
//    m_arrowFlag = false;
//    ConfigSettings::instance()->setValue("arrow", "is_straight", false);

    bool t_arrowStatus = ConfigSettings::instance()->value("arrow", "is_straight").toBool();

    if (t_arrowStatus) {
        blurButton->click();
        m_arrowFlag = true;
    }

    else {
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

//    pa = thickOneBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickOneBtn->setPalette(pa);

//    thickOneBtn->setToolTip(tr("fine"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon::fromTheme("brush small_normal"));
//    thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//    if (m_themeType == 1) {
//        thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickOneBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush small_normal.svg"));
//    }

//    connect(thickOneBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickOneBtn->setIcon(QIcon(":/image/newUI/press/brush small_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickOneBtn->setIcon(QIcon(":/image/newUI/dark/press/brush small_press.svg"));

//        }
//    });

    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();

//    pa = thickTwoBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickTwoBtn->setPalette(pa);

//    thickTwoBtn->setToolTip(tr("medium"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon::fromTheme("brush medium_normal"));
//    thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//    if (m_themeType == 1) {
//        thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush medium_normal.svg"));
//    }

//    connect(thickTwoBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/press/brush medium_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/press/brush medium_press.svg"));

//        }
//    });


    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();

//    pa = thickThreeBtn->palette();
//    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//    thickThreeBtn->setPalette(pa);

//    thickThreeBtn->setToolTip(tr("thick"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon::fromTheme("brush big_normal"));
//    thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//    if (m_themeType == 1) {
//        thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush big_normal.svg"));
//    }

//    connect(thickThreeBtn, &ToolButton::onPress, this, [ = ] {
//        if (m_themeType == 1)
//        {
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/press/brush big_press.svg"));

//        }

//        else if (m_themeType == 2)
//        {

//            thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/press/brush big_press.svg"));

//        }
//    });


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

        DPalette pa;
        if (thickOneBtn->isChecked()) {
//            pa = thickOneBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickOneBtn->setPalette(pa);
//            thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));
//            if (m_themeType == 1) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush small_checked.svg"));
//            }

            thickOneBtn->update();

            ConfigSettings::instance()->setValue("line", "linewidth_index", 0);
        }

        else {
//            pa = thickOneBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickOneBtn->setPalette(pa);
//            thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//            if (m_themeType == 1) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickOneBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush small_normal.svg"));
//            }

            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
//            pa = thickTwoBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickTwoBtn->setPalette(pa);
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));
//            if (m_themeType == 1) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush medium_checked.svg"));
//            }

            thickTwoBtn->update();

            ConfigSettings::instance()->setValue("line", "linewidth_index", 1);
        }

        else {
//            pa = thickTwoBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickTwoBtn->setPalette(pa);
//            thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//            if (m_themeType == 1) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickTwoBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush medium_normal.svg"));
//            }


            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
//            pa = thickThreeBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
//            thickThreeBtn->setPalette(pa);
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));
//            if (m_themeType == 1) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/checked/brush big_checked.svg"));
//            }

            thickThreeBtn->update();

            ConfigSettings::instance()->setValue("line", "linewidth_index", 2);
        }

        else {
//            pa = thickThreeBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            thickThreeBtn->setPalette(pa);
//            thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//            if (m_themeType == 1) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
//            }

//            else if (m_themeType == 2) {
//                thickThreeBtn->setIcon(QIcon(":/image/newUI/dark/normal/brush big_normal.svg"));
//            }

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
//    t_textFontSize->setValue(2);
    t_textFontSize->setRightIcon(QIcon(":/image/newUI/normal/Aa small_normal.svg"));
//    if (m_themeType == 1) {
//        t_textFontSize->setRightIcon(QIcon(":/image/newUI/normal/Aa big_normal.svg"));
//    }

//    else if (m_themeType == 2) {
//        t_textFontSize->setRightIcon(QIcon(":/image/newUI/dark/normal/Aa big_normal.svg"));
//    }
    t_textFontSize->setLeftIcon(QIcon(":/image/newUI/normal/Aa big_normal.svg"));
    t_textFontSize->setIconSize(TOOL_ICON_SIZE);
//    ConfigSettings::instance()->setValue("text", "fontsize", 12);

    int t_fontSize = ConfigSettings::instance()->value("text", "fontsize").toInt();

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

    t_blurAreaLayout->setContentsMargins(0, 0, 0, 0);
    t_blurAreaLayout->addWidget(t_textFontSize, Qt::AlignVCenter);
    t_blurArea->setLayout(t_blurAreaLayout);

    connect(t_textFontSize, &DSlider::valueChanged, this, [ = ] {
        int t_value = t_textFontSize->value();

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
            }

            else {
                setCurrentWidget(m_rectSubTool);
            }

        }

        if (shapeType == "oval") {
            if (m_circInitFlag == false) {
                initCircLabel();
                setCurrentWidget(m_circSubTool);
                m_circInitFlag = true;
            }

            else {
                setCurrentWidget(m_circSubTool);
            }
        }

        if (shapeType == "arrow") {
//            setCurrentWidget(m_lineSubTool);
            if (m_lineInitFlag == false) {
                initLineLabel();
                setCurrentWidget(m_lineSubTool);
                m_lineInitFlag = true;
            }

            else {
                setCurrentWidget(m_lineSubTool);
            }
        }

        if (shapeType == "line") {
//            setCurrentWidget(m_penSubTool);
            if (m_penInitFlag == false) {
                initPenLabel();
                setCurrentWidget(m_penSubTool);
                m_penInitFlag = true;
            }

            else {
                setCurrentWidget(m_penSubTool);
            }
        }

        if (shapeType == "text") {
//            setCurrentWidget(m_textSubTool);
            if (m_textInitFlag == false) {
                initTextLabel();
                setCurrentWidget(m_textSubTool);
                m_textInitFlag = true;
            }

            else {
                setCurrentWidget(m_textSubTool);
            }
        }
    }
}
void ShotToolWidget::colorChecked(QString colorType)
{
    qDebug() << colorType << m_currentType;
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
