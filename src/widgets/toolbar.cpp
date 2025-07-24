// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbar.h"
#include "../utils/baseutils.h"
#include "../utils/saveutils.h"
#include "../utils.h"
#include "../utils/configsettings.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"
#include "../utils/log.h"

#include <DIconButton>

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSettings>
#include <QBitmap>

#include <dgraphicsgloweffect.h>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 68;
const int TOOLBAR_WIDTH = 425;
}

ToolBarWidget::ToolBarWidget(MainWindow *pMainwindow, DWidget *parent)
    : DFloatingWidget(parent)
{
    qCDebug(dsrApp) << "ToolBarWidget constructor called.";
    setBlurBackgroundEnabled(true);
    blurBackground()->setRadius(30);
    blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    blurBackground()->setBlurEnabled(true);
    blurBackground()->setBlendMode(DBlurEffectWidget::InWidgetBlend);

    if (Utils::themeType == 1) {
        blurBackground()->setMaskColor(QColor(0xF7, 0xF7, 0xF7, 153));
    } else {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
    }

    m_hSeparatorLine = new DLabel(this);
    m_hSeparatorLine->setObjectName("HorSeparatorLine");
    m_hSeparatorLine->setFixedHeight(1);
    //分配pMainwindow主窗口指针给SubToolWidget（ToolTips需要该指针）
    m_subTool = new SubToolWidget(pMainwindow, this);
    // m_subTool->setVisible(false);
    //关闭按钮
    m_saveToLocalDirButton = new ToolButton(this);
    m_saveToLocalDirButton->setCheckable(false);
    m_saveToLocalDirButton->setFlat(true);
    m_closeButton = new ToolButton(this);
    m_closeButton->setCheckable(false);
    m_confirmButton = new ToolButton(this);
    m_confirmButton->setCheckable(false);

    m_closeButton->setIconSize(QSize(36, 36));
    m_confirmButton->setIconSize(QSize(36, 36));
    m_saveToLocalDirButton->setIconSize(QSize(56, 36));
    m_closeButton->setFixedSize(QSize(36, 36));
    m_confirmButton->setFixedSize(QSize(36, 36));
    m_saveToLocalDirButton->setFixedSize(QSize(56, 36));
    m_closeButton->setIcon(QIcon::fromTheme("close"));
    m_confirmButton->setIcon(QIcon::fromTheme("confirm"));

    m_saveToLocalDirButton->setText(QString(tr("save")));

    m_subTool->installTipHint(m_closeButton, tr("Close (Esc)"));
    m_subTool->installTipHint(m_confirmButton, tr("OK (Enter)"));
    m_subTool->installTipHint(m_saveToLocalDirButton, tr("save"));

    Utils::setAccessibility(m_closeButton, AC_TOOLBARWIDGET_CLOSE_BUTTON_TOOL);
    Utils::setAccessibility(m_confirmButton, AC_TOOLBARWIDGET_CLOSE_BUTTON_TOOL);
    Utils::setAccessibility(m_saveToLocalDirButton, AC_TOOLBARWIDGET_CLOSE_BUTTON_TOOL);

    setFixedHeight(TOOLBAR_HEIGHT);
    if (Utils::is3rdInterfaceStart) {
        qCDebug(dsrApp) << "Setting minimum width for 3rd interface mode";
        m_subTool->setMinimumWidth(TOOLBAR_WIDTH - 160); //减去隐藏按钮的最小宽度和
    }

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_closeButton, 0,  Qt::AlignCenter);
    hLayout->addWidget(m_confirmButton, 0,  Qt::AlignCenter);
    hLayout->addWidget(m_saveToLocalDirButton, 0,  Qt::AlignCenter);
    hLayout->addSpacing(10);
    setLayout(hLayout);

    connect(m_confirmButton, &ToolButton::clicked, pMainwindow, &MainWindow::confirm);
    connect(m_saveToLocalDirButton, &ToolButton::clicked,pMainwindow, &MainWindow::saveScreenShotToFile);

    connect(m_closeButton, &ToolButton::clicked, pMainwindow, &MainWindow::exitApp);    
    connect(m_subTool, &SubToolWidget::keyBoardButtonClicked, pMainwindow, &MainWindow::changeKeyBoardShowEvent);
    connect(m_subTool, &SubToolWidget::mouseBoardButtonClicked, pMainwindow, &MainWindow::changeMouseShowEvent);

    // TODO:treeland适配
    connect(m_subTool, &SubToolWidget::shotOptionMenuShown, pMainwindow, &MainWindow::handleOptionMenuShown);

    QMetaObject::Connection connectHandle =  connect(m_subTool, SIGNAL(cameraActionChecked(bool)), pMainwindow, SLOT(changeCameraSelectEvent(bool)));
    if (!connectHandle) {
        qCDebug(dsrApp) << "Connect cameraActionChecked signal failed!";
    }
    connect(m_subTool, &SubToolWidget::changeShotToolFunc, pMainwindow, &MainWindow::changeShotToolEvent);
    connect(m_subTool, &SubToolWidget::changeShotToolFunc, this, &ToolBarWidget::changeShotToolEvent);
}

void ToolBarWidget::setScrollShotDisabled(const bool state)
{
    qCDebug(dsrApp) << "ToolBarWidget::setScrollShotDisabled called with state:" << state;
    m_subTool->setScrollShotDisabled(state);
}

void ToolBarWidget::setPinScreenshotsEnable(const bool &state)
{
    qCDebug(dsrApp) << "ToolBarWidget::setPinScreenshotsEnable called with state:" << state;
    m_subTool->setPinScreenshotsEnable(state);

}

void ToolBarWidget::setOcrScreenshotsEnable(const bool &state)
{
    qCDebug(dsrApp) << "ToolBarWidget::setOcrScreenshotsEnable called with state:" << state;
    m_subTool->setOcrScreenshotEnable(state);
}

void ToolBarWidget::setButEnableOnLockScreen(const bool &state)
{
    qCDebug(dsrApp) << "ToolBarWidget::setButEnableOnLockScreen called with state:" << state;
    m_subTool->setButEnableOnLockScreen(state);
}

int ToolBarWidget::getFuncSubToolX(QString &func)
{
    qCDebug(dsrApp) << "ToolBarWidget::getFuncSubToolX called for function:" << func;
    return m_subTool->getFuncSubToolX(func);
}

void ToolBarWidget::setRecordLaunchFromMain(const unsigned int funType)
{
    qCDebug(dsrApp) << "ToolBarWidget::setRecordLaunchFromMain called with function type:" << funType;
    m_subTool->setRecordLaunchMode(funType);

}

void ToolBarWidget::setRecordButtonDisable()
{
    qCDebug(dsrApp) << "ToolBarWidget::setRecordButtonDisable called.";
   // m_subTool->setRecordButtonDisable();
}

void ToolBarWidget::setVideoInitFromMain()
{
    qCDebug(dsrApp) << "ToolBarWidget::setVideoInitFromMain called.";
    m_subTool->setVideoButtonInitFromSub();
}

void ToolBarWidget::shapeClickedFromBar(QString shape)
{
    qCDebug(dsrApp) << "ToolBarWidget::shapeClickedFromBar called with shape:" << shape;
    m_subTool->shapeClickedFromWidget(shape);
}

void ToolBarWidget::setCameraDeviceEnable(bool status)
{
    qCDebug(dsrApp) << "ToolBarWidget::setCameraDeviceEnable called with status:" << status;
    m_subTool->setCameraDeviceEnable(status);
}

void ToolBarWidget::changeShotToolEvent(const QString &func)
{
    if (!m_saveToLocalDirButton)
        return;

    if(func == "shot" && !m_saveToLocalDirButton->isVisible()) {
        m_saveToLocalDirButton->setVisible(true);
    } else if(func == "record") {
        m_saveToLocalDirButton->setVisible(false);
    }
}
/*
void ToolBarWidget::setExpand(bool expand, QString shapeType)
{
    Q_UNUSED(expand);
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
*/

QRect ToolBarWidget::getShotOptionRect(){
    qCDebug(dsrApp) << "ToolBarWidget::getShotOptionRect called.";
   return m_subTool->getShotOptionRect();
}

ToolBarWidget::~ToolBarWidget() {
    qCDebug(dsrApp) << "ToolBarWidget destructor called.";
}


ToolBar::ToolBar(DWidget *parent)
    : DLabel(parent)
{
    //setAttribute(Qt::WA_TranslucentBackground);
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    m_toolbarWidget = nullptr;
}

void ToolBar::paintEvent(QPaintEvent *e)
{
    DLabel::paintEvent(e);
}

#if (QT_VERSION_MAJOR == 5)
void ToolBar::enterEvent(QEvent *e)
{
    //    qApp->setOverrideCursor(Qt::ArrowCursor);
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
    DLabel::enterEvent(e);
}
#elif (QT_VERSION_MAJOR == 6)
void ToolBar::enterEvent(QEnterEvent *e)
{
    //    qApp->setOverrideCursor(Qt::ArrowCursor);
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
    DLabel::enterEvent(e);
}
#endif

bool ToolBar::eventFilter(QObject *obj, QEvent *event)
{
    return DLabel::eventFilter(obj, event);
}

void ToolBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        m_isPress = true;
        //获得鼠标的初始位置
        m_mouseStartPoint = event->globalPos();
        //获得窗口的初始位置
        m_windowStartPoint = this->frameGeometry().topLeft();
        m_pMainWindow->getSideBarStartPressPoint();
    }

}

void ToolBar::mouseMoveEvent(QMouseEvent *event)
{
    //判断是否在拖拽移动
    if (m_isPress) {
        m_isDrag = true;
        //获得鼠标移动的距离
        QPoint move_distance = event->globalPos() - m_mouseStartPoint;
        //改变窗口的位置
//        this->move(m_windowStartPoint + move_distance);
        if (m_pMainWindow) {
            m_pMainWindow->moveToolBars(m_windowStartPoint, move_distance);
        }
    }
}

void ToolBar::mouseReleaseEvent(QMouseEvent *event)
{
    //放下左键即停止移动
    if (event->button() == Qt::LeftButton) {
        m_isPress = false;
        m_isDrag = false;
        QApplication::setOverrideCursor(Qt::OpenHandCursor);
    }
}

void ToolBar::setScrollShotDisabled(const bool state)
{
    m_toolbarWidget->setScrollShotDisabled(state);
}

void ToolBar::setPinScreenshotsEnable(const bool &state)
{
    m_toolbarWidget->setPinScreenshotsEnable(state);
}

void ToolBar::setOcrScreenshotsEnable(const bool &state)
{
    m_toolbarWidget->setOcrScreenshotsEnable(state);
}

void ToolBar::setButEnableOnLockScreen(const bool &state)
{
    m_toolbarWidget->setButEnableOnLockScreen(state);
}

int ToolBar::getFuncSubToolX(QString &func)
{
    return m_toolbarWidget->getFuncSubToolX(func);
}

bool ToolBar::isDraged()
{
    return m_isDrag;
}

bool ToolBar::isPressed()
{
    return m_isPress;
}

void ToolBar::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();

    move(pos.x(), pos.y());
}


void ToolBar::currentFunctionMode(QString shapeType)
{
    qCDebug(dsrApp) << "Setting current function mode:" << shapeType;
    DPalette pa;
    update();
    emit currentFunctionToMain(shapeType);
}

void ToolBar::initToolBar(MainWindow *pmainWindow)
{
    m_pMainWindow = pmainWindow;
    setFixedHeight(TOOLBAR_HEIGHT);

    m_toolbarWidget = new ToolBarWidget(pmainWindow, this);
    QHBoxLayout *vLayout = new QHBoxLayout();
    vLayout->setSizeConstraint(QLayout::SetFixedSize);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addStretch();
    vLayout->addWidget(m_toolbarWidget);
    vLayout->addStretch();
    setLayout(vLayout);
    update();

    connect(m_toolbarWidget, &ToolBarWidget::changeFunctionSignal, this, &ToolBar::currentFunctionMode);
}

void ToolBar::showWidget(){
    if(m_toolbarWidget)
        m_toolbarWidget->show();
}

void ToolBar::hideWidget(){
    if(m_toolbarWidget)
        m_toolbarWidget->hide();
}

void ToolBar::setRecordButtonDisable()
{
    m_toolbarWidget->setRecordButtonDisable();
}

void ToolBar::setRecordLaunchMode(const unsigned int funType)
{
    qCDebug(dsrApp) << "Setting record launch mode:" << funType;
    m_toolbarWidget->setRecordLaunchFromMain(funType);
}

void ToolBar::setVideoButtonInit()
{
    m_toolbarWidget->setVideoInitFromMain();
}

void ToolBar::shapeClickedFromMain(QString shape)
{
    // 在工具栏显示之前，触发MainWindow::initShortcut()中的快捷键
    // R O L P T
    // 导致button没有初始化就执行click，导致崩溃。
    if (m_toolbarWidget) {
        m_toolbarWidget->shapeClickedFromBar(shape);
    }
}


void ToolBar::setCameraDeviceEnable(bool status)
{
    qCDebug(dsrApp) << "Setting camera device enable:" << status;
    m_toolbarWidget->setCameraDeviceEnable(status);
}

QRect ToolBar::getShotOptionRect(){
   return m_toolbarWidget->getShotOptionRect();
}
ToolBar::~ToolBar()
{
}
