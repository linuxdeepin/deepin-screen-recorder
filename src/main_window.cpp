/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *               2011 ~ 2018 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
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

#include <QApplication>
#include <QTimer>
#include <QKeyEvent>
#include <QObject>
#include <QPainter>
#include <QWidget>
#include <DWindowManagerHelper>
#include <DForeignWindow>
#include <QDebug>

#include "main_window.h"
#include <QVBoxLayout>
#include <QProcess>
#include "utils.h"
#include "record_button.h"
#include "record_option_panel.h"
#include "countdown_tooltip.h"
#include "constant.h"
#include <dscreenwindowsutil.h>
#include "utils/audioutils.h"
#include <DHiDPIHelper>

const int MainWindow::CURSOR_BOUND = 5;
const int MainWindow::RECORD_MIN_SIZE = 200;
const int MainWindow::DRAG_POINT_RADIUS = 8;

const int MainWindow::RECORD_BUTTON_NORMAL = 0;
const int MainWindow::RECORD_BUTTON_WAIT = 1;
const int MainWindow::RECORD_BUTTON_RECORDING = 2;

const int MainWindow::ACTION_MOVE = 0;
const int MainWindow::ACTION_RESIZE_TOP_LEFT = 1;
const int MainWindow::ACTION_RESIZE_TOP_RIGHT = 2;
const int MainWindow::ACTION_RESIZE_BOTTOM_LEFT = 3;
const int MainWindow::ACTION_RESIZE_BOTTOM_RIGHT = 4;
const int MainWindow::ACTION_RESIZE_TOP = 5;
const int MainWindow::ACTION_RESIZE_BOTTOM = 6;
const int MainWindow::ACTION_RESIZE_LEFT = 7;
const int MainWindow::ACTION_RESIZE_RIGHT = 8;

const int MainWindow::RECORD_OPTIONAL_PADDING = 12;


DWIDGET_USE_NAMESPACE

namespace {
const int RECORD_MIN_SIZE = 10;
const int SPACING = 10;
const int TOOLBAR_X_SPACING = 70;
const int TOOLBAR_Y_SPACING = 8;
const int CURSOR_WIDTH = 8;
const int CURSOR_HEIGHT = 18;
const int INDICATOR_WIDTH =  110;
const qreal RESIZEPOINT_WIDTH = 15;
}

DWM_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    m_wmHelper(DWindowManagerHelper::instance())
{
    initAttributes();

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &MainWindow::compositeChanged);
}

void MainWindow::initAttributes()
{
    // Init attributes.
    setWindowTitle(tr("Deepin screen recorder"));
    m_functionType = 0;
    m_keyBoardStatus = 0;
    m_multiKeyButtonsInOnSec = false;
    m_keyBoardTimer = new QTimer(this);
    m_keyButtonList.clear();
    m_tempkeyButtonList.clear();

    // Add Qt::WindowDoesNotAcceptFocus make window not accept focus forcely, avoid conflict with dde hot-corner.
//    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus | Qt::X11BypassWindowManagerHint);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);   // make MouseMove can response
    installEventFilter(this);  // add event filter

    isFirstDrag = false;
    isFirstMove = false;
    isFirstPressButton = false;
    isFirstReleaseButton = false;
    dragStartX = 0;
    dragStartY = 0;

    isPressButton = false;
    isReleaseButton = false;

    recordX = 0;
    recordY = 0;
    recordWidth = 0;
    recordHeight = 0;

    dragRecordX = -1;
    dragRecordY = -1;

    drawDragPoint = false;

    recordButtonStatus = RECORD_BUTTON_NORMAL;

    flashTrayIconCounter = 0;

    selectAreaName = "";

    createWinId();

    // Get all windows geometry.
    // Below code must execute before `window.showFullscreen,
    // otherwise deepin-screen-recorder window will add in window lists.
    QPoint pos = this->cursor().pos();
    const qreal ratio = devicePixelRatioF();
    DScreenWindowsUtil* screenWin = DScreenWindowsUtil::instance(pos);
    screenRect = screenWin->backgroundRect();
    screenRect = QRect(screenRect.topLeft() / ratio, screenRect.size());
    this->move(static_cast<int>(screenRect.x() * ratio),
               static_cast<int>(screenRect.y() * ratio));
    this->setFixedSize(screenRect.width(), screenRect.height());

    windowManager = new DWindowManager();
    windowManager->setRootWindowRect(screenRect);
    QList<xcb_window_t> windows = windowManager->getWindows();
    rootWindowRect = windowManager->getRootWindowRect();

    for (auto wid : DWindowManagerHelper::instance()->currentWorkspaceWindowIdList()) {
        if (wid == winId()) continue;

        DForeignWindow * window = DForeignWindow::fromWinId(wid);
        if (window) {
            window->deleteLater();
            windowRects << Dtk::Wm::WindowRect { window->x(), window->y(), window->width(), window->height() };
            windowNames << window->wmClass();
        }
    }

    recordButtonLayout = new QVBoxLayout();
    setLayout(recordButtonLayout);

    //屏蔽原有的录屏提示控件 by zyg
//    startTooltip = new StartTooltip();
//    startTooltip->setWindowManager(windowManager);

    //构建截屏工具栏按钮 by zyg
    m_toolBar = new ToolBar(this);
    m_toolBar->hide();



    countdownTooltip = new CountdownTooltip();
    connect(countdownTooltip, SIGNAL(finished()), this, SLOT(startRecord()));

    connect(m_toolBar, &ToolBar::currentFunctionToMain, this, &MainWindow::changeFunctionButton);
    connect(m_toolBar, &ToolBar::keyBoardCheckedToMain, this, &MainWindow::changeKeyBoardShowEvent);
    connect(m_toolBar, &ToolBar::microphoneActionCheckedToMain, this, &MainWindow::changeMicrophoneSelectEvent);
    connect(m_toolBar, &ToolBar::systemAudioActionCheckedToMain, this, &MainWindow::changeSystemAudioSelectEvent);
    //构建截屏录屏功能触发按钮
    m_recordButton = new QPushButton(this);
    m_recordButton->setFixedSize(60, 47);
    m_recordButton->setText(tr("Record"));
    m_recordButton->setObjectName("mainRecordBtn");

    m_shotButton = new QPushButton(this);
    m_shotButton->setFixedSize(60, 47);
    m_shotButton->setText(tr("Shot"));
    m_shotButton->setObjectName("mainShotBtn");

    m_recordButton->hide();
    m_shotButton->hide();

    QPoint curPos = this->cursor().pos();
    m_swUtil = DScreenWindowsUtil::instance(curPos);
    m_backgroundRect = m_swUtil->backgroundRect();
    m_backgroundRect = QRect(m_backgroundRect.topLeft() / ratio, m_backgroundRect.size());

    recordButton = new RecordButton();
    recordButton->setText(tr("Start recording"));
//    connect(recordButton, SIGNAL(clicked()), this, SLOT(startCountdown()));
    connect(m_recordButton, SIGNAL(clicked()), this, SLOT(startCountdown()));

    recordOptionPanel = new RecordOptionPanel();

    recordOptionPanel->setFixedWidth(recordButton->width());

    recordButtonLayout->addStretch();
    recordButtonLayout->addWidget(recordButton, 0, Qt::AlignCenter);
    recordButtonLayout->addSpacing(RECORD_OPTIONAL_PADDING);
    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
        recordButtonLayout->addWidget(recordOptionPanel, 0, Qt::AlignCenter);
    }
    recordButtonLayout->addStretch();

    recordButton->hide();
    recordOptionPanel->hide();

    m_selectedMic = true;
    m_selectedSystemAudio = false;

    // Just use for debug.
    // repaintCounter = 0;
}

void MainWindow::initResource()
{
    m_showButtons = new ShowButtons(this);
    connect(m_showButtons, SIGNAL(keyShowSignal(const QString&)),
            this, SLOT(showKeyBoardButtons(const QString&)));
    resizeHandleBigImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("resize_handle_big.svg"));
    resizeHandleSmallImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("resize_handle_small.svg"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon1.svg"))));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    setDragCursor();

    buttonFeedback = new ButtonFeedback();

    connect(&eventMonitor, SIGNAL(buttonedPress(int, int)), this, SLOT(showPressFeedback(int, int)), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(buttonedDrag(int, int)), this, SLOT(showDragFeedback(int, int)), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(buttonedRelease(int, int)), this, SLOT(showReleaseFeedback(int, int)), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(pressEsc()), this, SLOT(responseEsc()), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(pressKeyButton(unsigned char)), m_showButtons,
            SLOT(showContentButtons(unsigned char)), Qt::QueuedConnection);
    connect(&eventMonitor, SIGNAL(releaseKeyButton(unsigned char)), m_showButtons,
            SLOT(releaseContentButtons(unsigned char)), Qt::QueuedConnection);
    eventMonitor.start();

//    startTooltip->show();
//    startTooltip->windowHandle()->raise();
}

void MainWindow::showPressFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
        buttonFeedback->showPressFeedback(x, y);
    }
}

void MainWindow::showDragFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
        buttonFeedback->showDragFeedback(x, y);
    }
}

void MainWindow::showReleaseFeedback(int x, int y)
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
        buttonFeedback->showReleaseFeedback(x, y);
    }
}

void MainWindow::responseEsc()
{
    if (recordButtonStatus != RECORD_BUTTON_RECORDING) {
        QApplication::quit();
    }
}

void MainWindow::compositeChanged()
{
    if (!m_wmHelper->hasComposite()) {
        Utils::warnNoComposite();
        QApplication::quit();
    }
}

void MainWindow::updateToolBarPos()
{
    QPoint toolbarPoint;
    toolbarPoint = QPoint(recordX + recordWidth - m_toolBar->width() - TOOLBAR_X_SPACING,
                          std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));

    if (m_toolBar->width() > recordX + recordWidth) {
        toolbarPoint.setX(recordX + TOOLBAR_X_SPACING);
    }
    if (toolbarPoint.y()>= m_backgroundRect.y() + m_backgroundRect.height()
            - m_toolBar->height() - 28) {
        if (recordY > 28*2 + 10) {
            toolbarPoint.setY(recordY - m_toolBar->height() - TOOLBAR_Y_SPACING);
        } else {
            toolbarPoint.setY(recordY + TOOLBAR_Y_SPACING);
        }
    }
    m_toolBar->showAt(toolbarPoint);
}

void MainWindow::updateRecordButtonPos()
{
    QPoint recordButtonBarPoint;
    recordButtonBarPoint = QPoint(recordX + recordWidth - m_shotButton->width(),
                          std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));

    if (m_recordButton->width() > recordX + recordWidth) {
        recordButtonBarPoint.setX(recordX + TOOLBAR_X_SPACING);
    }
    if (recordButtonBarPoint.y()>= m_backgroundRect.y() + m_backgroundRect.height()
            - m_shotButton->height() - 28) {
        if (recordY > 28*2 + 10) {
            recordButtonBarPoint.setY(recordY - m_shotButton->height() - TOOLBAR_Y_SPACING);
        } else {
            recordButtonBarPoint.setY(recordY + TOOLBAR_Y_SPACING);
        }
    }

//    if (!m_recordButton->isVisible())
//    {
//        m_recordButton->show();
//    }
    if(m_functionType == 0)
    {
        if (!m_recordButton->isVisible())
        {
            m_recordButton->show();
        }
    }

    m_recordButton->move(recordButtonBarPoint.x(), recordButtonBarPoint.y());
}

void MainWindow::updateShotButtonPos()
{
    QPoint shotButtonBarPoint;
    shotButtonBarPoint = QPoint(recordX + recordWidth - m_shotButton->width(),
                          std::max(recordY + recordHeight + TOOLBAR_Y_SPACING, 0));

    if (m_shotButton->width() > recordX + recordWidth) {
        shotButtonBarPoint.setX(recordX + TOOLBAR_X_SPACING);
    }
    if (shotButtonBarPoint.y()>= m_backgroundRect.y() + m_backgroundRect.height()
            - m_shotButton->height() - 28) {
        if (recordY > 28*2 + 10) {
            shotButtonBarPoint.setY(recordY - m_shotButton->height() - TOOLBAR_Y_SPACING);
        } else {
            shotButtonBarPoint.setY(recordY + TOOLBAR_Y_SPACING);
        }
    }

//    if (!m_shotButton->isVisible())
//    {
//        m_shotButton->show();
//    }
    if(m_functionType == 1)
    {
        if (!m_shotButton->isVisible())
        {
            m_shotButton->show();
        }
    }
    m_shotButton->move(shotButtonBarPoint.x(), shotButtonBarPoint.y());
}

void MainWindow::changeFunctionButton(QString type)
{
    if(type == "record")
    {
        m_shotButton->hide();
//        updateRecordButtonPos();
        m_recordButton->show();
        m_functionType = 0;
    }

    else if(type == "shot")
    {
        m_recordButton->hide();
//        updateShotButtonPos();
        m_shotButton->show();
        m_functionType = 1;
    }
}

void MainWindow::showKeyBoardButtons(const QString &key)
{
    //键盘按钮启用状态下创建按键控件
    if(m_keyBoardStatus == 1)
    {
        connect(m_keyBoardTimer, SIGNAL(timeout()), this, SLOT(showMultiKeyBoardButtons()));


        if(m_keyBoardTimer->isActive())
        {
            qDebug()<<"timer is actived!";
        }

        else
        {
            qDebug()<<"timer start!";
            m_keyBoardTimer->start(1000);
        }

        KeyButtonWidget *t_keyWidget = new KeyButtonWidget(this);
        t_keyWidget->setKeyLabelWord(key);
//        m_tempkeyButtonList.append(t_keyWidget);

//        QPoint t_keyPoint;
//        t_keyPoint = QPoint(recordX + recordWidth/2 - t_keyWidget->width()/2,
//                              std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));

//        t_keyWidget->show();

        if(m_multiKeyButtonsInOnSec == true)
        {
            m_keyButtonList.append(t_keyWidget);

            if(m_keyButtonList.count() > 5)
            {
                delete m_keyButtonList.first();
                m_keyButtonList.pop_front();
            }
            qDebug() << "aaa key count:" << m_keyButtonList.count();
            //更新多按钮的位置
            updateMultiKeyBoardPos();
        }

        else
        {
            if(!m_keyButtonList.isEmpty())
            {
                qDeleteAll(m_keyButtonList);
                m_keyButtonList.clear();
            }

            m_keyButtonList.append(t_keyWidget);

            if(m_keyButtonList.count() > 5)
            {
                delete m_keyButtonList.first();
                m_keyButtonList.pop_front();
            }
            qDebug() << "bbb key count:" << m_keyButtonList.count();
            //更新多按钮的位置
            updateMultiKeyBoardPos();
            m_multiKeyButtonsInOnSec = true;

        }
//        t_keyWidget->move(t_keyPoint.x(), t_keyPoint.y());

        repaint();
    }
}

void MainWindow::changeKeyBoardShowEvent(bool checked)
{
    qDebug() << checked;

    if(checked == false)
    {
        m_keyBoardStatus = 0;
        if(m_keyButtonList.count() > 0)
        {
            for(int t_index = 0; t_index < m_keyButtonList.count(); t_index++)
            {
                m_keyButtonList.at(t_index)->hide();
            }
        }
    }

    else if(checked == true)
    {
        m_keyBoardStatus = 1;
    }
}
void MainWindow::changeMicrophoneSelectEvent(bool checked)
{
    m_selectedMic = checked;
}
void MainWindow::changeSystemAudioSelectEvent(bool checked)
{
    m_selectedSystemAudio = checked;
    AudioUtils* audioUtils = new AudioUtils(this);
    checked ? audioUtils->setupSystemAudioOutput() : audioUtils->setupMicrophoneOutput();
}


void MainWindow::showMultiKeyBoardButtons()
{
//    m_keyBoardTimer->stop();
//    if(!m_keyButtonList.isEmpty())
//    {
//        m_keyButtonList.clear();
//    }

//    m_keyButtonList.append(m_tempkeyButtonList);
//    m_keyButtonList = m_tempkeyButtonList;

//    if(!m_keyButtonList.isEmpty())
//    {
//        for(int t_index = 0; t_index < m_keyButtonList.count(); t_index++)
//        {
//            QPoint t_keyPoint;
//            t_keyPoint = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(t_index)->width()/2,
//                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
//            m_keyButtonList.at(t_index)->show();

//            m_keyButtonList.at(t_index)->move(t_keyPoint.x(), t_keyPoint.y());

//            repaint();
//        }
//    }

//    return;

    m_multiKeyButtonsInOnSec = false;
}

void MainWindow::updateMultiKeyBoardPos()
{
    QPoint t_keyPoint1;
    QPoint t_keyPoint2;
    QPoint t_keyPoint3;
    QPoint t_keyPoint4;
    QPoint t_keyPoint5;

    if(!m_keyButtonList.isEmpty())
    {
        switch (m_keyButtonList.count())
        {
            //一个按键的情况
            case 1:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(0)->width()/2,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();
            break;
            //两个按键的情况
            case 2:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(0)->width()/2 - m_keyButtonList.at(0)->width()/1.5,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(1)->width()/2 + m_keyButtonList.at(1)->width()/1.5,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();
            break;
            //三个按键的情况
            case 3:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(0)->width()/2 - m_keyButtonList.at(0)->width()*1.3,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(1)->width()/2,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(2)->width()/2 + m_keyButtonList.at(2)->width()*1.3,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();
            break;
            //四个按键的情况
            case 4:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(0)->width()/2 - m_keyButtonList.at(0)->width()*2,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(1)->width()/2 - m_keyButtonList.at(1)->width()/1.5,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(2)->width()/2 + m_keyButtonList.at(2)->width()/1.5,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(3)->width()/2 + m_keyButtonList.at(3)->width()*2,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();
            break;
            //五个按键的情况
            case 5:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(0)->width()/2 - m_keyButtonList.at(0)->width()*2.6,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(1)->width()/2 - m_keyButtonList.at(1)->width()*1.3,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(2)->width()/2,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(3)->width()/2 + m_keyButtonList.at(3)->width()*1.3,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();

            m_keyButtonList.at(4)->hide();
            t_keyPoint5 = QPoint(recordX + recordWidth/2 - m_keyButtonList.at(3)->width()/2 + m_keyButtonList.at(4)->width()*2.6,
                                  std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(4)->move(t_keyPoint5.x(), t_keyPoint5.y());
            m_keyButtonList.at(4)->show();
            break;
            default:
            break;
        }
    }

}

void MainWindow::paintEvent(QPaintEvent *)
{
    // Just use for debug.
    // repaintCounter++;
    // qDebug() << repaintCounter;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    if (recordWidth > 0 && recordHeight > 0) {

        QRect backgroundRect = QRect(0, 0, rootWindowRect.width, rootWindowRect.height);
        QRect frameRect = QRect(recordX, recordY, recordWidth, recordHeight);
        
        // Draw background.
        painter.setBrush(QBrush("#000000"));
        painter.setOpacity(0.2);

        painter.setClipping(true);
        painter.setClipRegion(QRegion(backgroundRect).subtracted(QRegion(frameRect)));
        painter.drawRect(backgroundRect);

        // Reset clip.
        painter.setClipRegion(QRegion(backgroundRect));

        // Draw frame.
        if (recordButtonStatus != RECORD_BUTTON_RECORDING) {
            painter.setRenderHint(QPainter::Antialiasing, false);
            QPen framePen(QColor("#01bdff"));
            framePen.setWidth(2);
            painter.setOpacity(1);
            painter.setBrush(QBrush());  // clear brush
            painter.setPen(framePen);
            painter.drawRect(QRect(
                                 std::max(frameRect.x(), 1),
                                 std::max(frameRect.y(), 1),
                                 std::min(frameRect.width() - 1, rootWindowRect.width - 2),
                                 std::min(frameRect.height() - 1, rootWindowRect.height - 2)));
            painter.setRenderHint(QPainter::Antialiasing, true);
        }

        // Draw drag pint.
        if (recordButtonStatus == RECORD_BUTTON_NORMAL && drawDragPoint) {
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS), resizeHandleBigImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleBigImg);

            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth, recordY - DRAG_POINT_RADIUS + recordHeight / 2), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS), resizeHandleSmallImg);
            painter.drawPixmap(QPoint(recordX - DRAG_POINT_RADIUS + recordWidth / 2, recordY - DRAG_POINT_RADIUS + recordHeight), resizeHandleSmallImg);
        }

        // Draw record panel.
        if (isFirstPressButton) {
            if (isFirstReleaseButton) {
                if (recordButtonStatus == RECORD_BUTTON_NORMAL && recordButton->isVisible()) {
                    QList<QRectF> rects;
                    rects << recordButton->geometry();

                    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
                        rects << recordOptionPanel->geometry();
                    }
                    Utils::blurRects(windowManager, this->winId(), rects);
                } else if (recordButtonStatus == RECORD_BUTTON_WAIT) {
                    QList<QRectF> rects;
                    rects << countdownTooltip->geometry();
                    Utils::blurRects(windowManager, this->winId(), rects);
                }
            }
        }
    }
}

bool MainWindow::eventFilter(QObject *, QEvent *event)
{
    bool needRepaint = false;

#undef KeyPress
#undef KeyRelease
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
            if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                if (keyEvent->key() == Qt::Key_Left) {
                    recordX = std::max(0, recordX - 1);
                    recordWidth = std::min(recordWidth + 1, rootWindowRect.width);

                    needRepaint = true;
                } else if (keyEvent->key() == Qt::Key_Right) {
                    recordWidth = std::min(recordWidth + 1, rootWindowRect.width);

                    needRepaint = true;
                } else if (keyEvent->key() == Qt::Key_Up) {
                    recordY = std::max(0, recordY - 1);
                    recordHeight = std::min(recordHeight + 1, rootWindowRect.height);

                    needRepaint = true;
                } else if (keyEvent->key() == Qt::Key_Down) {
                    recordHeight = std::min(recordHeight + 1, rootWindowRect.height);

                    needRepaint = true;
                }
            } else {
                if (keyEvent->key() == Qt::Key_Left) {
                    recordX = std::max(0, recordX - 1);

                    needRepaint = true;
                } else if (keyEvent->key() == Qt::Key_Right) {
                    recordX = std::min(rootWindowRect.width - recordWidth, recordX + 1);

                    needRepaint = true;
                } else if (keyEvent->key() == Qt::Key_Up) {
                    recordY = std::max(0, recordY - 1);

                    needRepaint = true;
                } else if (keyEvent->key() == Qt::Key_Down) {
                    recordY = std::min(rootWindowRect.height - recordHeight, recordY + 1);

                    needRepaint = true;
                }
            }

            if (recordButtonStatus == RECORD_BUTTON_NORMAL && needRepaint) {
                hideRecordButton();
            }
        }
    } else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // NOTE: must be use 'isAutoRepeat' to filter KeyRelease event send by Qt.
        if (!keyEvent->isAutoRepeat()) {
            if (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right || keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
                needRepaint = true;
            }

            if (recordButtonStatus == RECORD_BUTTON_NORMAL && needRepaint) {
                showRecordButton();
                updateToolBarPos();
                updateRecordButtonPos();
                updateShotButtonPos();
            }
        }

    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        dragStartX = mouseEvent->x();
        dragStartY = mouseEvent->y();
        if (!isFirstPressButton) {
            isFirstPressButton = true;

//            startTooltip->hide();
//            m_toolBar->hide();

            Utils::clearBlur(windowManager, this->winId());
        } else {
            dragAction = getAction(event);

            dragRecordX = recordX;
            dragRecordY = recordY;
            dragRecordWidth = recordWidth;
            dragRecordHeight = recordHeight;

            if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                hideRecordButton();
                m_toolBar->hide();
                m_recordButton->hide();
                m_shotButton->hide();

                //隐藏键盘按钮控件
                if(m_keyButtonList.count() > 0)
                {
                    for(int t_index = 0; t_index < m_keyButtonList.count(); t_index++)
                    {
                        m_keyButtonList.at(t_index)->hide();
                    }
                }
            }
        }

        isPressButton = true;
        isReleaseButton = false;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (!isFirstReleaseButton) {
            isFirstReleaseButton = true;

            updateCursor(event);
            updateToolBarPos();
            updateRecordButtonPos();
            updateShotButtonPos();

            // Record select area name with window name if just click (no drag).
            if (!isFirstDrag) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                for (auto it = windowRects.rbegin(); it != windowRects.rend(); ++it) {
                    if (QRect(it->x, it->y, it->width, it->height).contains(mouseEvent->pos() + screenRect.topLeft())) {
                        selectAreaName = windowNames[windowRects.rend() - it - 1];
                        break;
                    }
                }
            } else {
                // Make sure record area not too small.
                recordWidth = recordWidth < RECORD_MIN_SIZE ? RECORD_MIN_SIZE : recordWidth;
                recordHeight = recordHeight < RECORD_MIN_SIZE ? RECORD_MIN_SIZE : recordHeight;

                if (recordX + recordWidth > rootWindowRect.width) {
                    recordX = rootWindowRect.width - recordWidth;
                }

                if (recordY + recordHeight > rootWindowRect.height) {
                    recordY = rootWindowRect.height - recordHeight;
                }
            }

            showRecordButton();
            updateToolBarPos();
            updateRecordButtonPos();
            updateShotButtonPos();

            needRepaint = true;
        } else {
            if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                showRecordButton();
                updateToolBarPos();
                updateRecordButtonPos();
                updateShotButtonPos();
            }
        }

        isPressButton = false;
        isReleaseButton = true;

        needRepaint = true;
    } else if (event->type() == QEvent::MouseMove) {
        if (!isFirstMove) {
            isFirstMove = true;
        }

        if (isPressButton && isFirstPressButton) {
            if (!isFirstDrag) {
                isFirstDrag = true;

                selectAreaName = tr("Select area");
            }
        }

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (isFirstPressButton) {
            if (!isFirstReleaseButton) {
                if (isPressButton && !isReleaseButton) {
                    recordX = std::min(dragStartX, mouseEvent->x());
                    recordY = std::min(dragStartY, mouseEvent->y());
                    recordWidth = std::abs(dragStartX - mouseEvent->x());
                    recordHeight = std::abs(dragStartY - mouseEvent->y());

                    needRepaint = true;
                }
            } else if (isPressButton) {
                if (recordButtonStatus == RECORD_BUTTON_NORMAL && dragRecordX >= 0 && dragRecordY >= 0) {
                    if (dragAction == ACTION_MOVE) {
                        recordX = std::max(std::min(dragRecordX + mouseEvent->x() - dragStartX, rootWindowRect.width - recordWidth), 1);
                        recordY = std::max(std::min(dragRecordY + mouseEvent->y() - dragStartY, rootWindowRect.height - recordHeight), 1);
                    } else if (dragAction == ACTION_RESIZE_TOP_LEFT) {
                        resizeTop(mouseEvent);
                        resizeLeft(mouseEvent);
                    } else if (dragAction == ACTION_RESIZE_TOP_RIGHT) {
                        resizeTop(mouseEvent);
                        resizeRight(mouseEvent);
                    } else if (dragAction == ACTION_RESIZE_BOTTOM_LEFT) {
                        resizeBottom(mouseEvent);
                        resizeLeft(mouseEvent);
                    } else if (dragAction == ACTION_RESIZE_BOTTOM_RIGHT) {
                        resizeBottom(mouseEvent);
                        resizeRight(mouseEvent);
                    } else if (dragAction == ACTION_RESIZE_TOP) {
                        resizeTop(mouseEvent);
                    } else if (dragAction == ACTION_RESIZE_BOTTOM) {
                        resizeBottom(mouseEvent);
                    } else if (dragAction == ACTION_RESIZE_LEFT) {
                        resizeLeft(mouseEvent);
                    } else if (dragAction == ACTION_RESIZE_RIGHT) {
                        resizeRight(mouseEvent);
                    }

                    needRepaint = true;
                }
            }

            updateCursor(event);

            int action = getAction(event);
            bool drawPoint = action != ACTION_MOVE;
            if (drawPoint != drawDragPoint) {
                drawDragPoint = drawPoint;
                needRepaint = true;
            }
        } else {
            // Select the first window where the mouse is located
            const qreal ratio = devicePixelRatioF();
            const QPoint mousePoint = QCursor::pos();
            for (auto it = windowRects.rbegin(); it != windowRects.rend(); ++it) {
                if (QRect(it->x, it->y, it->width, it->height).contains(mousePoint)) {
                    recordX = it->x - static_cast<int>(screenRect.x() * ratio);
                    recordY = it->y - static_cast<int>(screenRect.y() * ratio);
                    recordWidth = it->width;
                    recordHeight = it->height;
                    needRepaint = true;
                    break;
                }
            }
        }
    }

    // Use flag instead call `repaint` directly,
    // to avoid repaint many times in one event function.
    if (needRepaint) {
        repaint();
    }

    return false;
}

void MainWindow::startRecord()
{
    Utils::clearBlur(windowManager, this->winId());
    recordButtonStatus = RECORD_BUTTON_RECORDING;

    resetCursor();

    repaint();

    trayIcon->show();

    flashTrayIconTimer = new QTimer(this);
    connect(flashTrayIconTimer, SIGNAL(timeout()), this, SLOT(flashTrayIcon()));
    flashTrayIconTimer->start(800);

    recordProcess.setRecordAudioInputType(getRecordInputType(m_selectedMic, m_selectedSystemAudio));
    recordProcess.startRecord();
//    voiceRecordProcess.startRecord();
}

void MainWindow::flashTrayIcon()
{
    if (flashTrayIconCounter % 2 == 0) {
        trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon2.svg"))));
    } else {
        trayIcon->setIcon(QIcon((Utils::getQrcPath("trayicon1.svg"))));
    }

    flashTrayIconCounter++;

    if (flashTrayIconCounter > 10) {
        flashTrayIconCounter = 1;
    }
}

void MainWindow::resizeTop(QMouseEvent *mouseEvent)
{
    int offsetY = mouseEvent->y() - dragStartY;
    recordY = std::max(std::min(dragRecordY + offsetY, dragRecordY + dragRecordHeight - RECORD_MIN_SIZE), 1);
    recordHeight = std::max(std::min(dragRecordHeight - offsetY, rootWindowRect.height), RECORD_MIN_SIZE);
}

void MainWindow::resizeBottom(QMouseEvent *mouseEvent)
{
    int offsetY = mouseEvent->y() - dragStartY;
    recordHeight = std::max(std::min(dragRecordHeight + offsetY, rootWindowRect.height), RECORD_MIN_SIZE);
}

void MainWindow::resizeLeft(QMouseEvent *mouseEvent)
{
    int offsetX = mouseEvent->x() - dragStartX;
    recordX = std::max(std::min(dragRecordX + offsetX, dragRecordX + dragRecordWidth - RECORD_MIN_SIZE), 1);
    recordWidth = std::max(std::min(dragRecordWidth - offsetX, rootWindowRect.width), RECORD_MIN_SIZE);
}

void MainWindow::resizeRight(QMouseEvent *mouseEvent)
{
    int offsetX = mouseEvent->x() - dragStartX;
    recordWidth = std::max(std::min(dragRecordWidth + offsetX, rootWindowRect.width), RECORD_MIN_SIZE);
}

int MainWindow::getAction(QEvent *event) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    int cursorX = mouseEvent->x();
    int cursorY = mouseEvent->y();

    if (cursorX > recordX - CURSOR_BOUND
        && cursorX < recordX + CURSOR_BOUND
        && cursorY > recordY - CURSOR_BOUND
        && cursorY < recordY + CURSOR_BOUND) {
        // Top-Left corner.
        return ACTION_RESIZE_TOP_LEFT;
    } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
               && cursorX < recordX + recordWidth + CURSOR_BOUND
               && cursorY > recordY + recordHeight - CURSOR_BOUND
               && cursorY < recordY + recordHeight + CURSOR_BOUND) {
        // Bottom-Right corner.
        return ACTION_RESIZE_BOTTOM_RIGHT;
    } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
               && cursorX < recordX + recordWidth + CURSOR_BOUND
               && cursorY > recordY - CURSOR_BOUND
               && cursorY < recordY + CURSOR_BOUND) {
        // Top-Right corner.
        return ACTION_RESIZE_TOP_RIGHT;
    } else if (cursorX > recordX - CURSOR_BOUND
               && cursorX < recordX + CURSOR_BOUND
               && cursorY > recordY + recordHeight - CURSOR_BOUND
               && cursorY < recordY + recordHeight + CURSOR_BOUND) {
        // Bottom-Left corner.
        return ACTION_RESIZE_BOTTOM_LEFT;
    } else if (cursorX > recordX - CURSOR_BOUND
               && cursorX < recordX + CURSOR_BOUND) {
        // Left.
        return ACTION_RESIZE_LEFT;
    } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
               && cursorX < recordX + recordWidth + CURSOR_BOUND) {
        // Right.
        return ACTION_RESIZE_RIGHT;
    } else if (cursorY > recordY - CURSOR_BOUND
               && cursorY < recordY + CURSOR_BOUND) {
        // Top.
        return ACTION_RESIZE_TOP;
    } else if (cursorY > recordY + recordHeight - CURSOR_BOUND
               && cursorY < recordY + recordHeight + CURSOR_BOUND) {
        // Bottom.
        return ACTION_RESIZE_BOTTOM;
    } else {
        return ACTION_MOVE;
    }
}

void MainWindow::updateCursor(QEvent *event)
{
    if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int cursorX = mouseEvent->x();
        int cursorY = mouseEvent->y();

        if (cursorX > recordX - CURSOR_BOUND
            && cursorX < recordX + CURSOR_BOUND
            && cursorY > recordY - CURSOR_BOUND
            && cursorY < recordY + CURSOR_BOUND) {
            // Top-Left corner.
            QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
        } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
                   && cursorX < recordX + recordWidth + CURSOR_BOUND
                   && cursorY > recordY + recordHeight - CURSOR_BOUND
                   && cursorY < recordY + recordHeight + CURSOR_BOUND) {
            // Bottom-Right corner.
            QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
        } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
                   && cursorX < recordX + recordWidth + CURSOR_BOUND
                   && cursorY > recordY - CURSOR_BOUND
                   && cursorY < recordY + CURSOR_BOUND) {
            // Top-Right corner.
            QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
        } else if (cursorX > recordX - CURSOR_BOUND
                   && cursorX < recordX + CURSOR_BOUND
                   && cursorY > recordY + recordHeight - CURSOR_BOUND
                   && cursorY < recordY + recordHeight + CURSOR_BOUND) {
            // Bottom-Left corner.
            QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
        } else if (cursorX > recordX - CURSOR_BOUND
                   && cursorX < recordX + CURSOR_BOUND) {
            // Left.
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
        } else if (cursorX > recordX + recordWidth - CURSOR_BOUND
                   && cursorX < recordX + recordWidth + CURSOR_BOUND) {
            // Right.
            QApplication::setOverrideCursor(Qt::SizeHorCursor);
        } else if (cursorY > recordY - CURSOR_BOUND
                   && cursorY < recordY + CURSOR_BOUND) {
            // Top.
            QApplication::setOverrideCursor(Qt::SizeVerCursor);
        } else if (cursorY > recordY + recordHeight - CURSOR_BOUND
                   && cursorY < recordY + recordHeight + CURSOR_BOUND) {
            // Bottom.
            QApplication::setOverrideCursor(Qt::SizeVerCursor);
        } else if (recordButton->geometry().contains(cursorX, cursorY) || recordOptionPanel->geometry().contains(cursorX, cursorY)) {
            // Record area.
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        } else {
            if (isPressButton) {
                QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            } else {
                QApplication::setOverrideCursor(Qt::OpenHandCursor);
            }
        }
    }
}

void MainWindow::setDragCursor()
{
    QApplication::setOverrideCursor(Qt::CrossCursor);
}

void MainWindow::resetCursor()
{
    QApplication::overrideCursor();
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason)
{
    stopRecord();
}

void MainWindow::stopRecord()
{
    if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
        hide();
        recordProcess.stopRecord();
//        voiceRecordProcess.stopRecord();
    }
}

void MainWindow::startCountdown()
{
    recordButtonStatus = RECORD_BUTTON_WAIT;

    const qreal ratio = devicePixelRatioF();
    const QPoint topLeft = geometry().topLeft();

    QRect recordRect {
        static_cast<int>(recordX * ratio + topLeft.x()),
        static_cast<int>(recordY * ratio + topLeft.y()),
        static_cast<int>(recordWidth * ratio),
        static_cast<int>(recordHeight * ratio)
    };

    recordProcess.setRecordInfo(recordRect, selectAreaName);
    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
        if (recordOptionPanel->isSaveAsGif()) {
            recordProcess.setRecordType(RecordProcess::RECORD_TYPE_GIF);
        } else {
            recordProcess.setRecordType(RecordProcess::RECORD_TYPE_VIDEO);
        }
    } else {
        recordProcess.setRecordType(RecordProcess::RECORD_TYPE_GIF);
    }

    resetCursor();

//    hideRecordButton();
    //隐藏所有的控件
    hideAllWidget();


    delete recordButtonLayout;

    countdownLayout = new QVBoxLayout();
    setLayout(countdownLayout);

    countdownLayout->addStretch();
    countdownLayout->addWidget(countdownTooltip, 0, Qt::AlignCenter);
    countdownLayout->addStretch();
    adjustLayout(countdownLayout, countdownTooltip->rect().width(), countdownTooltip->rect().height());

    countdownTooltip->start();


    Utils::passInputEvent(this->winId());

    repaint();
}

void MainWindow::showRecordButton()
{
//    updateToolBarPos();
//    recordButton->show();
    if (QSysInfo::currentCpuArchitecture().startsWith("x86")) {
//        recordOptionPanel->show();

        adjustLayout(recordButtonLayout,
                     recordButton->width(),
                     recordButton->height() + RECORD_OPTIONAL_PADDING + recordOptionPanel->height());
    } else {
        adjustLayout(recordButtonLayout,
                     recordButton->width(),
                     recordButton->height() + RECORD_OPTIONAL_PADDING);
    }
}

void MainWindow::hideRecordButton()
{
    recordButton->hide();
    recordOptionPanel->hide();

    Utils::clearBlur(windowManager, this->winId());
}

void MainWindow::hideAllWidget()
{
    m_toolBar->hide();
    m_recordButton->hide();
    m_shotButton->hide();

    //隐藏键盘按钮控件
    if(m_keyButtonList.count() > 0)
    {
        for(int t_index = 0; t_index < m_keyButtonList.count(); t_index++)
        {
            m_keyButtonList.at(t_index)->hide();
        }
    }

    Utils::clearBlur(windowManager, this->winId());
}

void MainWindow::adjustLayout(QVBoxLayout *layout, int layoutWidth, int layoutHeight)
{
    if (recordHeight < layoutHeight) {
        if (recordY + layoutHeight > rootWindowRect.height) {
            layout->setContentsMargins(
                recordX,
                recordY - layoutHeight - Constant::RECTANGLE_PADDING,
                rootWindowRect.width - recordX - recordWidth,
                rootWindowRect.height - recordY + Constant::RECTANGLE_PADDING);
        } else {
            layout->setContentsMargins(
                recordX,
                recordY + recordHeight + Constant::RECTANGLE_PADDING,
                rootWindowRect.width - recordX - recordWidth,
                rootWindowRect.height - (recordY + recordHeight + layoutHeight) + Constant::RECTANGLE_PADDING);
        }
    } else if (recordWidth < layoutWidth) {
        if (recordX + layoutWidth > rootWindowRect.width) {
            layout->setContentsMargins(
                recordX - layoutWidth - Constant::RECTANGLE_PADDING,
                recordY,
                rootWindowRect.width - recordX + Constant::RECTANGLE_PADDING,
                rootWindowRect.height - recordY - recordHeight);
        } else {
            layout->setContentsMargins(
                recordX + recordWidth + Constant::RECTANGLE_PADDING,
                recordY,
                rootWindowRect.width - (recordX + recordWidth + layoutWidth + Constant::RECTANGLE_PADDING),
                rootWindowRect.height - recordY - recordHeight);
        }
    } else {
        layout->setContentsMargins(
            recordX,
            recordY,
            rootWindowRect.width - recordX - recordWidth,
            rootWindowRect.height - recordY - recordHeight);
    }
}

void MainWindow::initShapeWidget(QString type)
{
    qDebug() << "show shapesWidget";
    m_shapesWidget = new ShapesWidget(this);
//    m_shapesWidget->setShiftKeyPressed(m_isShiftPressed);

    if (type != "color")
        m_shapesWidget->setCurrentShape(type);

    m_shapesWidget->show();
    m_shapesWidget->setFixedSize(recordWidth - 4, recordHeight - 4);
    m_shapesWidget->move(recordX + 2, recordY + 2);

//    updateToolBarPos();
//    m_toolBar->raise();
//    m_needDrawSelectedPoint = false;
    update();

//    connect(m_toolBar, &ToolBar::updateColor,
//            m_shapesWidget, &ShapesWidget::setPenColor);
//    connect(m_shapesWidget, &ShapesWidget::reloadEffectImg,
//            this, &MainWindow::reloadImage);
//    connect(this, &MainWindow::deleteShapes, m_shapesWidget,
//            &ShapesWidget::deleteCurrentShape);
//    connect(m_shapesWidget, &ShapesWidget::requestScreenshot,
//            this, &MainWindow::saveScreenshot);
//    connect(m_shapesWidget, &ShapesWidget::shapePressed,
//            m_toolBar, &ToolBar::shapePressed);
//    connect(m_shapesWidget, &ShapesWidget::saveBtnPressed,
//            m_toolBar, &ToolBar::saveBtnPressed);
//    connect(m_shapesWidget, &ShapesWidget::requestExit, this, &MainWindow::exitApp);
//    connect(this, &MainWindow::unDo, m_shapesWidget, &ShapesWidget::undoDrawShapes);
//    connect(this, &MainWindow::saveActionTriggered,
//            m_shapesWidget, &ShapesWidget::saveActionTriggered);
    connect(m_shapesWidget, &ShapesWidget::menuNoFocus, this, &MainWindow::activateWindow);
}

int MainWindow::getRecordInputType(bool selectedMic, bool selectedSystemAudio) {
    if (selectedMic && selectedSystemAudio) {
        return RecordProcess::RECORD_AUDIO_INPUT_MIC_SYSTEMAUDIO;
    } else if (selectedMic) {
        return RecordProcess::RECORD_AUDIO_INPUT_MIC;
    } else if (selectedSystemAudio) {
        return RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO;
    }
    return 0;

}
