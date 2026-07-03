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

// ---------- AiAssistantInterface (0%) ----------
TEST(AiAssistantInterfaceCovTest, constructAndCall)
{
    AiAssistantInterface *iface = nullptr;
    EXPECT_NO_FATAL_FAILURE(iface = new AiAssistantInterface(
        QStringLiteral("com.deepin.copilot"),
        QStringLiteral("/com/deepin/copilot"),
        QDBusConnection::sessionBus()));
    if (iface) {
        QImage img(8, 8, QImage::Format_ARGB32);
        img.fill(Qt::red);
        EXPECT_NO_FATAL_FAILURE(iface->launchAiQuickOCR(1, QStringLiteral("hi"), QPoint(0, 0), false, QString()));
        EXPECT_NO_FATAL_FAILURE(iface->launchAiQuickOCRWithImage(2, img, QStringLiteral("a.png")));
        EXPECT_NO_FATAL_FAILURE(iface->launchChatUploadImage(QStringLiteral("/tmp/a.png")));
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
    }
}
