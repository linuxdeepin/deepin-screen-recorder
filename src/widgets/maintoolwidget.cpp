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
#include "maintoolwidget.h"
#include <DSlider>
#include "toolbutton.h"
#include <QLineEdit>
#include <QButtonGroup>
#include <QHBoxLayout>
//#include <QSlider>
#include <QStyleFactory>
#include <QDebug>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 43;
const int TOOLBAR_WIDTH = 140;
const int BUTTON_SPACING = 4;
const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(65, 40);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(50, 40);
}


MainToolWidget::MainToolWidget(QWidget *parent) : QLabel(parent)
{
    initWidget();
}

MainToolWidget::~MainToolWidget()
{

}

void MainToolWidget::initWidget()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    setMouseTracking(true);
    setAcceptDrops(true);
    initMainLabel();
}

void MainToolWidget::initMainLabel()
{
    QList<ToolButton *> toolBtnList;
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    QPalette pa;

    ToolButton *recordBtn = new ToolButton();
    recordBtn->setCheckable(true);
    recordBtn->setText(tr("Record"));
    recordBtn->setObjectName("RecordBtn");
    recordBtn->setFixedSize(TOOL_BUTTON_SIZE);
    toolBtnList.append(recordBtn);

    ToolButton *shotBtn = new ToolButton();
    shotBtn->setText(tr("Shot"));
    shotBtn->setObjectName("ShotBtn");
    shotBtn->setFixedSize(TOOL_BUTTON_SIZE);
    toolBtnList.append(shotBtn);

    m_baseLayout = new QHBoxLayout();
    m_baseLayout->setMargin(0);
    m_baseLayout->setSpacing(0);
    m_baseLayout->addSpacing(4);

    for (int k = 0; k < toolBtnList.length(); k++) {
        m_baseLayout->addWidget(toolBtnList[k]);
        m_baseLayout->addSpacing(BUTTON_SPACING);

        buttonGroup->addButton(toolBtnList[k]);
    }
    setLayout(m_baseLayout);
    qDebug() << recordBtn->palette() << recordBtn->palette().dark().color().name(QColor::HexArgb);
    qDebug() << recordBtn->palette() << recordBtn->palette().light().color().name(QColor::HexArgb);
    qDebug() << recordBtn->palette() << recordBtn->palette().buttonText().color().name(QColor::HexArgb);

    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {

        QPalette pa;
        if (recordBtn->isChecked()) {
            pa = recordBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            recordBtn->setPalette(pa);

            m_isChecked = true;
            recordBtn->update();
            emit buttonChecked(m_isChecked, "record");
        }

        else {
            pa = recordBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            recordBtn->setPalette(pa);
        }

        if (shotBtn->isChecked()) {
            pa = shotBtn->palette();
            pa.setColor(QPalette::ButtonText, Qt::white);
            pa.setColor(QPalette::Dark, Qt::black);
            pa.setColor(QPalette::Light, Qt::black);
            shotBtn->setPalette(pa);

            m_isChecked = true;
            shotBtn->update();
            emit buttonChecked(m_isChecked, "shot");
        }

        else {
            pa = shotBtn->palette();
            pa.setColor(QPalette::ButtonText, QColor("#414d68"));
            pa.setColor(QPalette::Dark, QColor("#e3e3e3"));
            pa.setColor(QPalette::Light, QColor("#e6e6e6"));
            shotBtn->setPalette(pa);
        }
    });
    recordBtn->click();
}
