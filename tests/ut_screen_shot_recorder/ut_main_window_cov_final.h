// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QShortcut>
#include <QTimer>
#include <QEventLoop>
#include <QPixmap>
#include <QImage>
#include <QTest>
#include <QDBusPendingCallWatcher>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"

using namespace testing;

class MainWindowCovFinalTest : public Test
{
public:
    Stub stub;
    MainWindow *m_w = nullptr;
    static QRect mcft_geom_stub() { return QRect(0, 0, 1920, 1080); }
    static qreal mcft_dpr_stub() { return 1.0; }
    static int mcft_width_stub() { return 1920; }
    static int mcft_height_stub() { return 1080; }
    static void mcft_passInput_stub(int) {}

    void SetUp() override
    {
        stub.set(ADDR(QScreen, geometry), mcft_geom_stub);
        stub.set(ADDR(QScreen, devicePixelRatio), mcft_dpr_stub);
        stub.set(ADDR(QWidget, width), mcft_width_stub);
        stub.set(ADDR(QWidget, height), mcft_height_stub);
        stub.set(ADDR(Utils, passInputEvent), mcft_passInput_stub);
        m_w = new MainWindow;
        m_w->initAttributes();
        m_w->initResource();
    }
    void TearDown() override { }

    void emitAllShortcuts()
    {
        QList<QShortcut *> shortcuts = m_w->findChildren<QShortcut *>();
        for (QShortcut *sc : shortcuts) {
            EXPECT_NO_FATAL_FAILURE(emit sc->activated());
        }
        QTest::qWait(50);
    }
};

TEST_F(MainWindowCovFinalTest, InitToolBarShortcutLambdasShotMode)
{
    access_private_field::MainWindowm_functionType(*m_w) = MainWindow::status::shot;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitToolBarShortcut(*m_w));
    EXPECT_NO_FATAL_FAILURE(emitAllShortcuts());
}

TEST_F(MainWindowCovFinalTest, InitToolBarShortcutLambdasRecordMode)
{
    access_private_field::MainWindowm_functionType(*m_w) = MainWindow::status::record;
    access_private_field::MainWindowrecordButtonStatus(*m_w) = 0;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitToolBarShortcut(*m_w));
    EXPECT_NO_FATAL_FAILURE(emitAllShortcuts());
}

TEST_F(MainWindowCovFinalTest, InitToolBarShortcutLambdasScrollMode)
{
    access_private_field::MainWindowm_functionType(*m_w) = MainWindow::status::scrollshot;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitToolBarShortcut(*m_w));
    EXPECT_NO_FATAL_FAILURE(emitAllShortcuts());
}

TEST_F(MainWindowCovFinalTest, InitToolBarShortcutLambdasPinMode)
{
    access_private_field::MainWindowm_functionType(*m_w) = MainWindow::status::pinscreenshots;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitToolBarShortcut(*m_w));
    EXPECT_NO_FATAL_FAILURE(emitAllShortcuts());
}

TEST_F(MainWindowCovFinalTest, InitSaveShortcutLambdasShotMode)
{
    access_private_field::MainWindowm_functionType(*m_w) = MainWindow::status::shot;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitSaveShortcut(*m_w));
    EXPECT_NO_FATAL_FAILURE(emitAllShortcuts());
}

TEST_F(MainWindowCovFinalTest, InitSaveShortcutLambdasRecordMode)
{
    access_private_field::MainWindowm_functionType(*m_w) = MainWindow::status::record;
    access_private_field::MainWindowrecordButtonStatus(*m_w) = 2;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitSaveShortcut(*m_w));
    EXPECT_NO_FATAL_FAILURE(emitAllShortcuts());
}

TEST_F(MainWindowCovFinalTest, InitSaveShortcutLambdasScrollMode)
{
    access_private_field::MainWindowm_functionType(*m_w) = MainWindow::status::scrollshot;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitSaveShortcut(*m_w));
    EXPECT_NO_FATAL_FAILURE(emitAllShortcuts());
}

TEST_F(MainWindowCovFinalTest, InitSaveShortcutLambdasPinMode)
{
    access_private_field::MainWindowm_functionType(*m_w) = MainWindow::status::pinscreenshots;
    access_private_field::MainWindowrecordButtonStatus(*m_w) = 2;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowinitSaveShortcut(*m_w));
    EXPECT_NO_FATAL_FAILURE(emitAllShortcuts());
}

TEST_F(MainWindowCovFinalTest, FullScreenRecordNoOpPath)
{
    EXPECT_NO_FATAL_FAILURE(m_w->fullScreenRecord(QStringLiteral("")));
    EXPECT_NO_FATAL_FAILURE(m_w->fullScreenRecord(QStringLiteral("/tmp/ut_mcft_rec.mp4")));
}

#ifdef OCR_SCROLL_FLAGE_ON
TEST_F(MainWindowCovFinalTest, ScrollShotGrabPixmapOutside)
{
    access_private_field::MainWindowm_scrollShot(*m_w) = new ScrollScreenshot(m_w);
    access_private_field::MainWindowm_scrollShotType(*m_w) = MainWindow::ScrollShotType::Unknow;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotGrabPixmap(*m_w, PreviewWidget::PostionStatus::RIGHT, 1, 0));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotGrabPixmap(*m_w, PreviewWidget::PostionStatus::RIGHT, 0, 10));
}

TEST_F(MainWindowCovFinalTest, ScrollShotGrabPixmapInside)
{
    access_private_field::MainWindowm_scrollShot(*m_w) = new ScrollScreenshot(m_w);
    access_private_field::MainWindowm_scrollShotType(*m_w) = MainWindow::ScrollShotType::Unknow;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotGrabPixmap(*m_w, PreviewWidget::PostionStatus::INSIDE, 1, 0));
    QTest::qWait(200);
}

TEST_F(MainWindowCovFinalTest, OnScrollShotMerageImgStateFailed)
{
    access_private_field::MainWindowm_tipShowtimer(*m_w) = new QTimer(m_w);
    access_private_field::MainWindowm_initScroll(*m_w) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotMerageImgState(*m_w, PixMergeThread::MergeErrorValue::Failed));
}

TEST_F(MainWindowCovFinalTest, OnScrollShotMerageImgStateReachBottom)
{
    access_private_field::MainWindowm_tipShowtimer(*m_w) = new QTimer(m_w);
    access_private_field::MainWindowm_initScroll(*m_w) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotMerageImgState(*m_w, PixMergeThread::MergeErrorValue::ReachBottom));
}

TEST_F(MainWindowCovFinalTest, OnScrollShotMerageImgStateMaxHeight)
{
    access_private_field::MainWindowm_tipShowtimer(*m_w) = new QTimer(m_w);
    access_private_field::MainWindowm_initScroll(*m_w) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotMerageImgState(*m_w, PixMergeThread::MergeErrorValue::MaxHeight));
}

TEST_F(MainWindowCovFinalTest, OnScrollShotMerageImgStateInvalidArea)
{
    access_private_field::MainWindowm_tipShowtimer(*m_w) = new QTimer(m_w);
    access_private_field::MainWindowm_initScroll(*m_w) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotMerageImgState(*m_w, PixMergeThread::MergeErrorValue::InvalidArea));
}

TEST_F(MainWindowCovFinalTest, OnScrollShotMerageImgStateRollingTooFast)
{
    access_private_field::MainWindowm_tipShowtimer(*m_w) = new QTimer(m_w);
    access_private_field::MainWindowm_initScroll(*m_w) = false;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotMerageImgState(*m_w, PixMergeThread::MergeErrorValue::RoollingTooFast));
}
#endif

TEST_F(MainWindowCovFinalTest, ChangeShotToolEventScrollShotLambda)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeShotToolEvent(*m_w, QStringLiteral("scrollShot")));
    QEventLoop loop;
    QTimer::singleShot(200, &loop, &QEventLoop::quit);
    loop.exec();
}

TEST_F(MainWindowCovFinalTest, ChangeShotToolEventAiAssistant)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeShotToolEvent(*m_w, QStringLiteral("aiassistant")));
}

TEST_F(MainWindowCovFinalTest, PaintImageSafe)
{
    QPixmap r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowpaintImage(*m_w));
}

TEST_F(MainWindowCovFinalTest, PaintImageWithBackground)
{
    access_private_field::MainWindowm_backgroundPixmap(*m_w) = QPixmap(200, 200);
    access_private_field::MainWindowm_backgroundPixmap(*m_w).fill(Qt::blue);
    QPixmap r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowpaintImage(*m_w));
}

TEST_F(MainWindowCovFinalTest, OnAiAssistantSelectedExplain)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 0));
    QTest::qWait(100);
}

TEST_F(MainWindowCovFinalTest, OnAiAssistantSelectedSummarize)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 1));
    QTest::qWait(100);
}

TEST_F(MainWindowCovFinalTest, OnAiAssistantSelectedTranslate)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 2));
    QTest::qWait(100);
}

TEST_F(MainWindowCovFinalTest, OnAiAssistantSelectedAskAI)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 3));
    QTest::qWait(100);
}

TEST_F(MainWindowCovFinalTest, OnAiAssistantSelectedLambdaTrigger)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 0));
    QTest::qWait(200);
    QList<QDBusPendingCallWatcher *> watchers = m_w->findChildren<QDBusPendingCallWatcher *>();
    for (QDBusPendingCallWatcher *w : watchers) {
        EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(w, "finished", Qt::QueuedConnection,
            Q_ARG(QDBusPendingCallWatcher *, w)));
    }
    QTest::qWait(100);
}

TEST_F(MainWindowCovFinalTest, SetToolbarVisable)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setToolbarVisable(true));
    EXPECT_NO_FATAL_FAILURE(m_w->setToolbarVisable(false));
}
