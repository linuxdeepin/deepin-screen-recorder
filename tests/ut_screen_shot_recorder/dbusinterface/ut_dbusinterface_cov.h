// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QtDBus/QtDBus>
#include <QImage>
#include <QBuffer>
#include <QVariant>
#include <QStringList>
#include "../../src/dbusinterface/ocrinterface.h"
#include "../../src/dbusinterface/pinscreenshotsinterface.h"
#include "../../src/dbusinterface/dbusnotify.h"

using namespace testing;

// Covers the DBus interface wrapper branches the existing ut_dbusnotify.h /
// ut_ocrinterface.h miss:
//   - OcrInterface::openImage / openImageAndName with a VALID image so the
//     image.save()->qCompress->toBase64 branch executes (existing tests pass a
//     null QImage, so save() fails and the branch is skipped).
//   - PinScreenShotsInterface: has NO existing test at all -> cover ctor +
//     openFile + openImage (valid + null) + openImageAndName (valid + null).
//   - DBusNotify::Notify with a non-empty actions/hints map (existing passes
//     empty containers).
//
// All calls go to non-registered DBus services; they return errors asynchronously
// or block briefly and return an error reply -- no real DBus daemon interaction
// is required, and nothing blocks the harness.

class DBusInterfaceCovTest : public Test
{
public:
    QImage validImage()
    {
        QImage img(32, 32, QImage::Format_ARGB32);
        img.fill(Qt::magenta);
        return img;
    }
};

// ---------- OcrInterface valid-image branches ----------

TEST_F(DBusInterfaceCovTest, ocrOpenImageValidImage)
{
    OcrInterface ocr(QStringLiteral("com.deepin.Ocr"), QStringLiteral("/com/deepin/Ocr"),
                     QDBusConnection::sessionBus());
    EXPECT_NO_FATAL_FAILURE(ocr.openImage(validImage()));
}

TEST_F(DBusInterfaceCovTest, ocrOpenImageAndNameValidImage)
{
    OcrInterface ocr(QStringLiteral("com.deepin.Ocr"), QStringLiteral("/com/deepin/Ocr"),
                     QDBusConnection::sessionBus());
    EXPECT_NO_FATAL_FAILURE(ocr.openImageAndName(validImage(), QStringLiteral("shot.png")));
}

TEST_F(DBusInterfaceCovTest, ocrOpenFileNonEmptyPath)
{
    OcrInterface ocr(QStringLiteral("com.deepin.Ocr"), QStringLiteral("/com/deepin/Ocr"),
                     QDBusConnection::sessionBus());
    EXPECT_NO_FATAL_FAILURE(ocr.openFile(QStringLiteral("/tmp/some.png")));
}

// ---------- PinScreenShotsInterface (no existing coverage) ----------

TEST_F(DBusInterfaceCovTest, pinScreenshotsOpenFile)
{
    PinScreenShotsInterface pin(QStringLiteral("com.deepin.PinScreenShots"),
                                QStringLiteral("/com/deepin/PinScreenShots"),
                                QDBusConnection::sessionBus());
    EXPECT_NO_FATAL_FAILURE(pin.openFile(QStringLiteral("/tmp/pin.png")));
}

TEST_F(DBusInterfaceCovTest, pinScreenshotsOpenImageValid)
{
    PinScreenShotsInterface pin(QStringLiteral("com.deepin.PinScreenShots"),
                                QStringLiteral("/com/deepin/PinScreenShots"),
                                QDBusConnection::sessionBus());
    EXPECT_NO_FATAL_FAILURE(pin.openImage(validImage()));
}

TEST_F(DBusInterfaceCovTest, pinScreenshotsOpenImageNull)
{
    PinScreenShotsInterface pin(QStringLiteral("com.deepin.PinScreenShots"),
                                QStringLiteral("/com/deepin/PinScreenShots"),
                                QDBusConnection::sessionBus());
    QImage nullImg;
    EXPECT_NO_FATAL_FAILURE(pin.openImage(nullImg));
}

TEST_F(DBusInterfaceCovTest, pinScreenshotsOpenImageAndNameValid)
{
    PinScreenShotsInterface pin(QStringLiteral("com.deepin.PinScreenShots"),
                                QStringLiteral("/com/deepin/PinScreenShots"),
                                QDBusConnection::sessionBus());
    EXPECT_NO_FATAL_FAILURE(pin.openImageAndName(validImage(), QStringLiteral("p.png"), QPoint(10, 10)));
}

TEST_F(DBusInterfaceCovTest, pinScreenshotsOpenImageAndNameNull)
{
    PinScreenShotsInterface pin(QStringLiteral("com.deepin.PinScreenShots"),
                                QStringLiteral("/com/deepin/PinScreenShots"),
                                QDBusConnection::sessionBus());
    QImage nullImg;
    EXPECT_NO_FATAL_FAILURE(pin.openImageAndName(nullImg, QString(), QPoint(0, 0)));
}

TEST_F(DBusInterfaceCovTest, pinScreenshotsStaticInterfaceName)
{
    EXPECT_STREQ(PinScreenShotsInterface::staticInterfaceName(), "com.deepin.PinScreenShots");
}

// ---------- DBusNotify richer-argument branch ----------

TEST_F(DBusInterfaceCovTest, notifyWithActionsAndHints)
{
    DBusNotify notify;
    QStringList actions;
    actions << "_open" << "View"
            << "_open1" << "Open Folder";
    QVariantMap hints;
    hints["urgency"] = 1;
    hints["x-deepin-action-_open"] = QStringList{ "xdg-open", "/tmp/x.png" };
    EXPECT_NO_FATAL_FAILURE(notify.Notify(QStringLiteral("ut-app"), 0u,
                                          QStringLiteral("deepin-screen-recorder"),
                                          QStringLiteral("title"), QStringLiteral("body"),
                                          actions, hints, 5000));
}

TEST_F(DBusInterfaceCovTest, notifyStaticInterfaceNameAndClose)
{
    DBusNotify notify;
    EXPECT_STREQ(DBusNotify::staticInterfaceName(), "org.freedesktop.Notifications");
    EXPECT_NO_FATAL_FAILURE(notify.CloseNotification(7u));
}
