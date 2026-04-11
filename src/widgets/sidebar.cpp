// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebar.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../utils/log.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"
#include "shapetoolwidget.h"
#include "aiassistantwidget.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSettings>
#include <QVBoxLayout>
#include <QBitmap>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <dgraphicsgloweffect.h>


DWIDGET_USE_NAMESPACE

namespace {

//const QSize TOOLBAR_WIDGET_SIZE = QSize(66, 465);
//不同图形下二级菜单的大小及长度不一样
const QSize TOOLBAR_WIDGET_SIZE1 = QSize(290, 68);
const QSize TOOLBAR_WIDGET_SIZE2 = QSize(370, 68);
const QSize TOOLBAR_WIDGET_SIZE3 = QSize(500, 68);
const QSize TOOLBAR_WIDGET_SIZE4 = QSize(368, 68);
// const QSize TOOLBAR_WIDGET_AIMODE = QSize(316, 68);

//const int BUTTON_SPACING = 3;
//const int BTN_RADIUS = 3;
const QSize SPLITTER_SIZE = QSize(3, 30);


const QSize TOOLBAR_WIDGET_AIMODE() {
    return QSize(350, 68);
}

}


SideBarWidget::SideBarWidget(MainWindow *pmainwindow, DWidget *parent)
    : DFloatingWidget(parent)
    , m_expanded(false)
{
    qCDebug(dsrApp) << "SideBarWidget constructor called.";
    m_pMainWindow = pmainwindow;
    initSideBarWidget();
}

SideBarWidget::~SideBarWidget()
{
    qCDebug(dsrApp) << "SideBarWidget destructor called.";
}

void SideBarWidget::initSideBarWidget()
{
    qCDebug(dsrApp) << "SideBarWidget::initSideBarWidget called.";
//    this->setAttribute(Qt::WA_StyledBackground, true);
//    this->setStyleSheet("background-color: rgb(255,0, 0)");
    setBlurBackgroundEnabled(true);
    blurBackground()->setRadius(30);
    blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    blurBackground()->setBlurEnabled(true);
    if (Utils::themeType == 1) {
        qCDebug(dsrApp) << "Setting blur background mask color for light theme.";
        blurBackground()->setMaskColor(QColor(0xF7, 0xF7, 0xF7, 153));
    } else {
        qCDebug(dsrApp) << "Setting blur background mask color for dark theme.";
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
    }
    //设置侧边栏大小
    //setFixedSize(TOOLBAR_WIDGET_SIZE);

    //分割线
    m_seperator = new DVerticalLine(this);
    //m_seperator->setDisabled(true);
    m_seperator->setFixedSize(SPLITTER_SIZE);
    m_seperator1 = new DVerticalLine(this);
    m_seperator1->setFixedSize(SPLITTER_SIZE);
    
    // 初始化各个工具组件
    m_shapeTool = new ShapeToolWidget(this);
    m_colorTool = new ColorToolWidget(this);
    m_shotTool = new ShotToolWidget(m_pMainWindow, this);
    m_aiAssistantTool = new AIAssistantWidget(this);
    m_aiAssistantTool->hide();
    
    // 连接 AI 面板信号到 MainWindow
    connect(m_aiAssistantTool, &AIAssistantWidget::functionSelected, this,
            [this](AIAssistantWidget::AIFunction func) {
        m_pMainWindow->onAiAssistantSelected(static_cast<int>(func));
    });

    // 默认隐藏ShapeToolWidget和对应的分割线
    m_shapeTool->hide();
    m_seperator1->hide();

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(10, 0, 10, 0);
    hLayout->setSpacing(0);  // 设置组件间距为0
    hLayout->addWidget(m_shapeTool);
    hLayout->addWidget(m_seperator1);
    hLayout->addWidget(m_shotTool);
    hLayout->addWidget(m_aiAssistantTool);
    hLayout->setAlignment(m_aiAssistantTool, Qt::AlignCenter);
    hLayout->addWidget(m_seperator);
    hLayout->addWidget(m_colorTool);
    setLayout(hLayout);

    setMinimumSize(TOOLBAR_WIDGET_SIZE1);

    connect(m_shotTool, &ShotToolWidget::changeArrowAndLine, this, &SideBarWidget::changeArrowAndLineEvent);
    connect(m_colorTool, &ColorToolWidget::colorChecked, m_shotTool, &ShotToolWidget::colorChecked);
    connect(m_shapeTool, &ShapeToolWidget::shapeSelected, m_shotTool, &ShotToolWidget::shapeSelected);
}

void SideBarWidget::changeShotToolWidget(const QString &func)
{
    qCDebug(dsrApp) << "SideBarWidget::changeShotToolWidget called with func:" << func;
    
    // 先切换内容，确保布局正确
    m_shotTool->switchContent(func);
    m_colorTool->setFunction(func);
    
    // 处理分隔符显示（AI 模式同样隐藏主分割线）
    if (func == "effect" || func == "aiassistant") {
        qCDebug(dsrApp) << "Effect/AI mode: hiding separator";
        m_seperator->hide();
    } else {
        qCDebug(dsrApp) << "Non-effect mode: showing separator";
        m_seperator->show();
    }

    // 显示或隐藏各面板
    if (func == "gio" || func == "rectangle" || func == "oval") {
        qDebug(dsrApp) << "Geometry mode: showing ShapeToolWidget";
        m_shapeTool->show();
        m_seperator1->show();
        m_aiAssistantTool->hide();
        m_shotTool->show();
        m_colorTool->show();
        
        
        // 无论是几何图形模式还是直接选择矩形/椭圆，都读取上次选中的形状
        QString currentShape = ConfigSettings::instance()->getValue("shape", "current").toString();
        qDebug(dsrApp) << "Current shape from config:" << currentShape;
        
        // 如果没有历史记录，则默认使用矩形
        if (currentShape.isEmpty() || (currentShape != "rectangle" && currentShape != "oval")) {
            currentShape = "rectangle"; // 默认使用矩形
            qDebug(dsrApp) << "No valid shape in config, using rectangle as default.";
            ConfigSettings::instance()->setValue("shape", "current", currentShape);
        }
        
        // 如果直接选择了具体形状，则使用该形状，但不触发信号
        if (func == "rectangle" || func == "oval") {
            // 直接设置按钮状态，但不发送信号
            if (func == "rectangle") {
                qDebug(dsrApp) << "Directly selecting rectangle button.";
                m_shapeTool->selectRectangleWithoutSignal();
            } else if (func == "oval") {
                qDebug(dsrApp) << "Directly selecting oval button.";
                m_shapeTool->selectOvalWithoutSignal();
            }
            // 更新配置
            ConfigSettings::instance()->setValue("shape", "current", func);
        } else {
            // 如果是gio模式，则选择上次使用的形状
            qDebug(dsrApp) << "Selecting shape based on history:" << currentShape;
            m_shapeTool->selectShape(currentShape);
        }
    } else if (func == "aiassistant") {
        // AI 助手作为独立面板显示，隐藏其它
        qCWarning(dsrApp) << "AI assistant mode: showing AI assistant tool.";
        m_shapeTool->hide();
        m_seperator1->hide();
        m_shotTool->hide();
        m_colorTool->hide();
        m_seperator->hide();
        m_aiAssistantTool->show();
        
        // 使用 AI 控件自己计算的推荐尺寸，自适应不同语言
        QSize recommendedSize = m_aiAssistantTool->sizeHint();
        qCDebug(dsrApp) << "Using AI assistant recommended size:" << recommendedSize;
        resize(recommendedSize);
        m_currentFunc = func;
    } else if (func == "effect") {
        // 模糊工具不需要颜色功能，隐藏颜色工具和形状工具
        qCDebug(dsrApp) << "Effect mode: hiding ShapeToolWidget and ColorToolWidget";
        m_shapeTool->hide();
        m_seperator1->hide();
        m_aiAssistantTool->hide();
        m_shotTool->show();
        m_colorTool->hide();
        setMinimumSize(TOOLBAR_WIDGET_SIZE1);
        m_currentFunc = func;
    } else {
        qCDebug(dsrApp) << "Non-geometry mode: hiding ShapeToolWidget";
        m_shapeTool->hide();
        m_seperator1->hide();
        m_aiAssistantTool->hide();
        m_shotTool->show();
        m_colorTool->show();
        setMinimumSize(TOOLBAR_WIDGET_SIZE1);
        m_currentFunc = func;

    }

    //不同图形下二级菜单的大小及长度不一样
    if (func == "gio" || func == "rectangle" || func == "oval") {
        qCDebug(dsrApp) << "Resizing sidebar for geometry shapes.";
        resize(TOOLBAR_WIDGET_SIZE4);
    }
    else if(
        func == "line" ||
        func == "arrow" ||
        func == "pen") {
        qCDebug(dsrApp) << "Resizing sidebar for line, arrow, or pen.";
        resize(TOOLBAR_WIDGET_SIZE1);
    } else if (func == "text") {
        qCDebug(dsrApp) << "Resizing sidebar for text.";
        resize(TOOLBAR_WIDGET_SIZE2);
    } else if (func == "effect") {
        qCDebug(dsrApp) << "Resizing sidebar for effect.";
        resize(TOOLBAR_WIDGET_SIZE3);
    } else if(func == "aiassistant") {
        // AI 模式的尺寸已经在上面通过 sizeHint() 设置了，这里不需要再次设置
        qCDebug(dsrApp) << "AI assistant mode, size already set by sizeHint().";
    } else {
        resize(TOOLBAR_WIDGET_SIZE1);
    }
    
    // 确保布局更新
    layout()->invalidate();
    layout()->activate();
    updateGeometry();
    update();
}

int SideBarWidget::getSideBarWidth(const QString &func)
{
    qCDebug(dsrApp) << "SideBarWidget::getSideBarWidth called for function:" << func;
    int width = TOOLBAR_WIDGET_SIZE1.width();
    if (func == "rectangle" ||
            func == "oval" ||
            func == "line" ||
            func == "arrow" ||
            func == "pen") {
        resize(TOOLBAR_WIDGET_SIZE1);
        width = TOOLBAR_WIDGET_SIZE1.width();
    } else if (func == "text") {
        width = TOOLBAR_WIDGET_SIZE2.width();
    } else if (func == "effect") {
        width = TOOLBAR_WIDGET_SIZE3.width();
    } else if (func == "aiassistant") {
        // 使用 AI 控件的实际推荐宽度
        width = m_aiAssistantTool->sizeHint().width();
        qCDebug(dsrApp) << "AI assistant width from sizeHint():" << width;
    }
    
    qCDebug(dsrApp) << "Sidebar width determined:" << width;
    return width;
}

bool SideBarWidget::isAIMode() const
{
    return m_currentFunc == "aiassistant";
}

void SideBarWidget::paintEvent(QPaintEvent *e)
{
    DFloatingWidget::paintEvent(e);
}

void SideBarWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    qCDebug(dsrApp) << "SideBarWidget::showEvent called.";
    DFloatingWidget::showEvent(event);
}



SideBar::SideBar(DWidget *parent) : DLabel(parent)
    , m_sidebarWidget(nullptr), m_expanded(false)
{
    qCDebug(dsrApp) << "SideBar constructor called.";
}

SideBar::~SideBar()
{
    qCDebug(dsrApp) << "SideBar destructor called.";
}

void SideBar::changeShotToolFunc(const QString &func)
{
    qCDebug(dsrApp) << "SideBar::changeShotToolFunc called with func:" << func;
    m_sidebarWidget->changeShotToolWidget(func);
    resize(m_sidebarWidget->size());
}

int SideBar::getSideBarWidth(const QString &func)
{
    qCDebug(dsrApp) << "SideBar::getSideBarWidth called with func:" << func;
    return m_sidebarWidget->getSideBarWidth(func);
}

bool SideBar::isDraged()
{
    qCDebug(dsrApp) << "SideBar::isDraged called.";
    return m_isDrag;
}

bool SideBar::isPressed()
{
    qCDebug(dsrApp) << "SideBar::isPressed called.";
    return m_isPress;
}

void SideBar::showAt(QPoint pos)
{
    // pos已经是相对于父控件的本地坐标，直接使用
    if (!isVisible())
        this->show();

    move(pos.x(), pos.y());
}

void SideBar::showWidget(){
    qCDebug(dsrApp) << "SideBar::showWidget called.";
    m_sidebarWidget->show();
}

void SideBar::hideWidget(){
    qCDebug(dsrApp) << "SideBar::hideWidget called.";
    m_sidebarWidget->hide();
}

void SideBar::initSideBar(MainWindow *pmainWindow)
{
    qCDebug(dsrApp) << "SideBar::initSideBar called.";
    m_pMainWindow = pmainWindow;
    m_sidebarWidget = new SideBarWidget(pmainWindow, this);
    resize(m_sidebarWidget->size());

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
    qCDebug(dsrApp) << "SideBar::paintEvent called.";
    DLabel::paintEvent(e);
}

#if (QT_VERSION_MAJOR == 5)
void SideBar::enterEvent(QEvent *e)
{
    qCDebug(dsrApp) << "SideBar::enterEvent (Qt5) called.";
    if (m_sidebarWidget && m_sidebarWidget->isAIMode()) {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    } else {
        QApplication::setOverrideCursor(Qt::OpenHandCursor);
    }
    DLabel::enterEvent(e);
}
#elif (QT_VERSION_MAJOR == 6)
void SideBar::enterEvent(QEnterEvent *e)
{
    qCDebug(dsrApp) << "SideBar::enterEvent (Qt6) called.";
    if (m_sidebarWidget && m_sidebarWidget->isAIMode()) {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    } else {
        QApplication::setOverrideCursor(Qt::OpenHandCursor);
    }
    DLabel::enterEvent(e);
}
#endif

bool SideBar::eventFilter(QObject *obj, QEvent *event)
{
    qCDebug(dsrApp) << "SideBar::eventFilter called.";
    return DLabel::eventFilter(obj, event);
}

void SideBar::mousePressEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "SideBar::mousePressEvent called.";
    if (event->button() == Qt::LeftButton) {
        qCDebug(dsrApp) << "Mouse left button pressed.";
        if (m_sidebarWidget && m_sidebarWidget->isAIMode()) {
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        } else {
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        }
        m_isPress = true;
        //获得鼠标的初始位置
        m_mouseStartPoint = event->globalPos();
        //获得窗口的初始位置
        m_windowStartPoint = this->frameGeometry().topLeft();
        m_pMainWindow->getToolBarStartPressPoint();
    }
}

void SideBar::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "SideBar::mouseMoveEvent called.";
    //判断是否在拖拽移动
    if (m_isPress) {
        qCDebug(dsrApp) << "Mouse is pressed, moving sidebar.";
        m_isDrag = true;
        //获得鼠标移动的距离
        QPoint move_distance = event->globalPos() - m_mouseStartPoint;
        //改变窗口的位置
        if (m_pMainWindow) {
            m_pMainWindow->moveToolBars(m_windowStartPoint, move_distance);
        }
    }
}

void SideBar::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "SideBar::mouseReleaseEvent called.";
    //放下左键即停止移动
    if (event->button() == Qt::LeftButton) {
        qCDebug(dsrApp) << "Mouse left button released.";
        m_isPress = false;
        if (m_sidebarWidget && m_sidebarWidget->isAIMode()) {
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        } else {
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
        }
    }
}
