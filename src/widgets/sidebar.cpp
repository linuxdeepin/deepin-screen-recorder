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
#include "sidebar.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <dgraphicsgloweffect.h>
#include <QTimer>
#include <QSettings>
#include <QVBoxLayout>
#include <QBitmap>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 450;
const int TOOLBAR_WIDTH = 50;

const QSize TOOLBAR_WIDGET_SIZE = QSize(50, 450);
const int BUTTON_SPACING = 3;
const int BTN_RADIUS = 3;
}


SideBarWidget::SideBarWidget(DWidget *parent)
    : DBlurEffectWidget(parent),
      m_expanded(false)
{
    setBlurRectXRadius(10);
    setBlurRectYRadius(10);
    setRadius(30);
    setMode(DBlurEffectWidget::GaussianBlur);
    setBlurEnabled(true);
//    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(255, 255, 255, 76.5));
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

    QPixmap pixmap(":/image/newUI/normal/close-normal.svg");

//    m_closeButton = new ToolButton(this);
//    m_closeButton->setIconSize(QSize(40, 40));
//    m_closeButton->setIcon(QIcon(":/image/newUI/normal/close-normal.svg"));
//    m_closeButton->resize(pixmap.size());
//    /* 设置按钮的有效区域 */
//    m_closeButton->setMask(QBitmap(pixmap.mask()));
//    m_closeButton->setStyleSheet(button_style);

    m_closeButton = new DImageButton(this);
//    m_closeButton->setIconSize(QSize(40, 40));
//    m_closeButton->setIcon(QIcon(":/image/newUI/normal/close-normal.svg"));
//    m_closeButton->resize(pixmap.size());
    m_closeButton->setHoverPic(":/image/newUI/hover/close-hover.svg");
    m_closeButton->setNormalPic(":/image/newUI/normal/close-normal.svg");

    QVBoxLayout *VLayout = new QVBoxLayout();
    VLayout->setMargin(0);
//    VLayout->setSpacing(3);
    VLayout->addWidget(m_shotTool, 0, Qt::AlignTop | Qt::AlignHCenter);
    VLayout->addWidget(m_colorTool, 1,  Qt::AlignTop | Qt::AlignHCenter);
    VLayout->addWidget(m_closeButton, 2, Qt::AlignTop | Qt::AlignHCenter);
    setLayout(VLayout);

    connect(m_shotTool, &ShotToolWidget::changeArrowAndLine, this, &SideBarWidget::changeArrowAndLineEvent);
    connect(m_closeButton, &DImageButton::clicked, this, &SideBarWidget::closeSideBar);
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
    DBlurEffectWidget::paintEvent(e);

    QPainter painter(this);
    painter.setPen(QColor(255, 255, 255, 76.5));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawLine(QPointF(BTN_RADIUS, 0), QPointF(this->width() - 1, 0));
}

void SideBarWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    DBlurEffectWidget::showEvent(event);
}



SideBar::SideBar(DWidget *parent) : DLabel(parent)
{

}

SideBar::~SideBar()
{

}

void SideBar::changeShotToolFunc(const QString &func)
{
    m_sidebarWidget->changeShotToolWidget(func);
}

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
    connect(m_sidebarWidget, &SideBarWidget::closeSideBar, this, &SideBar::closeSideBarToMain);
}

void SideBar::setColorFunc(const QString &func)
{
    m_sidebarWidget->changeShotToolWidget(func);
}

void SideBar::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(QColor(0, 0, 0, 25));
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF rect(0, 0, this->width() - 1, this->height() - 1);
    painter.drawRoundedRect(rect.translated(0.5, 0.5), 3, 3, Qt::AbsoluteSize);

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
