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
#include "toolbutton.h"
#include "../utils/configsettings.h"

#include <DSlider>
#include <QLineEdit>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QDebug>
#include <DPalette>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 160;
const int TOOLBAR_WIDTH = 40;
const int BUTTON_SPACING = 4;
const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(38, 38);
const QSize TOOL_ICON_SIZE = QSize(25, 25);
const QSize SPLITTER_SIZE = QSize(30, 1);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(35, 30);
}

ColorToolWidget::ColorToolWidget(QWidget *parent) : DLabel(parent)
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
    QPalette pa;

    ToolButton *redBtn = new ToolButton();
    redBtn->setCheckable(true);
//    redBtn->setText(tr("Red"));

    pa = redBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    redBtn->setPalette(pa);

    redBtn->setToolTip(tr("Red"));
    redBtn->setObjectName("redBtn");
    redBtn->setFixedSize(TOOL_BUTTON_SIZE);
    redBtn->setIconSize(TOOL_ICON_SIZE);
    redBtn->setIcon(QIcon(":/image/newUI/normal/red.svg"));
    toolBtnList.append(redBtn);

    ToolButton *yellowBtn = new ToolButton();
//    yellowBtn->setText(tr("yellow"));

    pa = yellowBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    yellowBtn->setPalette(pa);

    yellowBtn->setToolTip(tr("yellow"));
    yellowBtn->setObjectName("yellowBtn");
    yellowBtn->setFixedSize(TOOL_BUTTON_SIZE);
    yellowBtn->setIconSize(TOOL_ICON_SIZE);
    yellowBtn->setIcon(QIcon(":/image/newUI/normal/yellow.svg"));
    toolBtnList.append(yellowBtn);

    ToolButton *blueBtn = new ToolButton();
//    blueBtn->setText(tr("blue"));
    pa = blueBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    blueBtn->setPalette(pa);

    blueBtn->setToolTip(tr("blue"));
    blueBtn->setObjectName("blueBtn");
    blueBtn->setFixedSize(TOOL_BUTTON_SIZE);
    blueBtn->setIconSize(TOOL_ICON_SIZE);
    blueBtn->setIcon(QIcon(":/image/newUI/normal/blue.svg"));
    toolBtnList.append(blueBtn);

    ToolButton *greenBtn = new ToolButton();
//    greenBtn->setText(tr("green"));
    pa = greenBtn->palette();
    pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
    pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
    greenBtn->setPalette(pa);

    greenBtn->setToolTip(tr("green"));
    greenBtn->setObjectName("greenBtn");
    greenBtn->setFixedSize(TOOL_BUTTON_SIZE);
    greenBtn->setIconSize(TOOL_ICON_SIZE);
    greenBtn->setIcon(QIcon(":/image/newUI/normal/green.svg"));
    toolBtnList.append(greenBtn);

    m_baseLayout = new QVBoxLayout();
    m_baseLayout->setMargin(0);
    m_baseLayout->setSpacing(0);

    for (int k = 0; k < toolBtnList.length(); k++) {
        m_baseLayout->addWidget(toolBtnList[k]);
        m_baseLayout->addSpacing(BUTTON_SPACING);

        buttonGroup->addButton(toolBtnList[k]);
    }

    setLayout(m_baseLayout);

    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {

        QPalette pa;
        if (redBtn->isChecked()) {
            pa = redBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            redBtn->setPalette(pa);

            m_isChecked = true;
            redBtn->update();
            emit colorChecked("red");

            ConfigSettings::instance()->setValue("common", "color_index", 0);
            ConfigSettings::instance()->setValue("rectangle", "color_index", 0);
            ConfigSettings::instance()->setValue("oval", "color_index", 0);
            ConfigSettings::instance()->setValue("arrow", "color_index", 0);
            ConfigSettings::instance()->setValue("line", "color_index", 0);
            ConfigSettings::instance()->setValue("text", "color_index", 0);
        }

        else {
            pa = redBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            redBtn->setPalette(pa);
        }

        if (yellowBtn->isChecked()) {
            pa = yellowBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            yellowBtn->setPalette(pa);

            m_isChecked = true;
            yellowBtn->update();
            emit colorChecked("yellow");
            ConfigSettings::instance()->setValue("common", "color_index", 1);
            ConfigSettings::instance()->setValue("rectangle", "color_index", 1);
            ConfigSettings::instance()->setValue("oval", "color_index", 1);
            ConfigSettings::instance()->setValue("arrow", "color_index", 1);
            ConfigSettings::instance()->setValue("line", "color_index", 1);
            ConfigSettings::instance()->setValue("text", "color_index", 1);
        }

        else {
            pa = yellowBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            yellowBtn->setPalette(pa);
        }

        if (blueBtn->isChecked()) {
            pa = blueBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            blueBtn->setPalette(pa);

            m_isChecked = true;
            blueBtn->update();
            emit colorChecked("blue");

            ConfigSettings::instance()->setValue("common", "color_index", 2);
            ConfigSettings::instance()->setValue("rectangle", "color_index", 2);
            ConfigSettings::instance()->setValue("oval", "color_index", 2);
            ConfigSettings::instance()->setValue("arrow", "color_index", 2);
            ConfigSettings::instance()->setValue("line", "color_index", 2);
            ConfigSettings::instance()->setValue("text", "color_index", 2);
        }

        else {
            pa = blueBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            blueBtn->setPalette(pa);
        }

        if (greenBtn->isChecked()) {
            pa = greenBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, QColor("#1C1C1C"));
            pa.setColor(QPalette::Light, QColor("#1C1C1C"));
            greenBtn->setPalette(pa);

            m_isChecked = true;
            greenBtn->update();
            emit colorChecked("green");

            ConfigSettings::instance()->setValue("common", "color_index", 3);
            ConfigSettings::instance()->setValue("rectangle", "color_index", 3);
            ConfigSettings::instance()->setValue("oval", "color_index", 3);
            ConfigSettings::instance()->setValue("arrow", "color_index", 3);
            ConfigSettings::instance()->setValue("line", "color_index", 3);
            ConfigSettings::instance()->setValue("text", "color_index", 3);
        }

        else {
            pa = greenBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor(28, 28, 28, 255));
            pa.setColor(QPalette::Dark, QColor(227, 227, 227, 150));
            pa.setColor(QPalette::Light, QColor(230, 230, 230, 150));
            greenBtn->setPalette(pa);
        }
    });
    redBtn->click();
    ConfigSettings::instance()->setValue("common", "color_index", 0);
    ConfigSettings::instance()->setValue("rectangle", "color_index", 0);
    ConfigSettings::instance()->setValue("oval", "color_index", 0);
    ConfigSettings::instance()->setValue("arrow", "color_index", 0);
    ConfigSettings::instance()->setValue("line", "color_index", 0);
    ConfigSettings::instance()->setValue("text", "color_index", 0);
}

void ColorToolWidget::setFunction(const QString &func)
{
    m_function = func;
}
