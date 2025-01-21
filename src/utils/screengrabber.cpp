// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screengrabber.h"

#include "../utils.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QStandardPaths>

ScreenGrabber::ScreenGrabber(QObject *parent) : QObject(parent)
{

}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    ok = true;
    if (Utils::isWaylandMode) {
        QRect recordRect{
            static_cast<int>(rect.x() * devicePixelRatio),
            static_cast<int>(rect.y() * devicePixelRatio),
            static_cast<int>(rect.width() * devicePixelRatio),
            static_cast<int>(rect.height() * devicePixelRatio)
        };
        QPixmap res;
        QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                     QStringLiteral("/Screenshot"),
                                     QStringLiteral("org.kde.kwin.Screenshot"));
        QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
        res = QPixmap(reply.value());
        if (!res.isNull()) {
            QFile dbusResult(reply.value());
            dbusResult.remove();
        } else {
            ok = false;
        }
        return res.copy(recordRect);
    }

    // Qt6中不再使用QDesktop，直接传0作为窗口ID来捕获整个屏幕
    QScreen *screen = QGuiApplication::screenAt(QPoint(0, 0));
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    if (screen) {
        return screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    }

    return {};
}
