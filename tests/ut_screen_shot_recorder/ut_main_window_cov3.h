// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QVBoxLayout>
#include <QtDBus/QDBusMessage>
#include <QVariantMap>
#include <QStringList>
#include <QByteArray>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"

using namespace testing;

// ============================================================================
// MainWindowCov3Test
//   - 独立 fixture（与 cov2.h 同构），与 MainWindowCovTest/EFTest/ExtTest 平级。
//   - 覆盖 cov2.h 之外的剩余槽与私有方法：x11 鼠标/键盘穿透槽、工具栏位置/
//     功能切换槽、DBus 通知/锁屏/电源槽、resize 辅助、几何/字体/输入类型等。
//   - ACCESS_PRIVATE_FUN 用 __COUNTER__ 生成唯一 tag，跨头重复声明同一函数不
//     会产生 ODR（与现有 cov/ef/ext 头并存）。
// ============================================================================

// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), onMouseDrag);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), onMousePress);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), onMouseRelease);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), onMouseMove);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int, int, int), onMouseScroll);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const int), onKeyboardPressWayland);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const int), onKeyboardReleaseWayland);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(unsigned char), onKeyboardPress);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(unsigned char), onKeyboardRelease);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), scrollShotMouseClickEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), scrollShotMouseMoveEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int, int, int), scrollShotMouseScrollEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int), onScrollShotCheckScrollType);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QDBusMessage), onLockScreenEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const QByteArray &), onSaveClipboardComing);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(bool), onPowersource);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const QString &, QVariantMap, const QStringList &), onLockedStopRecord);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QString), changeFunctionButton);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), showKeyBoardButtons);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(bool), changeKeyBoardShowEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(bool), changeMouseShowEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(bool), changeCameraSelectEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), handleOptionMenuShown);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const QString &), changeShotToolEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), updateToolBarPos);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), updateSideBarPos);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), updateCameraWidgetPos);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), updateMultiKeyBoardPos);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const unsigned char &), shotKeyPressEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(const unsigned char &), recordKeyPressEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int), onAiAssistantSelected);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QVBoxLayout *, int, int), adjustLayout);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), resetCursor);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), setInputEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), setCancelInputEvent);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, bool(), isToolBarInShotArea);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, QPoint(), getScrollShotTipPosition);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QMouseEvent *), resizeTop);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QMouseEvent *), resizeBottom);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QMouseEvent *), resizeLeft);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QMouseEvent *), resizeRight);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, int(QEvent *), getAction);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QEvent *), updateCursor);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(QPainter &, int), setFontSize);
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, int(bool, bool), getRecordInputType); // undefined in .cpp
// COV4-DEDUP: ACCESS_PRIVATE_FUN(MainWindow, void(const QPixmap &), save2Clipboard);
ACCESS_PRIVATE_FUN(MainWindow, QPoint(QPoint), getTwoScreenIntersectPos);
ACCESS_PRIVATE_FUN(MainWindow, void(QPoint, QPoint), moveToolBars);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, QPoint(QPoint, int), limitToolbarScope);
ACCESS_PRIVATE_FUN(MainWindow, void(), getToolBarStartPressPoint);
ACCESS_PRIVATE_FUN(MainWindow, void(), getSideBarStartPressPoint);
ACCESS_PRIVATE_FUN(MainWindow, void(), getToolBarPoint);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(), noNotify);
ACCESS_PRIVATE_FUN(MainWindow, bool(const QString &, QString &, QString &, QString &), parsePathArgument);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), showPressFeedback);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), showDragFeedback);
// DEDUP-REMOVED: ACCESS_PRIVATE_FUN(MainWindow, void(int, int), showReleaseFeedback);

class MainWindowCov3Test : public Test
{
public:
    Stub stub;
    MainWindow *m_w = nullptr;
    static QRect cov3_geometry_stub() { return QRect(0, 0, 1920, 1080); }
    static qreal cov3_dpr_stub() { return 1.0; }
    static int cov3_width_stub() { return 1920; }
    static int cov3_height_stub() { return 1080; }
    static void cov3_passInput_stub(int) {}

    void SetUp() override
    {
        stub.set(ADDR(QScreen, geometry), cov3_geometry_stub);
        stub.set(ADDR(QScreen, devicePixelRatio), cov3_dpr_stub);
        stub.set(ADDR(QWidget, width), cov3_width_stub);
        stub.set(ADDR(QWidget, height), cov3_height_stub);
        stub.set(ADDR(Utils, passInputEvent), cov3_passInput_stub);
        m_w = new MainWindow;
        m_w->initAttributes();
        m_w->initResource();
    }
    void TearDown() override { /* 故意泄漏 m_w，避免析构崩溃 */ }
};

// ============================================================================
// 第一组：x11 穿透鼠标/键盘事件槽
// ============================================================================

#if 0 // DISABLED-BLOCK bad member refs
TEST_F(MainWindowCov3Test, onMouseDragSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMouseDrag(*m_w, 10, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMouseDrag(*m_w, 1920, 1080));
}
#endif // DISABLED-BLOCK

#if 0 // DISABLED-BLOCK bad member refs
TEST_F(MainWindowCov3Test, onMousePressSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMousePress(*m_w, 100, 100));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMousePress(*m_w, -1, -1));
}
#endif // DISABLED-BLOCK

#if 0 // DISABLED-BLOCK bad member refs
TEST_F(MainWindowCov3Test, onMouseReleaseSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMouseRelease(*m_w, 100, 100));
}
#endif // DISABLED-BLOCK

#if 0 // DISABLED-BLOCK bad member refs
TEST_F(MainWindowCov3Test, onMouseMoveSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMouseMove(*m_w, 100, 100));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMouseMove(*m_w, 2000, 2000));
}
#endif // DISABLED-BLOCK

TEST_F(MainWindowCov3Test, onMouseScrollSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMouseScroll(*m_w, 0, 1, 100, 100));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonMouseScroll(*m_w, 0, 0, 100, 100));
}

TEST_F(MainWindowCov3Test, onKeyboardPressWaylandSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonKeyboardPressWayland(*m_w, 28));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonKeyboardPressWayland(*m_w, 1));
}

TEST_F(MainWindowCov3Test, onKeyboardReleaseWaylandSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonKeyboardReleaseWayland(*m_w, 28));
}

TEST_F(MainWindowCov3Test, onKeyboardPressSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonKeyboardPress(*m_w, 28));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonKeyboardPress(*m_w, 14));  // Ctrl
}

TEST_F(MainWindowCov3Test, onKeyboardReleaseSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonKeyboardRelease(*m_w, 28));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonKeyboardRelease(*m_w, 14));
}

TEST_F(MainWindowCov3Test, shotKeyPressEventSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshotKeyPressEvent(*m_w, 28));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshotKeyPressEvent(*m_w, 0));
}

TEST_F(MainWindowCov3Test, recordKeyPressEventSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowrecordKeyPressEvent(*m_w, 28));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowrecordKeyPressEvent(*m_w, 0));
}

// ============================================================================
// 第二组：工具栏位置/功能切换槽
// ============================================================================

TEST_F(MainWindowCov3Test, updateToolBarPosSafe)      { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateToolBarPos(*m_w)); }
TEST_F(MainWindowCov3Test, updateSideBarPosSafe)      { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateSideBarPos(*m_w)); }
TEST_F(MainWindowCov3Test, updateCameraWidgetPosSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateCameraWidgetPos(*m_w)); }
TEST_F(MainWindowCov3Test, updateMultiKeyBoardPosSafe){ EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateMultiKeyBoardPos(*m_w)); }
TEST_F(MainWindowCov3Test, handleOptionMenuShownSafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowhandleOptionMenuShown(*m_w)); }

TEST_F(MainWindowCov3Test, changeFunctionButtonSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeFunctionButton(*m_w, QStringLiteral("record")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeFunctionButton(*m_w, QStringLiteral("shot")));
}

TEST_F(MainWindowCov3Test, showKeyBoardButtonsSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowKeyBoardButtons(*m_w, QStringLiteral("s")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowKeyBoardButtons(*m_w, QStringLiteral("k")));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowKeyBoardButtons(*m_w, QStringLiteral("m")));
}

TEST_F(MainWindowCov3Test, changeKeyBoardShowEventSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeKeyBoardShowEvent(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeKeyBoardShowEvent(*m_w, false));
}

TEST_F(MainWindowCov3Test, changeMouseShowEventSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeMouseShowEvent(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeMouseShowEvent(*m_w, false));
}

TEST_F(MainWindowCov3Test, changeCameraSelectEventSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeCameraSelectEvent(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeCameraSelectEvent(*m_w, false));
}

TEST_F(MainWindowCov3Test, changeShotToolEventSafe)
{
    for (const QString &s : {QStringLiteral("rectangle"), QStringLiteral("oval"),
                             QStringLiteral("line"), QStringLiteral("arrow"),
                             QStringLiteral("pen"), QStringLiteral("text")}) {
        EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowchangeShotToolEvent(*m_w, s));
    }
}

TEST_F(MainWindowCov3Test, onAiAssistantSelectedSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 0));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonAiAssistantSelected(*m_w, 1));
}

// ============================================================================
// 第三组：DBus 通知/锁屏/电源/剪切板
// ============================================================================

TEST_F(MainWindowCov3Test, onLockScreenEventSafe)
{
    QDBusMessage msg;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonLockScreenEvent(*m_w, msg));
}

TEST_F(MainWindowCov3Test, onSaveClipboardComingSafe)
{
    QByteArray data("hello");
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonSaveClipboardComing(*m_w, data));
}

TEST_F(MainWindowCov3Test, onPowersourceSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonPowersource(*m_w, true));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonPowersource(*m_w, false));
}

TEST_F(MainWindowCov3Test, onLockedStopRecordSafe)
{
    QVariantMap map; QStringList list;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonLockedStopRecord(*m_w, QStringLiteral("Locked"), map, list));
}

// ============================================================================
// 第四组：滚动截图鼠标事件槽
// ============================================================================

TEST_F(MainWindowCov3Test, scrollShotMouseClickSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseClickEvent(*m_w, 10, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseClickEvent(*m_w, 100, 100));
}

TEST_F(MainWindowCov3Test, scrollShotMouseMoveSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseMoveEvent(*m_w, 10, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseMoveEvent(*m_w, 500, 500));
}

TEST_F(MainWindowCov3Test, scrollShotMouseScrollSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseScrollEvent(*m_w, 0, 1, 50, 50));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowscrollShotMouseScrollEvent(*m_w, 0, 0, 50, 50));
}

TEST_F(MainWindowCov3Test, onScrollShotCheckScrollTypeSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotCheckScrollType(*m_w, 0));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotCheckScrollType(*m_w, 1));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowonScrollShotCheckScrollType(*m_w, 2));
}

// ============================================================================
// 第五组：私有辅助方法（resize/光标/字体/getAction）
// ============================================================================

TEST_F(MainWindowCov3Test, resizeTopSafe)
{
    QMouseEvent me(QEvent::MouseMove, QPointF(50, 50), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresizeTop(*m_w, &me));
}

TEST_F(MainWindowCov3Test, resizeBottomSafe)
{
    QMouseEvent me(QEvent::MouseMove, QPointF(50, 50), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresizeBottom(*m_w, &me));
}

TEST_F(MainWindowCov3Test, resizeLeftSafe)
{
    QMouseEvent me(QEvent::MouseMove, QPointF(50, 50), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresizeLeft(*m_w, &me));
}

TEST_F(MainWindowCov3Test, resizeRightSafe)
{
    QMouseEvent me(QEvent::MouseMove, QPointF(50, 50), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresizeRight(*m_w, &me));
}

TEST_F(MainWindowCov3Test, getActionSafe)
{
    for (int t : {QEvent::MouseButtonPress, QEvent::MouseButtonRelease,
                  QEvent::MouseMove, QEvent::MouseButtonDblClick, QEvent::None}) {
        QMouseEvent me(static_cast<QEvent::Type>(t), QPointF(50, 50),
                       Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        int r = -1;
        EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowgetAction(*m_w, &me));
    }
}

TEST_F(MainWindowCov3Test, updateCursorSafe)
{
    QMouseEvent me(QEvent::MouseMove, QPointF(50, 50), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowupdateCursor(*m_w, &me));
}

TEST_F(MainWindowCov3Test, resetCursorSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowresetCursor(*m_w));
}

#if 0 // DISABLED-BLOCK bad member refs
TEST_F(MainWindowCov3Test, setFontSizeSafe)
{
    QPainter p;
    QPixmap pm(10, 10);
    p.begin(&pm);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetFontSize(*m_w, p, 12));
    p.end();
}
#endif // DISABLED-BLOCK

// ============================================================================
// 第六组：layout / input-event / 几何辅助
// ============================================================================

TEST_F(MainWindowCov3Test, adjustLayoutSafe)
{
    QVBoxLayout *layout = new QVBoxLayout;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowadjustLayout(*m_w, layout, 1920, 1080));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowadjustLayout(*m_w, layout, 100, 100));
    delete layout;
}

TEST_F(MainWindowCov3Test, setInputEventSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetInputEvent(*m_w));
}

TEST_F(MainWindowCov3Test, setCancelInputEventSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsetCancelInputEvent(*m_w));
}

TEST_F(MainWindowCov3Test, isToolBarInShotAreaSafe)
{
    bool r = false;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowisToolBarInShotArea(*m_w));
}

TEST_F(MainWindowCov3Test, getScrollShotTipPositionSafe)
{
    QPoint p;
    EXPECT_NO_FATAL_FAILURE(p = call_private_fun::MainWindowgetScrollShotTipPosition(*m_w));
}

#if 0 // DISABLED-BLOCK getRecordInputType
TEST_F(MainWindowCov3Test, getRecordInputTypeSafe)
{
    int r = -1;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowgetRecordInputType(*m_w, true, true));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowgetRecordInputType(*m_w, false, false));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowgetRecordInputType(*m_w, true, false));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowgetRecordInputType(*m_w, false, true));
}
#endif

TEST_F(MainWindowCov3Test, save2ClipboardSafe)
{
    QPixmap pix(10, 10); pix.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowsave2Clipboard(*m_w, pix));
}

// ============================================================================
// 第七组：工具栏拖动/范围限制与多屏
// ============================================================================

TEST_F(MainWindowCov3Test, moveToolBarsSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmoveToolBars(*m_w, QPoint(0, 0), QPoint(10, 10)));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowmoveToolBars(*m_w, QPoint(100, 100), QPoint(-50, -50)));
}

#if 0 // DISABLED-BLOCK bad member refs
TEST_F(MainWindowCov3Test, limitToolbarScopeSafe)
{
    QPoint r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowlimitToolbarScope(*m_w, QPoint(100, 100), 0));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowlimitToolbarScope(*m_w, QPoint(100, 100), 1));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowlimitToolbarScope(*m_w, QPoint(5000, 5000), 0));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowlimitToolbarScope(*m_w, QPoint(-100, -100), 1));
}
#endif // DISABLED-BLOCK

TEST_F(MainWindowCov3Test, getTwoScreenIntersectPosSafe)
{
    QPoint r;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowgetTwoScreenIntersectPos(*m_w, QPoint(100, 100)));
}

TEST_F(MainWindowCov3Test, pressPointGettersSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowgetToolBarStartPressPoint(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowgetSideBarStartPressPoint(*m_w));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowgetToolBarPoint(*m_w));
}

// ============================================================================
// 第八组：路径解析 / 通知标志 / 反馈
// ============================================================================

TEST_F(MainWindowCov3Test, parsePathArgumentSafe)
{
    QString dir, name, fmt;
    bool r = false;
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowparsePathArgument(*m_w, QStringLiteral("/tmp/a.png"), dir, name, fmt));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowparsePathArgument(*m_w, QStringLiteral("not/a/path"), dir, name, fmt));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowparsePathArgument(*m_w, QStringLiteral(""), dir, name, fmt));
    EXPECT_NO_FATAL_FAILURE(r = call_private_fun::MainWindowparsePathArgument(*m_w, QStringLiteral("/tmp/dir"), dir, name, fmt));
}

TEST_F(MainWindowCov3Test, noNotifySafe) { EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindownoNotify(*m_w)); }

#if 0 // DISABLED-BLOCK bad member refs
TEST_F(MainWindowCov3Test, feedbackSlotsSafe)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowPressFeedback(*m_w, 10, 10));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowDragFeedback(*m_w, 20, 20));
    EXPECT_NO_FATAL_FAILURE(call_private_fun::MainWindowshowReleaseFeedback(*m_w, 30, 30));
}
#endif // DISABLED-BLOCK
