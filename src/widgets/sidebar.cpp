// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebar.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSettings>
#include <QVBoxLayout>
#include <QBitmap>

#include <dgraphicsgloweffect.h>


DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 465;
const int TOOLBAR_WIDTH = 66;

const QSize TOOLBAR_WIDGET_SIZE = QSize(66, 465);
//const int BUTTON_SPACING = 3;
//const int BTN_RADIUS = 3;
}


SideBarWidget::SideBarWidget(DWidget *parent)
    : DFloatingWidget(parent)
    ,m_expanded(false)
{
    setBlurBackgroundEnabled(true);
    blurBackground()->setRadius(30);
    blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    blurBackground()->setBlurEnabled(true);
    if (Utils::themeType == 1) {
        blurBackground()->setMaskColor(QColor(255, 255, 255, 76));
    } else {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
    }
    //设置透明效果

    setFixedSize(TOOLBAR_WIDGET_SIZE);

//    qDebug() << "~~~~~~" << this->size();
    m_hSeparatorLine = new DLabel(this);
    m_hSeparatorLine->setObjectName("HorSeparatorLine");
    m_hSeparatorLine->setFixedHeight(1);
    m_colorTool = new ColorToolWidget(this);
    m_shotTool = new ShotToolWidget(this);


    QString button_style = "DPushButton{border-radius:30px;} "
                           "DPushButton::hover{border-image: url(:/image/newUI/hover/close-hover.svg)}";

    QPixmap pixmap(":/newUI/normal/close-normal.svg");
    m_closeButton = new DImageButton(this);
    Utils::setAccessibility(m_closeButton, AC_TOOLBARWIDGET_CLOSE_BUTTON_SIDE);
    if (Utils::themeType == 1) {
        m_closeButton->setHoverPic(":/newUI/hover/close-hover.svg");
        m_closeButton->setNormalPic(":/newUI/normal/close-normal.svg");
    } else {
        m_closeButton->setHoverPic(":/newUI/dark/hover/close-hover_dark.svg");
        m_closeButton->setNormalPic(":/newUI/dark/normal/close-normal_dark.svg");
    }

    QVBoxLayout *VLayout = new QVBoxLayout(this);
    VLayout->setMargin(0);
//    VLayout->setSpacing(3);
    VLayout->addWidget(m_shotTool, 0,  Qt::AlignCenter);
    VLayout->addWidget(m_colorTool, 1,   Qt::AlignCenter);
    VLayout->addWidget(m_closeButton, 2, Qt::AlignCenter);
    setLayout(VLayout);

    connect(m_shotTool, &ShotToolWidget::changeArrowAndLine, this, &SideBarWidget::changeArrowAndLineEvent);
    connect(m_closeButton, &DImageButton::clicked, this, &SideBarWidget::closeSideBar);
    connect(m_colorTool, &ColorToolWidget::colorChecked, m_shotTool, &ShotToolWidget::colorChecked);
}

SideBarWidget::~SideBarWidget()
{

}

void SideBarWidget::changeShotToolWidget(const QString &func)
{
    m_shotTool->switchContent(func);
    m_colorTool->setFunction(func);
}

void SideBarWidget::paintEvent(QPaintEvent *e)
{
    DFloatingWidget::paintEvent(e);
}

void SideBarWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    DFloatingWidget::showEvent(event);
}



SideBar::SideBar(DWidget *parent) : DLabel(parent)
  ,m_sidebarWidget(nullptr), m_expanded(false)
{

}

SideBar::~SideBar()
{

}

void SideBar::changeShotToolFunc(const QString &func)
{
    m_sidebarWidget->changeShotToolWidget(func);
}
/*
bool SideBar::isButtonChecked()
{
    return m_expanded;
}

void SideBar::setExpand(bool expand, QString shapeType)
{
    emit buttonChecked(shapeType);
    if (expand) {
        m_expanded = true;
        setFixedSize(TOOLBAR_WIDTH,
                     TOOLBAR_WIDGET_SIZE.height() * 2 + 3);
        emit heightChanged();
    }

    update();
}
*/
void SideBar::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();

    move(pos.x(), pos.y());
}

void SideBar::initSideBar()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    m_sidebarWidget = new SideBarWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(1, 1, 1, 1);
    vLayout->addStretch();
    vLayout->addWidget(m_sidebarWidget);
    vLayout->addStretch();
    setLayout(vLayout);

    connect(m_sidebarWidget, &SideBarWidget::changeArrowAndLineEvent, this, &SideBar::changeArrowAndLineToMain);
    connect(m_sidebarWidget, &SideBarWidget::closeSideBar, this, [ = ] { this->hide(); });
}
/*
 * never used
void SideBar::setColorFunc(const QString &func)
{
    m_sidebarWidget->changeShotToolWidget(func);
}
*/
void SideBar::paintEvent(QPaintEvent *e)
{
    DLabel::paintEvent(e);
}

void SideBar::enterEvent(QEvent *e)
{
    qApp->setOverrideCursor(Qt::ArrowCursor);
    DLabel::enterEvent(e);
}

bool SideBar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::PaletteChange) {
        qDebug() << obj;
        qDebug() << "--------------";
    }

    return DLabel::eventFilter(obj, event);
}
