// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gtest/gtest.h>

#include <QDBusConnection>
#include <QImage>
#include <QPixmap>

#include "../../src/dbusinterface/aiassistantinterface.h"
#include "../../src/dbusinterface/dbusnotify.h"
#include "../../src/dbusinterface/ocrinterface.h"
#include "../../src/dbusinterface/pinscreenshotsinterface.h"
#include "../../src/dbusservice/dbusscreenshot.h"
#include "../../src/pin_screenshots/putils.h"
#include "../../src/utils/tempfile.h"
#include "../../src/utils_interface.h"
#include "../../src/widgets/camerawidget.h"
#include "../../src/countdown_tooltip.h"
#include "../../src/widgets/previewwidget.h"

using namespace testing;

TEST(GeneratedDBusInterfacesExtraCovTest, callAllProxyMethods)
{
    const QDBusConnection bus = QDBusConnection::sessionBus();

    utils_interface audio(QStringLiteral("com.deepin.daemon.Audio"),
                          QStringLiteral("/com/deepin/daemon/Audio"), bus);
    (void)audio.bluetoothAudioMode();
    (void)audio.bluetoothAudioModeOpts();
    (void)audio.cards();
    (void)audio.cardsWithoutUnavailable();
    (void)audio.defaultSink();
    (void)audio.defaultSource();
    (void)audio.increaseVolume();
    audio.setIncreaseVolume(true);
    (void)audio.maxUIVolume();
    (void)audio.reduceNoise();
    audio.setReduceNoise(true);
    (void)audio.sinkInputs();
    (void)audio.sinks();
    (void)audio.sources();
    (void)audio.IsPortEnabled(0, QString());
    (void)audio.Reset();
    (void)audio.SetBluetoothAudioMode(QString());
    (void)audio.SetDefaultSink(QString());
    (void)audio.SetDefaultSource(QString());
    (void)audio.SetPort(0, QString(), 0);
    (void)audio.SetPortEnabled(0, QString(), true);

    DBusNotify notify;
    (void)notify.ClearRecords();
    (void)notify.CloseNotification(0);
    (void)notify.GetAllRecords();
    (void)notify.GetCapbilities();
    (void)notify.GetServerInformation();
    QString vendor;
    QString version;
    QString specVersion;
    (void)notify.GetServerInformation(vendor, version, specVersion);
    (void)notify.Notify(QStringLiteral("app"), 0, QStringLiteral("icon"),
                        QStringLiteral("summary"), QStringLiteral("body"),
                        QStringList(), QVariantMap(), 1000);
    (void)notify.RemoveRecord(QString());

    AiAssistantInterface ai(QStringLiteral("com.deepin.copilot"),
                            QStringLiteral("/com/deepin/copilot"), bus);
    (void)ai.launchAiQuickOCR(1, QString(), QPoint(), false, QString());
    (void)ai.launchAiQuickOCRWithImage(1, QImage(2, 2, QImage::Format_ARGB32),
                                      QString());
    (void)ai.launchChatUploadImage(QString());

    OcrInterface ocr(QStringLiteral("com.deepin.Ocr"),
                     QStringLiteral("/com/deepin/Ocr"), bus);
    (void)ocr.openFile(QString());
    (void)ocr.openImage(QImage(2, 2, QImage::Format_ARGB32));
    (void)ocr.openImageAndName(QImage(2, 2, QImage::Format_ARGB32), QString());

    PinScreenShotsInterface pin(QStringLiteral("com.deepin.PinScreenShots"),
                                QStringLiteral("/com/deepin/PinScreenShots"), bus);
    (void)pin.openFile(QString());
    (void)pin.openImage(QImage(2, 2, QImage::Format_ARGB32));
    (void)pin.openImageAndName(QImage(2, 2, QImage::Format_ARGB32), QString(), QPoint());
    (void)pin.openImageAndNameForTable(QImage(2, 2, QImage::Format_ARGB32), QString(), QPoint());

    DBusScreenshot screenshot(QStringLiteral("com.deepin.DeepinScreenshot"),
                              QStringLiteral("/com/deepin/DeepinScreenshot"), bus);
    (void)screenshot.DelayScreenshot(1);
    (void)screenshot.FullscreenScreenshot();
    (void)screenshot.NoNotifyScreenshot();
    (void)screenshot.SavePathScreenshot(QString());
    (void)screenshot.StartScreenshot();
    (void)screenshot.TopWindowScreenshot();
}

TEST(MiscUtilityExtraCovTest, themeCameraPreviewAndBackground)
{
    EXPECT_NO_FATAL_FAILURE((void)PUtils::isDarkTheme());
    EXPECT_NO_FATAL_FAILURE((void)PUtils::themeColor(Qt::black, Qt::white));

    CameraWidget camera;
    camera.setParentGlobalOffset(QPoint(12, 34));
    EXPECT_EQ(camera.parentGlobalOffset(), QPoint(12, 34));

    PreviewWidget preview(QRect(0, 0, 320, 240));
    preview.setMaxPreviewSize(QSize(640, 480));

    CountdownTooltip tooltip;
    QPixmap background(16, 16);
    background.fill(Qt::blue);
    EXPECT_NO_FATAL_FAILURE(tooltip.setBackgroundPixmap(background, QPoint(3, 4)));
}
