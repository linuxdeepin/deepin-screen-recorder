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
#include <QDebug>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QThread>
ScreenGrabber::ScreenGrabber(QObject *parent)
    : QObject(parent)
{
}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    ok = true;
    if (Utils::isWaylandMode) {
        int count = 0;
        QRect recordRect {
            static_cast<int>(rect.x() * devicePixelRatio),
            static_cast<int>(rect.y() * devicePixelRatio),
            static_cast<int>(rect.width() * devicePixelRatio),
            static_cast<int>(rect.height() * devicePixelRatio)
        };
        QPixmap res;
        QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                     QStringLiteral("/Screenshot"),
                                     QStringLiteral("org.kde.kwin.Screenshot"));
        QDBusReply<QString> reply;
        while (count < 6) {
            reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
            res = QPixmap(reply.value());
            if (!res.isNull()) {
                break;
            }
            count++;
            QThread::msleep(50);
        }
        if (!res.isNull()) {
            QFile dbusResult(reply.value());
            dbusResult.remove();
        } else {
            ok = false;
            qDebug() << __FUNCTION__ << __LINE__ << "Get Pixmap:" << res.size() << "try failed at " << count << "times";
        }
        return res.copy(recordRect);
    }

    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    return t_primaryScreen->grabWindow(QApplication::desktop()->winId(), rect.x(), rect.y(), rect.width(), rect.height());
}
