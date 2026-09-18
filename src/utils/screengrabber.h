// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENGRABBER_H
#define SCREENGRABBER_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QList>
class QScreen;

class ScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject *parent = nullptr);
    QPixmap grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio);

private:
    QPixmap grabWaylandScreenshot(bool &ok, const QRect &rect, const qreal devicePixelRatio);
    QPixmap grabX11Screenshot(bool &ok, const QRect &rect, const qreal devicePixelRatio);
    QList<QScreen*> findIntersectingScreens(const QRect &rect);
    QPixmap grabPrimaryScreenFallback(bool &ok, const QRect &rect, const qreal devicePixelRatio);
    QPixmap grabSingleScreen(bool &ok, const QRect &rect, QScreen *screen, const qreal devicePixelRatio);
    QPixmap grabMultipleScreens(bool &ok, const QRect &rect, const QList<QScreen*> &screens, const qreal devicePixelRatio);
    QPixmap grabScreenFragment(QScreen *screen, const QRect &intersection, const qreal devicePixelRatio);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 X11 workaround: 使用 XGetImage 绕过 grabWindow 的 bug（运行时再判断是否 XCB）
    QPixmap grabWithXGetImage(bool &ok, const QRect &rect);
#endif

public:
    
    // 获取 X11 根窗口的物理像素大小（Qt6+XCB 环境下使用）
    static QSize getX11RootWindowSize();
};

#endif // SCREENGRABBER_H
