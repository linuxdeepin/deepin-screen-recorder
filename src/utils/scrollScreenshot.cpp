// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollScreenshot.h"
#include "utils.h"
#include "log.h"

#include <QDebug>
#include <QDateTime>
#include <QEventLoop>
#include <X11/Xlibint.h>
#include <X11/extensions/XTest.h>

ScrollScreenshot::ScrollScreenshot(QObject *parent)
    : QObject(parent)
{
    qCDebug(dsrApp) << "ScrollScreenshot initialized.";
    Q_UNUSED(parent);
    qRegisterMetaType<PixMergeThread::MergeErrorValue>("MergeErrorValue");

#if defined(KF5_WAYLAND_FLAGE_ON) && !defined(DWAYLAND_SUPPORT)
    if (Utils::isWaylandMode) {
        m_WaylandScrollMonitor = new WaylandScrollMonitor(this);  // 初始化wayland模拟滚动
        qCDebug(dsrApp) << "WaylandScrollMonitor initialized for Wayland mode.";
    }
#endif

    m_mouseWheelTimer = new QTimer(this);
    connect(m_mouseWheelTimer, &QTimer::timeout, this, [=] {
        qCDebug(dsrApp) << "Mouse wheel timer timed out.";
        if (!Utils::isWaylandMode) {
            // 发送滚轮事件， 自动滚动
            static Display *m_display = XOpenDisplay(nullptr);
            XTestFakeButtonEvent(m_display, Button5, 1, CurrentTime);
            XFlush(m_display);
            XTestFakeButtonEvent(m_display, Button5, 0, CurrentTime);
            XFlush(m_display);
            qCDebug(dsrApp) << "Simulated X11 mouse wheel scroll.";
        } else {
#ifdef KF5_WAYLAND_FLAGE_ON
#ifdef DWAYLAND_SUPPORT
            WaylandMouseSimulator::instance()->doWaylandAutoScroll();  // waland滚动
            qCDebug(dsrApp) << "Simulated Wayland auto scroll using WaylandMouseSimulator.";
#else
            m_WaylandScrollMonitor->doWaylandAutoScroll();
            qCDebug(dsrApp) << "Simulated Wayland auto scroll using WaylandScrollMonitor.";
#endif  // DWAYLAND_SUPPORT
#endif  // KF5_WAYLAND_FLAGE_ON
        }

        //当模拟鼠标进行自动滚动时，会发射此信号
        emit autoScroll(m_autoScrollFlag++);
        qCDebug(dsrApp) << "Emitted autoScroll signal.";
        // 滚动区域高度 200-300 取值2
        // 滚动区域高度 > 300  取值 3
        // 滚动区域高度 > 600  取值 5
        m_scrollCount++;
        if (m_scrollCount % m_shotFrequency == 0) {
            emit getOneImg();
        }
    });

    m_PixMerageThread = new PixMergeThread(this);
    connect(m_PixMerageThread, SIGNAL(updatePreviewImg(QImage)), this, SIGNAL(updatePreviewImg(QImage)));
    connect(m_PixMerageThread,
            SIGNAL(merageError(PixMergeThread::MergeErrorValue)),
            this,
            SLOT(merageImgState(PixMergeThread::MergeErrorValue)));
    connect(m_PixMerageThread, &PixMergeThread::invalidAreaError, this, &ScrollScreenshot::merageInvalidArea);
    if (Utils::isWaylandMode) {
#ifdef KF5_WAYLAND_FLAGE_ON
#ifdef DWAYLAND_SUPPORT
        connect(this,
                &ScrollScreenshot::sigalWheelScrolling,
                WaylandMouseSimulator::instance(),
                &WaylandMouseSimulator::slotManualScroll);
        qCDebug(dsrApp) << "Connected sigalWheelScrolling to WaylandMouseSimulator for manual scroll.";
#else
        connect(this, &ScrollScreenshot::sigalWheelScrolling, m_WaylandScrollMonitor, &WaylandScrollMonitor::slotManualScroll);
        qCDebug(dsrApp) << "Connected sigalWheelScrolling to WaylandScrollMonitor for manual scroll.";
#endif  // DWAYLAND_SUPPORT
#endif  // KF5_WAYLAND_FLAGE_ON
    }
}

ScrollScreenshot::~ScrollScreenshot()
{
    qCDebug(dsrApp) << "ScrollScreenshot destructor called.";
    if (nullptr != m_PixMerageThread) {
        m_PixMerageThread->stopTask();  //避免使用terminate
        m_PixMerageThread->wait();
        delete m_PixMerageThread;
        m_PixMerageThread = nullptr;
        qCDebug(dsrApp) << "PixMergeThread stopped, waited, and deleted.";
    }
}

void ScrollScreenshot::addPixmap(const QPixmap &piximg, int wheelDirection)
{
    qCDebug(dsrApp) << "Adding pixmap, wheelDirection:" << wheelDirection;
    if (m_startPixMerageThread == false) {
        m_PixMerageThread->start();
        m_startPixMerageThread = true;
        qCDebug(dsrApp) << "PixMergeThread started.";
    }
    m_PixMerageThread->setScrollModel(m_isManualScrollModel);
    if (m_isManualScrollModel == false) {  //自动
        qCDebug(dsrApp) << "Auto scroll mode.";
        if (m_curStatus == Wait) {
            m_mouseWheelTimer->start(300);
            m_curStatus = Merging;
            qCDebug(dsrApp) << "Started mouse wheel timer, status set to Merging.";
        }
        if (m_curStatus == Merging) {
            m_lastDirection = PixMergeThread::PictureDirection::ScrollDown;  // 记录滚动方向
            m_PixMerageThread->addShotImg(piximg, PixMergeThread::PictureDirection::ScrollDown);
            qCDebug(dsrApp) << "Added shot image for auto scroll (ScrollDown).";
        }
    } else if (m_isManualScrollModel == true) {  //手动
        qCDebug(dsrApp) << "Manual scroll mode.";
        // qCDebug(dsrApp) << "function piximg is null: " << __func__ << " ,line: " << __LINE__;
        m_mouseWheelTimer->stop();
        PixMergeThread::PictureDirection status = (wheelDirection == WheelDown) ? (PixMergeThread::PictureDirection::ScrollDown) :
                                                                                  (PixMergeThread::PictureDirection::ScrollUp);
        m_lastDirection = status;  // 记录滚动方向
        m_PixMerageThread->addShotImg(piximg, status);
        qCDebug(dsrApp) << "Added shot image for manual scroll, status:" << status;
    }
}

void ScrollScreenshot::addLastPixmap(const QPixmap &piximg)
{
    qCDebug(dsrApp) << "Adding last pixmap.";
#if (QT_VERSION_MAJOR == 5)
    setTimeAndCalculateTimeDiff(static_cast<int>(QDateTime::currentDateTime().toTime_t()));
#elif (QT_VERSION_MAJOR == 6)
    setTimeAndCalculateTimeDiff(static_cast<int>(QDateTime::currentDateTime().toSecsSinceEpoch()));
#endif
    m_PixMerageThread->setIsLastImg(true);  //添加最后一张图片标记
    if (nullptr != m_PixMerageThread) {
        m_PixMerageThread->addShotImg(piximg, m_lastDirection);
        qCDebug(dsrApp) << "Set last image flag and added shot image.";
    }
    qCDebug(dsrApp) << "Last pixmap added.";
}

void ScrollScreenshot::clearPixmap()
{
    m_PixMerageThread->clearCurImg();
    qCDebug(dsrApp) << "Cleared current pixmap.";
}

void ScrollScreenshot::changeState(const bool isStop)
{
    qCDebug(dsrApp) << "Changing state, isStop:" << isStop;
    // qCDebug(dsrApp) << __FUNCTION__ << "====" << isStop;
    //  暂停
    if (isStop && m_curStatus == Merging) {
        m_curStatus = Stop;
        m_mouseWheelTimer->stop();
        qCDebug(dsrApp) << "Paused scrolling, status set to Stop.";
    }

    // 开始
    if (!isStop && m_curStatus == Stop) {
        m_curStatus = Merging;
        m_mouseWheelTimer->start(300);
        qCDebug(dsrApp) << "Resumed scrolling, status set to Merging.";
    }
}
QImage ScrollScreenshot::savePixmap()
{
    qCDebug(dsrApp) << "Saving pixmap.";
    m_mouseWheelTimer->stop();

    QEventLoop eventloop1;  // 延迟500毫秒
    QTimer::singleShot(500, &eventloop1, SLOT(quit()));
    eventloop1.exec();
    qCDebug(dsrApp) << "Event loop executed for 500ms delay.";

    m_PixMerageThread->stopTask();
    m_PixMerageThread->wait();
    qCDebug(dsrApp) << "PixMergeThread stopped and waited.";
    return m_PixMerageThread->getMerageResult();
}

//设置滚动模式，先设置滚动模式，再添加图片
void ScrollScreenshot::setScrollModel(bool model)
{
    m_isManualScrollModel = model;
    qCDebug(dsrApp) << "Setting scroll model to:" << model;
}
//获取调整区域
QRect ScrollScreenshot::getInvalidArea()
{
    qCDebug(dsrApp) << "Returning invalid area:" << m_rect;
    return m_rect;
}
//设置时间并计算时间差
void ScrollScreenshot::setTimeAndCalculateTimeDiff(int time)
{
    qCDebug(dsrApp) << "Setting time and calculating time difference.";
    m_PixMerageThread->calculateTimeDiff(time);
}

void ScrollScreenshot::merageImgState(PixMergeThread::MergeErrorValue state)
{
    qCDebug(dsrApp) << "Merge image state received:" << state;
    qCDebug(dsrApp) << "拼接状态值:" << state;
    m_mouseWheelTimer->stop();
    if (state == PixMergeThread::MaxHeight) {
        m_curStatus = ScrollStatus::Mistake;
        qCDebug(dsrApp) << "Merge state is MaxHeight, setting status to Mistake.";
    }
    emit merageError(state);
    qCDebug(dsrApp) << "Emitted merageError signal.";
}

//调整捕捉区域
void ScrollScreenshot::merageInvalidArea(PixMergeThread::MergeErrorValue state, QRect rect)
{
    qCDebug(dsrApp) << "Merge invalid area received, state:" << state << ", rect:" << rect;
    m_rect = rect;
    m_mouseWheelTimer->stop();
    if (state == PixMergeThread::MaxHeight) {
        m_curStatus = ScrollStatus::Mistake;
        qCDebug(dsrApp) << "Merge state is MaxHeight, setting status to Mistake.";
    }
    emit merageError(state);
    qCDebug(dsrApp) << "Emitted merageError signal for invalid area.";
}
