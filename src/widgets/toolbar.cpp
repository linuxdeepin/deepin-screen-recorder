/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Maintainer: Peng Hui<penghui@deepin.com>
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

#include "toolbar.h"
#include "../utils/baseutils.h"
#include "../utils/saveutils.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <dgraphicsgloweffect.h>
#include <QTimer>
#include <QSettings>
#include <QBitmap>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 47;
const int TOOLBAR_WIDTH = 505;

const QSize TOOLBAR_WIDGET_SIZE = QSize(505, 47);
const int BUTTON_SPACING = 3;
const int BTN_RADIUS = 3;
}

ToolBarWidget::ToolBarWidget(QWidget *parent)
    : DBlurEffectWidget(parent),
      m_expanded(false)
{
    setBlurRectXRadius(10);
    setBlurRectYRadius(10);
    setRadius(30);
    setMode(DBlurEffectWidget::GaussianBlur);
    setBlurEnabled(true);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(255, 255, 255, 76.5));
    //设置透明效果
//    setMaskAlpha(0);
//    setMaskColor(DBlurEffectWidget::LightColor);
    setFixedSize(TOOLBAR_WIDGET_SIZE);

//    qDebug() << "~~~~~~" << this->size();
    m_hSeparatorLine = new QLabel(this);
    m_hSeparatorLine->setObjectName("HorSeparatorLine");
    m_hSeparatorLine->setFixedHeight(1);

//    m_majToolbar = new MajToolBar(this);
//    m_subToolbar = new SubToolBar(this);

    m_mainTool = new MainToolWidget(this);
    m_subTool = new SubToolWidget(this);

    QString button_style = "QPushButton{border-radius:30px;} "
                           "QPushButton::hover{border-image: url(:/image/newUI/hover/close-hover.svg)}";

    QPixmap pixmap(":/image/newUI/normal/close-normal.svg");

    m_closeButton = new ToolButton(this);
    m_closeButton->setIconSize(QSize(40, 40));
    m_closeButton->setIcon(QIcon(":/image/newUI/normal/close-normal.svg"));
    m_closeButton->resize(pixmap.size());
    /* 设置按钮的有效区域 */
    m_closeButton->setMask(QBitmap(pixmap.mask()));
    m_closeButton->setStyleSheet(button_style);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(2);
    hLayout->addWidget(m_mainTool, 0, Qt::AlignLeft);
    hLayout->addWidget(m_subTool, 1, Qt::AlignLeft);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_closeButton, 2, Qt::AlignLeft);
    setLayout(hLayout);

    connect(m_mainTool, &MainToolWidget::buttonChecked, this, &ToolBarWidget::setExpand);
    connect(m_closeButton, &DPushButton::clicked, this, &ToolBarWidget::closeButtonSignal);
    connect(m_subTool, &SubToolWidget::keyBoardButtonClicked, this, &ToolBarWidget::keyBoardCheckedSlot);
    connect(m_subTool, &SubToolWidget::mouseBoardButtonClicked, this, &ToolBarWidget::mouseCheckedSignalToToolBar);
    connect(m_subTool, SIGNAL(microphoneActionChecked(bool)), this, SIGNAL(microphoneActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(systemAudioActionChecked(bool)), this, SIGNAL(systemAudioActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(cameraActionChecked(bool)), this, SIGNAL(cameraActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(gifActionChecked(bool)), this, SIGNAL(gifActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(mp4ActionChecked(bool)), this, SIGNAL(mp4ActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(videoFrameRateChanged(int)), this, SIGNAL(frameRateChangedSignal(int)));
    connect(m_subTool, SIGNAL(changeShotToolFunc(const QString &)), this, SIGNAL(shotToolChangedSignal(const QString &)));
}

void ToolBarWidget::paintEvent(QPaintEvent *e)
{
    DBlurEffectWidget::paintEvent(e);

    QPainter painter(this);
    painter.setPen(QColor(255, 255, 255, 76.5));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawLine(QPointF(BTN_RADIUS, 0), QPointF(this->width() - 1, 0));
}

void ToolBarWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    QSettings settings(this);
    settings.beginGroup("common");
    bool expand = settings.value("expand_savelist", false).toBool();
    settings.endGroup();

    if (expand)
        QTimer::singleShot(0, this, [ = ] { setExpand(expand, "saveList"); });

    DBlurEffectWidget::showEvent(event);
}

bool ToolBarWidget::isButtonChecked()
{
    return m_expanded;
}

void ToolBarWidget::specifiedSavePath()
{
    m_majToolbar->specificedSavePath();
}

void ToolBarWidget::keyBoardCheckedSlot(bool checked)
{
    emit keyBoardCheckedSignal(checked);
}

void ToolBarWidget::changeArrowAndLineFromBar(int line)
{
    m_subTool->changeArrowAndLineFromSideBar(line);
}

void ToolBarWidget::setExpand(bool expand, QString shapeType)
{
//    m_subToolbar->switchContent(shapeType);
    m_subTool->switchContent(shapeType);
//    emit expandChanged(expand, shapeType);

//    if (expand) {
//        m_expanded = true;
//        setFixedSize(TOOLBAR_WIDGET_SIZE.width(),
//                                 TOOLBAR_WIDGET_SIZE.height()*2+1);
//        m_hSeparatorLine->show();
//        m_subToolbar->show();
//    }
    emit changeFunctionSignal(shapeType);
    update();
}

ToolBarWidget::~ToolBarWidget() {}


ToolBar::ToolBar(QWidget *parent)
    : QLabel(parent)
{

}

void ToolBar::setExpand(bool expand, QString shapeType)
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

void ToolBar::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(QColor(0, 0, 0, 25));
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF rect(0, 0, this->width() - 1, this->height() - 1);
    painter.drawRoundedRect(rect.translated(0.5, 0.5), 3, 3, Qt::AbsoluteSize);

    QLabel::paintEvent(e);
}

void ToolBar::enterEvent(QEvent *e)
{
    qApp->setOverrideCursor(Qt::ArrowCursor);
    QLabel::enterEvent(e);
}

bool ToolBar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::PaletteChange) {
        qDebug() << obj;
        qDebug() << "--------------";
    }

    return QLabel::eventFilter(obj, event);
}

void ToolBar::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();

    move(pos.x(), pos.y());
}

void ToolBar::specificedSavePath()
{
    m_toolbarWidget->specifiedSavePath();
}

void ToolBar::currentFunctionMode(QString shapeType)
{
    emit currentFunctionToMain(shapeType);
}

void ToolBar::keyBoardCheckedToMainSlot(bool checked)
{
    emit keyBoardCheckedToMain(checked);
}

void ToolBar::microphoneActionCheckedToMainSlot(bool checked)
{
    emit microphoneActionCheckedToMain(checked);
}
void ToolBar::systemAudioActionCheckedToMainSlot(bool checked)
{
    emit systemAudioActionCheckedToMain(checked);
}

void ToolBar::changeArrowAndLineFromMain(int line)
{
    m_toolbarWidget->changeArrowAndLineFromBar(line);
}

void ToolBar::initToolBar()
{
    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    m_toolbarWidget = new ToolBarWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(1, 1, 1, 1);
    vLayout->addStretch();
    vLayout->addWidget(m_toolbarWidget);
    vLayout->addStretch();
    setLayout(vLayout);

    connect(m_toolbarWidget, &ToolBarWidget::expandChanged, this, &ToolBar::setExpand);
    connect(m_toolbarWidget, &ToolBarWidget::saveImage, this, &ToolBar::requestSaveScreenshot);
    connect(m_toolbarWidget, &ToolBarWidget::closeButtonSignal, this, &ToolBar::closeButtonToMain);
    connect(m_toolbarWidget, &ToolBarWidget::colorChanged, this, &ToolBar::updateColor);
    connect(this, &ToolBar::shapePressed, m_toolbarWidget, &ToolBarWidget::shapePressed);
    connect(this, &ToolBar::saveBtnPressed, m_toolbarWidget, &ToolBarWidget::saveBtnPressed);
    connect(m_toolbarWidget, &ToolBarWidget::saveSpecifiedPath, this, &ToolBar::saveSpecifiedPath);
    connect(m_toolbarWidget, &ToolBarWidget::closed, this, &ToolBar::closed);

    connect(m_toolbarWidget, &ToolBarWidget::changeFunctionSignal, this, &ToolBar::currentFunctionMode);
    connect(m_toolbarWidget, &ToolBarWidget::keyBoardCheckedSignal, this, &ToolBar::keyBoardCheckedToMainSlot);
    connect(m_toolbarWidget, &ToolBarWidget::microphoneActionCheckedSignal, this, &ToolBar::microphoneActionCheckedToMainSlot);
    connect(m_toolbarWidget, &ToolBarWidget::systemAudioActionCheckedSignal, this, &ToolBar::systemAudioActionCheckedToMainSlot);
    connect(m_toolbarWidget, &ToolBarWidget::cameraActionCheckedSignal, this, &ToolBar::cameraActionCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::mouseCheckedSignalToToolBar, this, &ToolBar::mouseCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::gifActionCheckedSignal, this, &ToolBar::gifActionCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::mp4ActionCheckedSignal, this, &ToolBar::mp4ActionCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::frameRateChangedSignal, this, &ToolBar::frameRateChangedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::shotToolChangedSignal, this, &ToolBar::shotToolChangedToMain);
}
bool ToolBar::isButtonChecked()
{
    return m_expanded;
}

ToolBar::~ToolBar()
{
}
