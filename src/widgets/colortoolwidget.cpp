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
#include "colortoolwidget.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"

#include <DSlider>
#include <DLineEdit>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>
#include <DPalette>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 175;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 3;
//const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize TOOL_ICON_SIZE = QSize(30, 30);
//const QSize SPLITTER_SIZE = QSize(30, 1);
//const QSize MIN_TOOL_BUTTON_SIZE = QSize(35, 30);
}

ColorToolWidget::ColorToolWidget(DWidget *parent) : DLabel(parent)
{
    initWidget();
    m_function = "rectangle";
}

ColorToolWidget::~ColorToolWidget()
{

}

void ColorToolWidget::initWidget()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    setMouseTracking(true);
    setAcceptDrops(true);
    initColorLabel();
}

void ColorToolWidget::initColorLabel()
{
    QList<ToolButton *> toolBtnList;
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    //DPalette pa;

    m_redBtn = new ToolButton();
    m_redBtn->setCheckable(true);
    //m_redBtn->setObjectName("redBtn");
    Utils::setAccessibility(m_redBtn, AC_COLORTOOLWIDGET_RED_BUT);
    m_redBtn->setFixedSize(TOOL_BUTTON_SIZE);
    m_redBtn->setIconSize(TOOL_ICON_SIZE);
    m_redBtn->setIcon(QIcon(":/image/newUI/normal/red.svg"));
//    m_redBtn->setToolTip(tr("Select Red"));
    toolBtnList.append(m_redBtn);

    m_yellowBtn = new ToolButton();
    //m_yellowBtn->setObjectName("yellowBtn");
    Utils::setAccessibility(m_yellowBtn, AC_COLORTOOLWIDGET_YELLOW_BUT);
    m_yellowBtn->setFixedSize(TOOL_BUTTON_SIZE);
    m_yellowBtn->setIconSize(TOOL_ICON_SIZE);
    m_yellowBtn->setIcon(QIcon(":/image/newUI/normal/yellow.svg"));
//    m_yellowBtn->setToolTip(tr("Select Yellow"));
    toolBtnList.append(m_yellowBtn);

    m_blueBtn = new ToolButton();
    //m_blueBtn->setObjectName("blueBtn");
    Utils::setAccessibility(m_blueBtn, AC_COLORTOOLWIDGET_BLUE_BUT);
    m_blueBtn->setFixedSize(TOOL_BUTTON_SIZE);
    m_blueBtn->setIconSize(TOOL_ICON_SIZE);
    m_blueBtn->setIcon(QIcon(":/image/newUI/normal/blue.svg"));
//    m_blueBtn->setToolTip(tr("Select Blue"));
    toolBtnList.append(m_blueBtn);

    m_greenBtn = new ToolButton();
    //m_greenBtn->setObjectName("greenBtn");
    Utils::setAccessibility(m_greenBtn, AC_COLORTOOLWIDGET_GREEN_BUT);
    m_greenBtn->setFixedSize(TOOL_BUTTON_SIZE);
    m_greenBtn->setIconSize(TOOL_ICON_SIZE);
    m_greenBtn->setIcon(QIcon(":/image/newUI/normal/green.svg"));
//    m_greenBtn->setToolTip(tr("Select Green"));
    toolBtnList.append(m_greenBtn);

    m_baseLayout = new QVBoxLayout();
    m_baseLayout->setContentsMargins(1, 0, 0, 0);
//    m_baseLayout->setMargin(0);
    m_baseLayout->setSpacing(0);

    for (int k = 0; k < toolBtnList.length(); k++) {
        m_baseLayout->addWidget(toolBtnList[k]);
        m_baseLayout->addSpacing(BUTTON_SPACING);

        buttonGroup->addButton(toolBtnList[k]);
    }
    m_baseLayout->addSpacing(5);
    setLayout(m_baseLayout);

    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {
        Q_UNUSED(status);
        //DPalette pa;
        if (m_redBtn->isChecked()) {
            m_isChecked = true;
            m_redBtn->update();
            emit colorChecked("red");

            ConfigSettings::instance()->setValue(m_function, "color_index", 0);
            ConfigSettings::instance()->setValue("common", "color_index", 0);
        }

        if (m_yellowBtn->isChecked()) {
            m_isChecked = true;
            m_yellowBtn->update();
            emit colorChecked("yellow");
            ConfigSettings::instance()->setValue(m_function, "color_index", 1);
            ConfigSettings::instance()->setValue("common", "color_index", 1);
        }
        if (m_blueBtn->isChecked()) {
            m_isChecked = true;
            m_blueBtn->update();
            emit colorChecked("blue");

            ConfigSettings::instance()->setValue(m_function, "color_index", 2);
            ConfigSettings::instance()->setValue("common", "color_index", 2);
        }

        if (m_greenBtn->isChecked()) {
            m_isChecked = true;
            m_greenBtn->update();
            emit colorChecked("green");

            ConfigSettings::instance()->setValue(m_function, "color_index", 3);
            ConfigSettings::instance()->setValue("common", "color_index", 3);
        }
    });
}

void ColorToolWidget::setFunction(const QString &func)
{
    m_function = func;
    int t_color = 0;
    t_color = ConfigSettings::instance()->value(m_function, "color_index").toInt();
//    ConfigSettings::instance()->setValue(m_function, "color_index", 0);

    switch (t_color) {
    case 0:
        m_redBtn->click();
        break;
    case 1:
        m_yellowBtn->click();
        break;
    case 2:
        m_blueBtn->click();
        break;
    case 3:
        m_greenBtn->click();
        break;
    default:
        break;
    }
}
