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
const int TOOLBAR_HEIGHT = 155;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 2;
const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize TOOL_ICON_SIZE = QSize(25, 25);
const QSize SPLITTER_SIZE = QSize(30, 1);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(35, 30);
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
    DPalette pa;

    m_redBtn = new ToolButton();
    m_redBtn->setCheckable(true);
//    redBtn->setText(tr("Red"));

    pa = m_redBtn->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
    m_redBtn->setPalette(pa);

    m_redBtn->setToolTip(tr("Red"));
    m_redBtn->setObjectName("redBtn");
    m_redBtn->setFixedSize(TOOL_BUTTON_SIZE);
    m_redBtn->setIconSize(TOOL_ICON_SIZE);
    m_redBtn->setIcon(QIcon(":/image/newUI/normal/red.svg"));
    toolBtnList.append(m_redBtn);

    m_yellowBtn = new ToolButton();
//    yellowBtn->setText(tr("yellow"));

    pa = m_yellowBtn->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
    m_yellowBtn->setPalette(pa);

    m_yellowBtn->setToolTip(tr("yellow"));
    m_yellowBtn->setObjectName("yellowBtn");
    m_yellowBtn->setFixedSize(TOOL_BUTTON_SIZE);
    m_yellowBtn->setIconSize(TOOL_ICON_SIZE);
    m_yellowBtn->setIcon(QIcon(":/image/newUI/normal/yellow.svg"));
    toolBtnList.append(m_yellowBtn);

    m_blueBtn = new ToolButton();
//    blueBtn->setText(tr("blue"));
    pa = m_blueBtn->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
    m_blueBtn->setPalette(pa);

    m_blueBtn->setToolTip(tr("blue"));
    m_blueBtn->setObjectName("blueBtn");
    m_blueBtn->setFixedSize(TOOL_BUTTON_SIZE);
    m_blueBtn->setIconSize(TOOL_ICON_SIZE);
    m_blueBtn->setIcon(QIcon(":/image/newUI/normal/blue.svg"));
    toolBtnList.append(m_blueBtn);

    m_greenBtn = new ToolButton();
//    greenBtn->setText(tr("green"));
    pa = m_greenBtn->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
    m_greenBtn->setPalette(pa);

    m_greenBtn->setToolTip(tr("green"));
    m_greenBtn->setObjectName("greenBtn");
    m_greenBtn->setFixedSize(TOOL_BUTTON_SIZE);
    m_greenBtn->setIconSize(TOOL_ICON_SIZE);
    m_greenBtn->setIcon(QIcon(":/image/newUI/normal/green.svg"));
    toolBtnList.append(m_greenBtn);

    m_baseLayout = new QVBoxLayout();
    m_baseLayout->setMargin(0);
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

        DPalette pa;
        if (m_redBtn->isChecked()) {
            pa = m_redBtn->palette();
            pa.setColor(DPalette::ButtonText, Qt::white);
            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
            m_redBtn->setPalette(pa);

            m_isChecked = true;
            m_redBtn->update();
            emit colorChecked("red");

            ConfigSettings::instance()->setValue(m_function, "color_index", 0);
            ConfigSettings::instance()->setValue("common", "color_index", 0);
        }

        else {
            pa = m_redBtn->palette();
            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
            m_redBtn->setPalette(pa);
        }

        if (m_yellowBtn->isChecked()) {
            pa = m_yellowBtn->palette();
            pa.setColor(DPalette::ButtonText, Qt::white);
            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
            m_yellowBtn->setPalette(pa);

            m_isChecked = true;
            m_yellowBtn->update();
            emit colorChecked("yellow");
            ConfigSettings::instance()->setValue(m_function, "color_index", 1);
            ConfigSettings::instance()->setValue("common", "color_index", 1);
        }

        else {
            pa = m_yellowBtn->palette();
            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
            m_yellowBtn->setPalette(pa);
        }

        if (m_blueBtn->isChecked()) {
            pa = m_blueBtn->palette();
            pa.setColor(DPalette::ButtonText, Qt::white);
            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
            m_blueBtn->setPalette(pa);

            m_isChecked = true;
            m_blueBtn->update();
            emit colorChecked("blue");

            ConfigSettings::instance()->setValue(m_function, "color_index", 2);
            ConfigSettings::instance()->setValue("common", "color_index", 2);
        }

        else {
            pa = m_blueBtn->palette();
            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
            m_blueBtn->setPalette(pa);
        }

        if (m_greenBtn->isChecked()) {
            pa = m_greenBtn->palette();
            pa.setColor(DPalette::ButtonText, Qt::white);
            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
            m_greenBtn->setPalette(pa);

            m_isChecked = true;
            m_greenBtn->update();
            emit colorChecked("green");

            ConfigSettings::instance()->setValue(m_function, "color_index", 3);
            ConfigSettings::instance()->setValue("common", "color_index", 3);
        }

        else {
            pa = m_greenBtn->palette();
            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
            m_greenBtn->setPalette(pa);
        }
    });
//    m_redBtn->click();
//    ConfigSettings::instance()->setValue("common", "color_index", 0);
//    ConfigSettings::instance()->setValue("rectangle", "color_index", 0);
//    ConfigSettings::instance()->setValue("oval", "color_index", 0);
//    ConfigSettings::instance()->setValue("arrow", "color_index", 0);
//    ConfigSettings::instance()->setValue("line", "color_index", 0);
//    ConfigSettings::instance()->setValue("text", "color_index", 0);
}

void ColorToolWidget::setFunction(const QString &func)
{
    m_function = func;
    int t_color = ConfigSettings::instance()->value(m_function, "color_index").toInt();
    ConfigSettings::instance()->setValue(m_function, "color_index", 0);
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
