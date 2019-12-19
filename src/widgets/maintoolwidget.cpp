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
#include <DLineEdit>
#include <QButtonGroup>
#include <QHBoxLayout>
//#include <QSlider>
#include <QStyleFactory>
#include <QDebug>
#include <DFontSizeManager>
#include "../utils/configsettings.h"
#include "tooltips.h"

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 43;
const int TOOLBAR_WIDTH = 155;
const int BUTTON_SPACING = 3;
const int COLOR_NUM = 16;
const QSize TOOL_BUTTON_SIZE = QSize(70, 40);
const QSize MIN_TOOL_BUTTON_SIZE = QSize(50, 40);
}


MainToolWidget::MainToolWidget(DWidget *parent) : DStackedWidget(parent)
{
    initWidget();
}

MainToolWidget::~MainToolWidget()
{

}

void MainToolWidget::initWidget()
{
//    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
//    setFixedHeight(43);
//    setMinimumSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
//    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hintFilter = new HintFilter;
    m_themeType = 0;
    m_themeType = ConfigSettings::instance()->value("common", "themeType").toInt();
//    setMouseTracking(true);
//    setAcceptDrops(true);
    initMainLabel();
    setCurrentWidget(m_mainTool);
}

void MainToolWidget::initMainLabel()
{
    m_mainTool = new DLabel(this);
    QList<ToolButton *> toolBtnList;
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    DPalette pa;

//    QString record_button_style = "DPushButton:press{QIcon(:/image/newUI/press/screencap-press.svg)}";

    m_recordBtn = new ToolButton();
    DFontSizeManager::instance()->bind(m_recordBtn, DFontSizeManager::T8);

    m_recordBtn->setCheckable(true);
    m_recordBtn->setText(tr("Record"));
    m_recordBtn->setObjectName("RecordBtn");
//    m_recordBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//    m_recordBtn->setMinimumSize(TOOL_BUTTON_SIZE);
    m_recordBtn->setMinimumHeight(40);
    m_recordBtn->setIconSize(QSize(20, 20));
    if (m_themeType == 1) {
        m_recordBtn->setIcon(QIcon(":/image/newUI/normal/screencap-normal.svg"));
    }

    else if (m_themeType == 2) {
        m_recordBtn->setIcon(QIcon(":/image/newUI/dark/normal/screencap-normal_dark.svg"));
    }

//    m_recordBtn->setToolTip(tr("Switch to record mode"));
    installTipHint(m_recordBtn, tr("Switch to record mode"));
//    recordBtn->setStyleSheet(record_button_style);
    toolBtnList.append(m_recordBtn);

    connect(m_recordBtn, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_recordBtn->setIcon(QIcon(":/image/newUI/press/screencap-press.svg"));
        }

        else if (m_themeType == 2)
        {
            m_recordBtn->setIcon(QIcon(":/image/newUI/dark/press/screencap-press.svg"));
        }
    });

//    QString shot_button_style = "DPushButton:press{QIcon(:/image/newUI/press/shot-press.svg)}";

    m_shotBtn = new ToolButton();
    DFontSizeManager::instance()->bind(m_shotBtn, DFontSizeManager::T8);

    m_shotBtn->setText(tr("Shot"));
    m_shotBtn->setObjectName("ShotBtn");
//    m_shotBtn->setMinimumSize(TOOL_BUTTON_SIZE);
    m_shotBtn->setMinimumHeight(40);

    m_shotBtn->setIconSize(QSize(20, 20));

    if (m_themeType == 1) {
        m_shotBtn->setIcon(QIcon(":/image/newUI/normal/screenshot-normal.svg"));
    }

    else if (m_themeType == 2) {
        m_shotBtn->setIcon(QIcon(":/image/newUI/dark/normal/screenshot-normal_dark.svg"));
    }

    connect(m_shotBtn, &ToolButton::onPress, this, [ = ] {
        if (m_themeType == 1)
        {
            m_shotBtn->setIcon(QIcon(":/image/newUI/press/screenshot-press.svg"));
        }

        else if (m_themeType == 2)
        {
            m_shotBtn->setIcon(QIcon(":/image/newUI/dark/press/screenshot-press.svg"));
        }
    });

//    m_shotBtn->setToolTip(tr("Switch to shot mode"));
    installTipHint(m_shotBtn, tr("Switch to shot mode"));
//    shotBtn->setStyleSheet(shot_button_style);
    toolBtnList.append(m_shotBtn);

    m_baseLayout = new QHBoxLayout();
    m_baseLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_baseLayout->setContentsMargins(4, 0, 0, 0);
//    m_baseLayout->setMargin(1);
    m_baseLayout->setSpacing(0);
    m_baseLayout->addSpacing(5);

    for (int k = 0; k < toolBtnList.length(); k++) {
        m_baseLayout->addWidget(toolBtnList[k]);
        m_baseLayout->addSpacing(BUTTON_SPACING);

        buttonGroup->addButton(toolBtnList[k]);
    }
//    setLayout(m_baseLayout);
    m_mainTool->setLayout(m_baseLayout);
    addWidget(m_mainTool);
//    qDebug() << recordBtn->palette() << recordBtn->palette().dark().color().name(QColor::HexArgb);
//    qDebug() << recordBtn->palette() << recordBtn->palette().light().color().name(QColor::HexArgb);
//    qDebug() << recordBtn->palette() << recordBtn->palette().buttonText().color().name(QColor::HexArgb);

    connect(buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
    [ = ](int status) {

        DPalette pa;
        if (m_recordBtn->isChecked()) {
//            pa = m_recordBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
            m_recordBtn->setIconSize(QSize(20, 20));

            if (m_themeType == 1) {
                m_recordBtn->setIcon(QIcon(":/image/newUI/checked/screencap-checked.svg"));
            }

            else if (m_themeType == 2) {
                m_recordBtn->setIcon(QIcon(":/image/newUI/dark/checked/screencap-checked.svg"));
            }
//            m_recordBtn->setPalette(pa);

            m_isChecked = true;
            m_recordBtn->update();
            emit buttonChecked(m_isChecked, "record");
        }

        else {
//            pa = m_recordBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
//            m_recordBtn->setPalette(pa);
            m_recordBtn->setIconSize(QSize(20, 20));

            if (m_themeType == 1) {
                m_recordBtn->setIcon(QIcon(":/image/newUI/normal/screencap-normal.svg"));
            }

            else if (m_themeType == 2) {
                m_recordBtn->setIcon(QIcon(":/image/newUI/dark/normal/screencap-normal_dark.svg"));
            }
//            m_recordBtn->setPalette(pa);
        }

        if (m_shotBtn->isChecked()) {
//            pa = m_shotBtn->palette();
//            pa.setColor(DPalette::ButtonText, Qt::white);
//            pa.setColor(DPalette::Dark, QColor("#1C1C1C"));
//            pa.setColor(DPalette::Light, QColor("#1C1C1C"));
            m_shotBtn->setIconSize(QSize(20, 20));

            if (m_themeType == 1) {
                m_shotBtn->setIcon(QIcon(":/image/newUI/checked/screenshot-checked.svg"));
            }

            else if (m_themeType == 2) {
                m_shotBtn->setIcon(QIcon(":/image/newUI/dark/checked/screenshot-checked.svg"));
            }
//            m_shotBtn->setPalette(pa);

            m_isChecked = true;
            m_shotBtn->update();
            emit buttonChecked(m_isChecked, "shot");
        }

        else {
//            pa = m_shotBtn->palette();
//            pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
//            pa.setColor(DPalette::Dark, QColor(227, 227, 227, 150));
//            pa.setColor(DPalette::Light, QColor(230, 230, 230, 150));
            m_shotBtn->setIconSize(QSize(20, 20));

            if (m_themeType == 1) {
                m_shotBtn->setIcon(QIcon(":/image/newUI/normal/screenshot-normal.svg"));
            }

            else if (m_themeType == 2) {
                m_shotBtn->setIcon(QIcon(":/image/newUI/dark/normal/screenshot-normal_dark.svg"));
            }
//            m_shotBtn->setPalette(pa);
        }
    });
    m_shotBtn->click();

}

void MainToolWidget::installTipHint(QWidget *w, const QString &hintstr)
{
    // TODO: parent must be mainframe
    auto hintWidget = new ToolTips("", this->parentWidget()->parentWidget()->parentWidget());
    hintWidget->hide();
    hintWidget->setText(hintstr);
    hintWidget->setFixedHeight(32);
    installHint(w, hintWidget);
}

void MainToolWidget::installHint(QWidget *w, QWidget *hint)
{
    w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    w->installEventFilter(hintFilter);
}

void MainToolWidget::setRecordButtonOut()
{
    m_recordBtn->setDisabled(true);
}

void MainToolWidget::setRecordLauchMode(bool recordLaunch)
{
//    qDebug() << "main record mode";
    if (recordLaunch == true) {
        m_recordBtn->click();
    }

}
