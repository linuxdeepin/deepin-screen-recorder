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
#include "toolbutton.h"
#include "../utils/configsettings.h"

#include <DSlider>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QLine>
#include <DPalette>
#include <QDebug>
#include <DFrame>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 200;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 3;
const int SHOT_BUTTON_SPACING = 3;
const int COLOR_NUM = 16;
const QSize TOOL_ICON_SIZE = QSize(25, 25);
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize TOOL_SLIDER_SIZE = QSize(38, 180);
const QSize SPLITTER_SIZE = QSize(30, 1);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(35, 30);
}

ShotToolWidget::ShotToolWidget(QWidget *parent) : DStackedWidget(parent)
{
    initWidget();
}

ShotToolWidget::~ShotToolWidget()
{

}

void ShotToolWidget::initWidget()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    m_arrowFlag = false;

    initRectLabel();
    initCircLabel();
    initLineLabel();
    initPenLabel();
    initTextLabel();
    setCurrentWidget(m_rectSubTool);
}

void ShotToolWidget::initRectLabel()
{
    m_rectSubTool = new DLabel(this);
    QPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    QButtonGroup *t_funcBtnGroup = new QButtonGroup();
    t_funcBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();

    pa = thickOneBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickOneBtn->setPalette(pa);

    thickOneBtn->setToolTip(tr("fine"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();

    pa = thickTwoBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickTwoBtn->setPalette(pa);

    thickTwoBtn->setToolTip(tr("medium"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();

    pa = thickThreeBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickThreeBtn->setPalette(pa);

    thickThreeBtn->setToolTip(tr("thick"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

    connect(t_thicknessBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {

        QPalette pa;
        if (thickOneBtn->isChecked()) {
            pa = thickOneBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickOneBtn->setPalette(pa);
            thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));

            thickOneBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 1);
        }

        else {
            pa = thickOneBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickOneBtn->setPalette(pa);
            thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));

            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
            pa = thickTwoBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickTwoBtn->setPalette(pa);
            thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));

            thickTwoBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 2);
        }

        else {
            pa = thickTwoBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickTwoBtn->setPalette(pa);
            thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));

            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
            pa = thickThreeBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickThreeBtn->setPalette(pa);
            thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));

            thickThreeBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 3);
        }

        else {
            pa = thickThreeBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickThreeBtn->setPalette(pa);
            thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));

            thickThreeBtn->update();
        }
    });
    thickOneBtn->click();
    ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 1);


    rectLayout->addSpacing(BUTTON_SPACING);
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

    //模糊按钮
    ToolButton *blurButton = new ToolButton();

    pa = blurButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    blurButton->setPalette(pa);

    blurButton->setToolTip(tr("blur"));
    blurButton->setObjectName("blurButton");
    blurButton->setFixedSize(TOOL_BUTTON_SIZE);
    blurButton->setIconSize(TOOL_ICON_SIZE);
    blurButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
    btnList.append(blurButton);

    //马赛克按钮
    ToolButton *mosaicBtn = new ToolButton();

    pa = mosaicBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    mosaicBtn->setPalette(pa);

    mosaicBtn->setToolTip(tr("mosaic"));
    mosaicBtn->setObjectName("mosaicBtn");
    mosaicBtn->setFixedSize(TOOL_BUTTON_SIZE);
    mosaicBtn->setIconSize(TOOL_ICON_SIZE);
    mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
    btnList.append(mosaicBtn);

    connect(blurButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = blurButton->isChecked();

        if (t_status == true)
        {
            QPalette pa;
            pa = blurButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            blurButton->setPalette(pa);
            blurButton->setIcon(QIcon(":/image/newUI/checked/vague_checked.svg"));

            ConfigSettings::instance()->setValue("effect", "is_blur", blurButton->isChecked());

            mosaicBtn->setChecked(false);
            pa = mosaicBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            mosaicBtn->setPalette(pa);
            mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));

            mosaicBtn->update();
            ConfigSettings::instance()->setValue("effect", "is_mosaic", mosaicBtn->isChecked());

        }

        else
        {
            QPalette pa;
            blurButton->setChecked(false);
            pa = blurButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            blurButton->setPalette(pa);
            blurButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));

            blurButton->update();
            ConfigSettings::instance()->setValue("effect", "is_blur", blurButton->isChecked());
        }
    });

    connect(mosaicBtn, &ToolButton::clicked, this, [ = ] {
        bool t_status = mosaicBtn->isChecked();

        if (t_status == true)
        {
            QPalette pa;
            pa = mosaicBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            mosaicBtn->setPalette(pa);
            mosaicBtn->setIcon(QIcon(":/image/newUI/checked/Mosaic_checked.svg"));

            ConfigSettings::instance()->setValue("effect", "is_mosaic", mosaicBtn->isChecked());
            m_arrowFlag = true;
            blurButton->setChecked(false);
            pa = blurButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            blurButton->setPalette(pa);
            blurButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));

            blurButton->update();
            ConfigSettings::instance()->setValue("effect", "is_blur", blurButton->isChecked());
        }

        else
        {
            QPalette pa;
            mosaicBtn->setChecked(false);
            pa = mosaicBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            mosaicBtn->setPalette(pa);
            mosaicBtn->setIconSize(QSize(30, 30));
            mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));

            mosaicBtn->update();
            ConfigSettings::instance()->setValue("effect", "is_mosaic", mosaicBtn->isChecked());
        }

    });


    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
    }

    m_rectSubTool->setLayout(rectLayout);
    addWidget(m_rectSubTool);
}

void ShotToolWidget::initCircLabel()
{
    m_circSubTool = new DLabel(this);
    QPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    QButtonGroup *t_funcBtnGroup = new QButtonGroup();
    t_funcBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();

    pa = thickOneBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickOneBtn->setPalette(pa);

    thickOneBtn->setToolTip(tr("fine"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();

    pa = thickTwoBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickTwoBtn->setPalette(pa);

    thickTwoBtn->setToolTip(tr("medium"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();

    pa = thickThreeBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickThreeBtn->setPalette(pa);

    thickThreeBtn->setToolTip(tr("thick"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

    connect(t_thicknessBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {

        QPalette pa;
        if (thickOneBtn->isChecked()) {
            pa = thickOneBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickOneBtn->setPalette(pa);
            thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));

            thickOneBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 1);
        }

        else {
            pa = thickOneBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickOneBtn->setPalette(pa);
            thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));

            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
            pa = thickTwoBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickTwoBtn->setPalette(pa);
            thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));

            thickTwoBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 2);
        }

        else {
            pa = thickTwoBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickTwoBtn->setPalette(pa);
            thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));

            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
            pa = thickThreeBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickThreeBtn->setPalette(pa);
            thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));

            thickThreeBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 3);
        }

        else {
            pa = thickThreeBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickThreeBtn->setPalette(pa);
            thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));

            thickThreeBtn->update();
        }
    });
    thickOneBtn->click();
    ConfigSettings::instance()->setValue("oval", "linewidth_index", 1);

    rectLayout->addSpacing(BUTTON_SPACING);
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

    //模糊按钮
    ToolButton *blurButton = new ToolButton();

    pa = blurButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    blurButton->setPalette(pa);

    blurButton->setToolTip(tr("blur"));
    blurButton->setObjectName("blurButton");
    blurButton->setFixedSize(TOOL_BUTTON_SIZE);
    blurButton->setIconSize(TOOL_ICON_SIZE);
    blurButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));
    btnList.append(blurButton);

    //马赛克按钮
    ToolButton *mosaicBtn = new ToolButton();

    pa = mosaicBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    mosaicBtn->setPalette(pa);

    mosaicBtn->setToolTip(tr("mosaic"));
    mosaicBtn->setObjectName("mosaicBtn");
    mosaicBtn->setFixedSize(TOOL_BUTTON_SIZE);
    mosaicBtn->setIconSize(TOOL_ICON_SIZE);
    mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));
    btnList.append(mosaicBtn);

    connect(blurButton, &ToolButton::clicked, this, [ = ] {
        bool t_status = blurButton->isChecked();

        if (t_status == true)
        {
            QPalette pa;
            pa = blurButton->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            blurButton->setPalette(pa);
            blurButton->setIcon(QIcon(":/image/newUI/checked/vague_checked.svg"));

            ConfigSettings::instance()->setValue("effect", "is_blur", blurButton->isChecked());

            mosaicBtn->setChecked(false);
            pa = mosaicBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            mosaicBtn->setPalette(pa);
            mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));

            mosaicBtn->update();
            ConfigSettings::instance()->setValue("effect", "is_mosaic", mosaicBtn->isChecked());

        }

        else
        {
            QPalette pa;
            blurButton->setChecked(false);
            pa = blurButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            blurButton->setPalette(pa);
            blurButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));

            blurButton->update();
            ConfigSettings::instance()->setValue("effect", "is_blur", blurButton->isChecked());
        }
    });

    connect(mosaicBtn, &ToolButton::clicked, this, [ = ] {
        bool t_status = mosaicBtn->isChecked();

        if (t_status == true)
        {
            QPalette pa;
            pa = mosaicBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            mosaicBtn->setPalette(pa);
            mosaicBtn->setIcon(QIcon(":/image/newUI/checked/Mosaic_checked.svg"));

            ConfigSettings::instance()->setValue("effect", "is_mosaic", mosaicBtn->isChecked());
            m_arrowFlag = true;
            blurButton->setChecked(false);
            pa = blurButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            blurButton->setPalette(pa);
            blurButton->setIcon(QIcon(":/image/newUI/normal/vague_normal.svg"));

            blurButton->update();
            ConfigSettings::instance()->setValue("effect", "is_blur", blurButton->isChecked());
        }

        else
        {
            QPalette pa;
            mosaicBtn->setChecked(false);
            pa = mosaicBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            mosaicBtn->setPalette(pa);
            mosaicBtn->setIconSize(QSize(30, 30));
            mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Mosaic_normal.svg"));

            mosaicBtn->update();
            ConfigSettings::instance()->setValue("effect", "is_mosaic", mosaicBtn->isChecked());
        }

    });

    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
    }

    m_circSubTool->setLayout(rectLayout);
    addWidget(m_circSubTool);
}

void ShotToolWidget::initLineLabel()
{
    m_lineSubTool = new DLabel(this);
    QPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    //选择功能按钮组
    QButtonGroup *t_funcBtnGroup = new QButtonGroup();
    t_funcBtnGroup->setExclusive(false);
    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();

    pa = thickOneBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickOneBtn->setPalette(pa);

    thickOneBtn->setToolTip(tr("fine"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();

    pa = thickTwoBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickTwoBtn->setPalette(pa);

    thickTwoBtn->setToolTip(tr("medium"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();

    pa = thickThreeBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickThreeBtn->setPalette(pa);

    thickThreeBtn->setToolTip(tr("thick"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

    connect(t_thicknessBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {

        QPalette pa;
        if (thickOneBtn->isChecked()) {
            pa = thickOneBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickOneBtn->setPalette(pa);
            thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));

            thickOneBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 1);
        }

        else {
            pa = thickOneBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickOneBtn->setPalette(pa);
            thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));

            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
            pa = thickTwoBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickTwoBtn->setPalette(pa);
            thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));

            thickTwoBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 2);
        }

        else {
            pa = thickTwoBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickTwoBtn->setPalette(pa);
            thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));

            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
            pa = thickThreeBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickThreeBtn->setPalette(pa);
            thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));

            thickThreeBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 3);
        }

        else {
            pa = thickThreeBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickThreeBtn->setPalette(pa);
            thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));

            thickThreeBtn->update();
        }
    });
    thickOneBtn->click();

    ConfigSettings::instance()->setValue("arrow", "arrow_linewidth_index", 1);
    ConfigSettings::instance()->setValue("arrow", "straightline_linewidth_index", 1);

    rectLayout->addSpacing(BUTTON_SPACING);
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
    pa = blurButton->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    blurButton->setPalette(pa);

    blurButton->setToolTip(tr("line"));
    blurButton->setObjectName("blurButton");
    blurButton->setFixedSize(TOOL_BUTTON_SIZE);
    blurButton->setIconSize(TOOL_ICON_SIZE);
    blurButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));
    btnList.append(blurButton);

    //箭头按钮
    ToolButton *mosaicBtn = new ToolButton();

    pa = mosaicBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    mosaicBtn->setPalette(pa);

    mosaicBtn->setToolTip(tr("arrow"));
    mosaicBtn->setObjectName("mosaicBtn");
    mosaicBtn->setFixedSize(TOOL_BUTTON_SIZE);
    mosaicBtn->setIconSize(TOOL_ICON_SIZE);
    mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));
    btnList.append(mosaicBtn);

    connect(blurButton, &ToolButton::clicked, this, [ = ] {
        QPalette pa;
        pa = blurButton->palette();
        pa.setColor(QPalette::ButtonText, Qt::white);
        pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
        pa.setColor(QPalette::Light, QColor("#1C1C1C"));
        blurButton->setPalette(pa);
        blurButton->setIcon(QIcon(":/image/newUI/checked/line-checked.svg"));

        blurButton->update();

        emit changeArrowAndLine(0);
        ConfigSettings::instance()->setValue("arrow", "is_straight", true);
        m_arrowFlag = false;

        if (blurButton->isChecked())
        {
            mosaicBtn->setChecked(false);
            pa = mosaicBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            mosaicBtn->setPalette(pa);
            mosaicBtn->setIcon(QIcon(":/image/newUI/normal/Arrow-normal.svg"));

            mosaicBtn->update();
        }
    });
    connect(mosaicBtn, &ToolButton::clicked, this, [ = ] {
        QPalette pa;
        pa = mosaicBtn->palette();
        pa.setColor(QPalette::ButtonText, Qt::white);
        pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
        pa.setColor(QPalette::Light, QColor("#1C1C1C"));
        mosaicBtn->setPalette(pa);
        mosaicBtn->setIcon(QIcon(":/image/newUI/checked/Arrow-checked.svg"));

        mosaicBtn->update();

        emit changeArrowAndLine(1);
        ConfigSettings::instance()->setValue("arrow", "is_straight", false);
        m_arrowFlag = true;
        if (mosaicBtn->isChecked())
        {
            blurButton->setChecked(false);
            pa = blurButton->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            blurButton->setPalette(pa);
            blurButton->setIcon(QIcon(":/image/newUI/normal/line-normal.svg"));

            blurButton->update();
        }
    });

    blurButton->click();
    m_arrowFlag = false;
    ConfigSettings::instance()->setValue("arrow", "is_straight", true);

    for (int j = 0; j < btnList.length(); j++) {
        rectLayout->addWidget(btnList[j]);
        t_funcBtnGroup->addButton(btnList[j]);
    }

    m_lineSubTool->setLayout(rectLayout);
    addWidget(m_lineSubTool);
}

void ShotToolWidget::initPenLabel()
{
    m_penSubTool = new DLabel(this);
    QPalette pa;

    //选择粗细按钮组
    QButtonGroup *t_thicknessBtnGroup = new QButtonGroup();
    t_thicknessBtnGroup->setExclusive(true);

    QList<ToolButton *> btnList;

    //粗细程度１级按钮
    ToolButton *thickOneBtn = new ToolButton();

    pa = thickOneBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickOneBtn->setPalette(pa);

    thickOneBtn->setToolTip(tr("fine"));
    thickOneBtn->setObjectName("thickOneBtn");
    thickOneBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickOneBtn->setIconSize(TOOL_ICON_SIZE);
    thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));
    btnList.append(thickOneBtn);

    //粗细程度２级按钮
    ToolButton *thickTwoBtn = new ToolButton();

    pa = thickTwoBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickTwoBtn->setPalette(pa);

    thickTwoBtn->setToolTip(tr("medium"));
    thickTwoBtn->setObjectName("thickTwoBtn");
    thickTwoBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickTwoBtn->setIconSize(TOOL_ICON_SIZE);
    thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));
    btnList.append(thickTwoBtn);

    //粗细程度3级按钮
    ToolButton *thickThreeBtn = new ToolButton();

    pa = thickThreeBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    thickThreeBtn->setPalette(pa);

    thickThreeBtn->setToolTip(tr("thick"));
    thickThreeBtn->setObjectName("thickThreeBtn");
    thickThreeBtn->setFixedSize(TOOL_BUTTON_SIZE);
    thickThreeBtn->setIconSize(TOOL_ICON_SIZE);
    thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));
    btnList.append(thickThreeBtn);

    QVBoxLayout *rectLayout = new QVBoxLayout();
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);
    for (int i = 0; i < btnList.length(); i++) {
        rectLayout->addWidget(btnList[i]);
        t_thicknessBtnGroup->addButton(btnList[i]);
    }

    connect(t_thicknessBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {

        QPalette pa;
        if (thickOneBtn->isChecked()) {
            pa = thickOneBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickOneBtn->setPalette(pa);
            thickOneBtn->setIcon(QIcon(":/image/newUI/checked/brush small_checked.svg"));

            thickOneBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 1);
        }

        else {
            pa = thickOneBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickOneBtn->setPalette(pa);
            thickOneBtn->setIcon(QIcon(":/image/newUI/normal/brush small_normal.svg"));

            thickOneBtn->update();
        }

        if (thickTwoBtn->isChecked()) {
            pa = thickTwoBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickTwoBtn->setPalette(pa);
            thickTwoBtn->setIcon(QIcon(":/image/newUI/checked/brush medium_checked.svg"));

            thickTwoBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 2);
        }

        else {
            pa = thickTwoBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickTwoBtn->setPalette(pa);
            thickTwoBtn->setIcon(QIcon(":/image/newUI/normal/brush medium_normal.svg"));

            thickTwoBtn->update();
        }

        if (thickThreeBtn->isChecked()) {
            pa = thickThreeBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            thickThreeBtn->setPalette(pa);
            thickThreeBtn->setIcon(QIcon(":/image/newUI/checked/brush big_checked.svg"));

            thickThreeBtn->update();

            ConfigSettings::instance()->setValue("rectangle", "linewidth_index", 3);
        }

        else {
            pa = thickThreeBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            thickThreeBtn->setPalette(pa);
            thickThreeBtn->setIcon(QIcon(":/image/newUI/normal/brush big_normal.svg"));

            thickThreeBtn->update();
        }
    });
    thickOneBtn->click();
    ConfigSettings::instance()->setValue("line", "linewidth_index", 1);


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

    DSlider *t_textFontSize = new DSlider();
    t_textFontSize->setOrientation(Qt::Vertical);
//    t_textFontSize->setTickPosition(QSlider::NoTicks);
    t_textFontSize->setFixedSize(TOOL_SLIDER_SIZE);
    t_textFontSize->setMinimum(0);
    t_textFontSize->setMaximum(11);
    t_textFontSize->setTickInterval(1);
    t_textFontSize->setValue(4);
    ConfigSettings::instance()->setValue("text", "fontsize", 18);

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
    rectLayout->setMargin(0);
    rectLayout->setSpacing(0);
    rectLayout->addSpacing(5);

    rectLayout->addWidget(t_textFontSize);

    m_textSubTool->setLayout(rectLayout);
    addWidget(m_textSubTool);
}

void ShotToolWidget::switchContent(QString shapeType)
{
    if (!shapeType.isEmpty()) {
        if (shapeType == "rectangle") {
            setCurrentWidget(m_rectSubTool);
        }

        if (shapeType == "oval") {
            setCurrentWidget(m_circSubTool);
        }

        if (shapeType == "arrow") {
            setCurrentWidget(m_lineSubTool);
        }

        if (shapeType == "line") {
            setCurrentWidget(m_penSubTool);
        }

        if (shapeType == "text") {
            setCurrentWidget(m_textSubTool);
        }
    }
}
