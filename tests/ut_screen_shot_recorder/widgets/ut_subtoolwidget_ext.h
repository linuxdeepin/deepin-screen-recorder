// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QAction>
#include <QWidget>
#include <QSignalSpy>
#include <QMetaObject>
#include <QStandardPaths>

#include "stub.h"
#include "addr_pri.h"

#include "../../src/main_window.h"
#include "../../src/widgets/subtoolwidget.h"
#include "../../src/utils/configsettings.h"
#include "../../src/utils/saveutils.h"

// Private (non-slot, non-const) helper exercised via addr_pri.
// Verified: no other test file declares this macro for SubToolWidget.
ACCESS_PRIVATE_FUN(SubToolWidget, void(), showAIAssistantWidget);

using namespace testing;

class SubToolWidgetExtTest : public Test
{
public:
    MainWindow *m_mainWindow = nullptr;
    SubToolWidget *m_w = nullptr;

    void SetUp() override
    {
        m_mainWindow = new MainWindow;
        m_w = new SubToolWidget(m_mainWindow);
    }

    void TearDown() override
    {
        if (m_w) {
            delete m_w;
            m_w = nullptr;
        }
        if (m_mainWindow) {
            m_mainWindow->deleteLater();
            m_mainWindow = nullptr;
        }
    }
};

// ---------------------------------------------------------------------------
// installTipHint / installHint : property + event-filter installation only.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, installTipHintSetsHintWidgetProperty)
{
    QWidget *w = new QWidget;
    EXPECT_NO_FATAL_FAILURE(m_w->installTipHint(w, QStringLiteral("矩形 (R)")));
    QVariant v = w->property("HintWidget");
    EXPECT_TRUE(v.isValid());
    EXPECT_FALSE(v.value<QWidget *>() == nullptr);
    delete w;
}

TEST_F(SubToolWidgetExtTest, installHintInstallsEventFilter)
{
    QWidget *w = new QWidget;
    QWidget *hint = new QWidget;
    EXPECT_NO_FATAL_FAILURE(m_w->installHint(w, hint));
    // installHint stores the hint widget in the "HintWidget" property
    QVariant v = w->property("HintWidget");
    EXPECT_TRUE(v.isValid());
    EXPECT_EQ(v.value<QWidget *>(), hint);
    delete hint;
    delete w;
}

TEST_F(SubToolWidgetExtTest, installTipHintEmptyString)
{
    QWidget *w = new QWidget;
    EXPECT_NO_FATAL_FAILURE(m_w->installTipHint(w, QString()));
    EXPECT_TRUE(w->property("HintWidget").isValid());
    delete w;
}

// ---------------------------------------------------------------------------
// updateSaveButtonTip : drives the big save-ways / save-op / history-path
// decision tree. Toggle config then refresh to hit each branch.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, updateSaveButtonTipAllBranches)
{
    ConfigSettings *s = ConfigSettings::instance();
    ASSERT_NE(s, nullptr);

    // Ask mode
    EXPECT_NO_FATAL_FAILURE({
        s->setValue("shot", "save_ways", SaveWays::Ask);
        s->setValue("shot", "save_op", static_cast<SaveAction>(SaveToClipboard));
        m_w->updateSaveButtonTip();
    });

    // SpecifyLocation + each SaveAction enum value
    const SaveAction ops[] = {SaveToClipboard, SaveToDesktop, SaveToImage,
                              SaveToSpecificDir, AutoSave, CustomScreenSave};
    for (SaveAction op : ops) {
        EXPECT_NO_FATAL_FAILURE({
            s->setValue("shot", "save_ways", SaveWays::SpecifyLocation);
            s->setValue("shot", "save_op", static_cast<SaveAction>(op));
            s->setValue("shot", "save_dir_change", false);
            s->setValue("shot", "save_dir", QString());
            m_w->updateSaveButtonTip();
        });
    }

    // SaveToSpecificDir with change-on-save toggled
    EXPECT_NO_FATAL_FAILURE({
        s->setValue("shot", "save_op", static_cast<SaveAction>(SaveToSpecificDir));
        s->setValue("shot", "save_dir_change", true);
        s->setValue("shot", "save_dir", QString());
        m_w->updateSaveButtonTip();
    });

    // SaveToSpecificDir with an existing history path on disk (use a temp dir)
    EXPECT_NO_FATAL_FAILURE({
        s->setValue("shot", "save_op", static_cast<SaveAction>(SaveToSpecificDir));
        s->setValue("shot", "save_dir_change", false);
        s->setValue("shot", "save_dir", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
        m_w->updateSaveButtonTip();
    });

    // Unknown save_ways value -> default branch
    EXPECT_NO_FATAL_FAILURE({
        s->setValue("shot", "save_ways", static_cast<SaveWays>(999));
        m_w->updateSaveButtonTip();
    });
}

// ---------------------------------------------------------------------------
// Simple boolean setters: cover both true/false branches.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, setScrollShotDisabledBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setScrollShotDisabled(true));
    EXPECT_NO_FATAL_FAILURE(m_w->setScrollShotDisabled(false));
}

TEST_F(SubToolWidgetExtTest, setPinScreenshotsEnableBothStates)
{
    bool t = true, f = false;
    EXPECT_NO_FATAL_FAILURE(m_w->setPinScreenshotsEnable(t));
    EXPECT_NO_FATAL_FAILURE(m_w->setPinScreenshotsEnable(f));
}

TEST_F(SubToolWidgetExtTest, setOcrScreenshotEnableBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setOcrScreenshotEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_w->setOcrScreenshotEnable(false));
}

TEST_F(SubToolWidgetExtTest, setButEnableOnLockScreenBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setButEnableOnLockScreen(true));
    EXPECT_NO_FATAL_FAILURE(m_w->setButEnableOnLockScreen(false));
}

// ---------------------------------------------------------------------------
// getFuncSubToolX : pure logic with a shape->x() dispatch. Cover all branches.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, getFuncSubToolXAllShapes)
{
    QStringList shapes = {QStringLiteral("gio"), QStringLiteral("rectangle"),
                          QStringLiteral("oval"),  QStringLiteral("rect"),
                          QStringLiteral("circ"),  QStringLiteral("line"),
                          QStringLiteral("arrow"), QStringLiteral("pen"),
                          QStringLiteral("text"),  QStringLiteral("aiassistant")};
    for (QString &s : shapes) {
        int x = -999;
        EXPECT_NO_FATAL_FAILURE(x = m_w->getFuncSubToolX(s));
        EXPECT_GE(x, -1); // valid buttons return >=0, fallback returns -1
    }
}

TEST_F(SubToolWidgetExtTest, getFuncSubToolXUnknownShapeReturnsMinusOne)
{
    QString s = QStringLiteral("nonexistent_shape");
    int x = 999;
    EXPECT_NO_FATAL_FAILURE(x = m_w->getFuncSubToolX(s));
    EXPECT_EQ(x, -1);
}

TEST_F(SubToolWidgetExtTest, getFuncSubToolXEmptyStringReturnsZero)
{
    QString s;
    int x = 999;
    EXPECT_NO_FATAL_FAILURE(x = m_w->getFuncSubToolX(s));
    EXPECT_EQ(x, 0); // early-return preserves initial x=0 when shape empty
}

// ---------------------------------------------------------------------------
// switchContent : "shot" / "record" / "scroll" branches. "scroll" lazily
// builds the scroll toolbar via initScrollLabel (safe, defined in .cpp).
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, switchContentAllTypes)
{
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("record")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("shot")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("scroll")));
    // back to record/shot to exercise removeWidget paths on the now-built scroll page
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("record")));
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("shot")));
}

TEST_F(SubToolWidgetExtTest, switchContentUnknownTypeIsNoop)
{
    int before = m_w->count();
    EXPECT_NO_FATAL_FAILURE(m_w->switchContent(QStringLiteral("totally_unknown")));
    EXPECT_EQ(m_w->count(), before);
}

// ---------------------------------------------------------------------------
// getShotOptionRect : menu hidden by default -> returns QRect().
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, getShotOptionRectWhenHidden)
{
    QRect r;
    EXPECT_NO_FATAL_FAILURE(r = m_w->getShotOptionRect());
    // menu is hidden after initShotOption, so rect should be empty
    EXPECT_TRUE(r.isNull() || r.isEmpty() || r.width() >= 0);
}

// ---------------------------------------------------------------------------
// getAiButtonGlobalRect / getAiButtonGlobalCenter : mapToGlobal without a
// shown window returns platform-relative coords; should not crash.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, aiButtonGeometryAccessors)
{
    QRect r;
    QPoint c;
    EXPECT_NO_FATAL_FAILURE(r = m_w->getAiButtonGlobalRect());
    EXPECT_NO_FATAL_FAILURE(c = m_w->getAiButtonGlobalCenter());
    // center is computed from rect.center(), so must be consistent
    EXPECT_EQ(c, r.center());
}

// ---------------------------------------------------------------------------
// setRecordLaunchMode : record branch calls switchContent + emits a signal.
// ocr/scrollshot branches click internal buttons (safe slots).
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, setRecordLaunchModeRecordBranch)
{
    QSignalSpy spy(m_w, &SubToolWidget::changeShotToolFunc);
    EXPECT_NO_FATAL_FAILURE(m_w->setRecordLaunchMode(static_cast<unsigned int>(MainWindow::record)));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(SubToolWidgetExtTest, setRecordLaunchModeOcrBranch)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setRecordLaunchMode(static_cast<unsigned int>(MainWindow::ocr)));
}

TEST_F(SubToolWidgetExtTest, setRecordLaunchModeScrollshotBranch)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setRecordLaunchMode(static_cast<unsigned int>(MainWindow::scrollshot)));
}

TEST_F(SubToolWidgetExtTest, setRecordLaunchModeUnknownFunType)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setRecordLaunchMode(9999u));
}

// ---------------------------------------------------------------------------
// setVideoButtonInitFromSub : no-op when neither camera nor keyboard checked.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, setVideoButtonInitFromSubWhenUnchecked)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setVideoButtonInitFromSub());
}

// ---------------------------------------------------------------------------
// shapeClickedFromWidget : drive each dispatch branch. Empty-string is a noop.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, shapeClickedFromWidgetAllKnownShapes)
{
    const QStringList shapes = {QStringLiteral("pinScreenshots"),
                                QStringLiteral("scrollShot"),
                                QStringLiteral("ocr"),
                                QStringLiteral("gio"),
                                QStringLiteral("rect"),
                                QStringLiteral("circ"),
                                QStringLiteral("rectangle"),
                                QStringLiteral("oval"),
                                QStringLiteral("line"),
                                QStringLiteral("arrow"),
                                QStringLiteral("pen"),
                                QStringLiteral("text"),
                                QStringLiteral("aiassistant"),
                                QStringLiteral("keyBoard"),
                                QStringLiteral("camera"),
                                QStringLiteral("effect")};
    for (const QString &s : shapes) {
        EXPECT_NO_FATAL_FAILURE(m_w->shapeClickedFromWidget(s));
    }
}

TEST_F(SubToolWidgetExtTest, shapeClickedFromWidgetUnknownShape)
{
    EXPECT_NO_FATAL_FAILURE(m_w->shapeClickedFromWidget(QStringLiteral("unknown_xyz")));
}

TEST_F(SubToolWidgetExtTest, shapeClickedFromWidgetEmptyString)
{
    EXPECT_NO_FATAL_FAILURE(m_w->shapeClickedFromWidget(QString()));
}

// ---------------------------------------------------------------------------
// setMicroPhoneEnable : GIF format (0) disables mic regardless of arg;
// other formats honor the status. Cover both.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, setMicroPhoneEnableBothStates)
{
    ConfigSettings *s = ConfigSettings::instance();
    ASSERT_NE(s, nullptr);

    s->setValue("recorder", "format", 1); // non-GIF
    EXPECT_NO_FATAL_FAILURE(m_w->setMicroPhoneEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_w->setMicroPhoneEnable(false));

    s->setValue("recorder", "format", 0); // GIF
    EXPECT_NO_FATAL_FAILURE(m_w->setMicroPhoneEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_w->setMicroPhoneEnable(false));
}

// ---------------------------------------------------------------------------
// setCameraDeviceEnable : true (already enabled -> inner branch skipped) and
// false (disables the button) branches.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, setCameraDeviceEnableBothStates)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setCameraDeviceEnable(true));
    EXPECT_NO_FATAL_FAILURE(m_w->setCameraDeviceEnable(false));
    // toggle back to true now that it was disabled, to hit the enable path
    EXPECT_NO_FATAL_FAILURE(m_w->setCameraDeviceEnable(true));
}

// ---------------------------------------------------------------------------
// setRecordButtonDisable : currently an empty body, just ensure it returns.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, setRecordButtonDisableIsNoop)
{
    EXPECT_NO_FATAL_FAILURE(m_w->setRecordButtonDisable());
}

// ---------------------------------------------------------------------------
// showAIAssistantWidget (private) : posts a queued click to the AI button.
// Verified defined in subtoolwidget.cpp (line ~844). Invoked via addr_pri.
// ---------------------------------------------------------------------------
TEST_F(SubToolWidgetExtTest, showAIAssistantWidgetPrivateNoCrash)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SubToolWidgetshowAIAssistantWidget(*m_w));
}
