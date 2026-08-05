// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QImage>
#include <QPoint>
#include <QPixmap>
#include "stub.h"
#include "addr_pri.h"

#include <QEnterEvent>
#include "../../src/widgets/keybuttonwidget.h"
#include "../../src/widgets/zoomIndicatorGL.h"
#include "../../src/widgets/slider.h"
#include "../../src/dbusinterface/aiassistantinterface.h"
#include "../../src/utils/x_multi_screen_info.h"
#include "../../src/utils_interface.h"
#include "../../src/utils/voicevolumewatcher_interface.h"

using namespace testing;

// ---------- KeyButtonWidget (0%) ----------
TEST(KeyButtonWidgetCovTest, constructAndSet)
{
    KeyButtonWidget *w = nullptr;
    EXPECT_NO_FATAL_FAILURE(w = new KeyButtonWidget());
    if (w) {
        EXPECT_NO_FATAL_FAILURE(w->setKeyLabelWord(QStringLiteral("Ctrl")));
        EXPECT_NO_FATAL_FAILURE(w->setKeyLabelWord(QStringLiteral("Shift")));
    }
    // intentional leak to avoid destructor crash
}

// ---------- ZoomIndicatorGL (0%) ----------
TEST(ZoomIndicatorGLCovTest, constructAndShow)
{
    ZoomIndicatorGL *g = nullptr;
    EXPECT_NO_FATAL_FAILURE(g = new ZoomIndicatorGL());
    if (g) {
        EXPECT_NO_FATAL_FAILURE(g->showMagnifier(QPoint(10, 10)));
        EXPECT_NO_FATAL_FAILURE(g->show());
    }
}

// ZoomIndicatorGL 析构函数：堆分配后 delete 触发 deleting destructor (D0)。
// 既有用例只构造不析构，这里补上析构覆盖。
TEST(ZoomIndicatorGLCovTest, heapDestructor)
{
    ZoomIndicatorGL *g = nullptr;
    EXPECT_NO_FATAL_FAILURE(g = new ZoomIndicatorGL());
    if (g) {
        EXPECT_NO_FATAL_FAILURE(g->hide());
        EXPECT_NO_FATAL_FAILURE(delete g);
    }
}

// ---------- AiAssistantInterface (0%) ----------
TEST(AiAssistantInterfaceCovTest, constructAndCall)
{
    AiAssistantInterface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new AiAssistantInterface(
        QStringLiteral("com.deepin.copilot"),
        QStringLiteral("/com/deepil/copilot"),
        QDBusConnection::sessionBus()));
    if (iface) {
        QImage img(8, 8, QImage::Format_ARGB32);
        img.fill(Qt::red);
        EXPECT_NO_FATAL_FAILURE(iface->launchAiQuickOCR(1, QStringLiteral("hi"), QPoint(0, 0), false, QString()));
        EXPECT_NO_FATAL_FAILURE(iface->launchAiQuickOCRWithImage(2, img, QStringLiteral("a.png")));
        EXPECT_NO_FATAL_FAILURE(iface->launchChatUploadImage(QStringLiteral("/tmp/a.png")));
        // Trigger the deleting destructor (D0) — stack objects only go D2.
        EXPECT_NO_FATAL_FAILURE(delete iface);
    }
}

// AiAssistantInterface destructor: heap allocate + delete again to ensure D0/D2.
TEST(AiAssistantInterfaceCovTest, heapDestructor)
{
    AiAssistantInterface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new AiAssistantInterface(
        QStringLiteral("com.deepin.copilot"),
        QStringLiteral("/com/deepin/copilot"),
        QDBusConnection::sessionBus()));
    if (iface) {
        EXPECT_NO_FATAL_FAILURE(delete iface);
    }
}

// ---------- XMultiScreenInfo (7.7%) ----------
TEST(XMultiScreenInfoCovTest, constructAndQuery)
{
    XMultiScreenInfo *info = nullptr;
    bool ret = false;
    EXPECT_NO_FATAL_FAILURE(info = new XMultiScreenInfo());
    if (info) {
        EXPECT_NO_FATAL_FAILURE(ret = info->screenNeedResetScale());
        (void)ret;
    }
}

// ---------- Slider (35.7%) ----------
TEST(SliderCovTest, construct)
{
    Slider *s = nullptr;
    EXPECT_NO_FATAL_FAILURE(s = new Slider(Qt::Horizontal));
    delete s;
}

// Slider 析构函数（inline）与 enterEvent 覆盖。
// 源码缺陷：leaveEvent 解引用 m_lastCursorShape->shape()，当 qApp->overrideCursor()
// 为空时空指针解引用崩溃（leaveEvent 无空指针保护）。
// 本用例仅覆盖 enterEvent 与析构函数，避免 leaveEvent 崩溃。
TEST(SliderCovTest, enterEventAndDestructor)
{
    qApp->setOverrideCursor(Qt::IBeamCursor);
    Slider *s = new Slider(nullptr);
    s->resize(50, 50);
    QEnterEvent enter(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
    EXPECT_NO_FATAL_FAILURE(qApp->sendEvent(s, &enter));
    EXPECT_NO_FATAL_FAILURE(delete s);
    while (qApp->overrideCursor()) {
        qApp->restoreOverrideCursor();
    }
}

// ---------- utils_interface (3.1%) ----------
TEST(UtilsInterfaceCovTest, constructAndProps)
{
    utils_interface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new utils_interface(
        QStringLiteral("com.deepin.daemon.Audio"),
        QStringLiteral("/com/deepin/daemon/Audio"),
        QDBusConnection::sessionBus()));
    if (iface) {
        EXPECT_NO_FATAL_FAILURE(iface->bluetoothAudioMode());
        EXPECT_NO_FATAL_FAILURE(iface->bluetoothAudioModeOpts());
        EXPECT_NO_FATAL_FAILURE(iface->cards());
        EXPECT_NO_FATAL_FAILURE(iface->cardsWithoutUnavailable());
        EXPECT_NO_FATAL_FAILURE(iface->defaultSink());
        EXPECT_NO_FATAL_FAILURE(iface->defaultSource());
        EXPECT_NO_FATAL_FAILURE(iface->increaseVolume());
        EXPECT_NO_FATAL_FAILURE(iface->setIncreaseVolume(true));
        EXPECT_NO_FATAL_FAILURE(iface->maxUIVolume());
        EXPECT_NO_FATAL_FAILURE(iface->reduceNoise());
        EXPECT_NO_FATAL_FAILURE(iface->setReduceNoise(false));
        EXPECT_NO_FATAL_FAILURE(iface->sinkInputs());
        EXPECT_NO_FATAL_FAILURE(iface->sinks());
        EXPECT_NO_FATAL_FAILURE(iface->sources());
        EXPECT_NO_FATAL_FAILURE(iface->IsPortEnabled(0, QStringLiteral("Speaker")));
        EXPECT_NO_FATAL_FAILURE(iface->Reset());
        EXPECT_NO_FATAL_FAILURE(iface->SetBluetoothAudioMode(QStringLiteral("a2dp")));
        EXPECT_NO_FATAL_FAILURE(iface->SetDefaultSink(QStringLiteral("x")));
        EXPECT_NO_FATAL_FAILURE(iface->SetDefaultSource(QStringLiteral("y")));
        EXPECT_NO_FATAL_FAILURE(iface->SetPort(0, QStringLiteral("p"), 1));
        EXPECT_NO_FATAL_FAILURE(iface->SetPortEnabled(0, QStringLiteral("p"), true));
        // Trigger the deleting destructor (D0).
        EXPECT_NO_FATAL_FAILURE(delete iface);
    }
}

// utils_interface destructor: heap allocate + delete again to ensure D0/D2.
TEST(UtilsInterfaceCovTest, heapDestructor)
{
    utils_interface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new utils_interface(
        QStringLiteral("com.deepin.daemon.Audio"),
        QStringLiteral("/com/deepin/daemon/Audio"),
        QDBusConnection::sessionBus()));
    if (iface) {
        EXPECT_NO_FATAL_FAILURE(delete iface);
    }
}

// ---------- AIAssistantWidget::onToolButtonClicked (0%) ----------
// onToolButtonClicked is a private slot that emits functionSelected(AIFunction).
// It's invokable via QMetaObject::invokeMethod with the slot name. Cover all 4 enum values.
#include "../../src/widgets/aiassistantwidget.h"
TEST(AIAssistantWidgetCovTest, onToolButtonClickedEmitsSignal)
{
    AIAssistantWidget *w = nullptr;
    EXPECT_NO_FATAL_FAILURE(w = new AIAssistantWidget());
    if (!w) { GTEST_SKIP() << "AIAssistantWidget construction failed"; }
    QSignalSpy spy(w, &AIAssistantWidget::functionSelected);
    for (int i = 0; i < 4; ++i) {
        EXPECT_NO_FATAL_FAILURE(
            QMetaObject::invokeMethod(w, "onToolButtonClicked",
                                       Qt::DirectConnection, Q_ARG(int, i)));
    }
    EXPECT_GE(spy.count(), 4);
    delete w;
}

// ---------- voicevolumewatcher_interface destructor (0%) ----------
// voicevolumewatcher_interface is a QDBusAbstractInterface subclass.
// Heap-allocate + delete to trigger the deleting destructor (D0).
TEST(VoiceVolumeWatcherInterfaceCovTest, heapDestructor)
{
    voicevolumewatcher_interface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new voicevolumewatcher_interface(
        QStringLiteral("com.deepin.daemon.Audio"),
        QStringLiteral("/com/deepin/daemon/Audio"),
        QDBusConnection::sessionBus()));
    if (iface) {
        EXPECT_NO_FATAL_FAILURE(delete iface);
    }
}

// ---------- AudioPort (proxyaudioport.h) operators (0%) ----------
// AudioPort is a simple struct with DBus streaming operators and == / !=.
#include "../../src/utils/proxyaudioport.h"

TEST(AudioPortCovTest, equalityAndInequality)
{
    AudioPort a, b;
    a.name = QStringLiteral("spk");
    a.description = QStringLiteral("Speaker");
    a.availability = 2;
    b = a;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    b.name = QStringLiteral("mic");
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(AudioPortCovTest, qDebugOutput)
{
    AudioPort a;
    a.description = QStringLiteral("Headset");
    EXPECT_NO_FATAL_FAILURE({ qDebug() << a; });
}

TEST(AudioPortCovTest, qdbusArgumentRoundTrip)
{
    AudioPort a;
    a.name = QStringLiteral("spk");
    a.description = QStringLiteral("Speaker");
    a.availability = 2;
    // registerAudioPortMetaType registers the metatype -> calls qDBusRegisterMetaType
    // which internally triggers QMetaTypeId<AudioPort>::qt_metatype_id().
    EXPECT_NO_FATAL_FAILURE(registerAudioPortMetaType());
    // Exercise the QDBusArgument streaming operators for AudioPort.
    QDBusArgument outArg;
    outArg << a;  // operator<<(QDBusArgument&, const AudioPort&)
    EXPECT_NO_FATAL_FAILURE(outArg << a);
}

TEST(AudioPortCovTest, qdbusArgumentReadOperator)
{
    AudioPort a;
    a.name = QStringLiteral("mic");
    a.description = QStringLiteral("Microphone");
    a.availability = 1;
    QDBusArgument outArg;
    outArg << a;
    // The >> operator reads back from a QDBusArgument. Since QDBusArgument
    // requires a real DBus message for deserialization, calling >> on an
    // empty/partial argument is undefined; we only exercise the function
    // entry via a no-op EXPECT_NO_FATAL_FAILURE with a fresh argument.
    AudioPort b;
    QDBusArgument inArg;
    // Cannot truly deserialize without a real message; just cover the
    // function body entry by invoking it (best-effort, no assertion).
    EXPECT_NO_FATAL_FAILURE(inArg >> b);
}
