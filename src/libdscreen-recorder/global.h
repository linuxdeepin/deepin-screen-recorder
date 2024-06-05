// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "libdscreen_recorder_global.h"

#include <QObject>

class QPointF;
class QRectF;
class QSizeF;

enum class RecorderMode {
    NoGrabModes =           0b0000000,
    Screenshot =            0b0000001,
    DelayScreenshot =       0b0000001,
    FullScreenScreenshot =  0b0000010,
    TopWindowScreenshot =   0b0000100,
    NoNotifyScreenshot =    0b0001000,
};

class LIBDSCREENRECORDERSHARED_EXPORT Global : public QObject
{
    Q_OBJECT
public:
    static Global *instance();

    Q_INVOKABLE static qreal dpx(qreal dpr);
    Q_INVOKABLE static qreal dprRound(qreal value, qreal dpr);
    Q_INVOKABLE static qreal dprCeil(qreal value, qreal dpr);
    Q_INVOKABLE static qreal dprFloor(qreal value, qreal dpr);

    Q_INVOKABLE static qreal mapFromPlatformValue(qreal value, qreal dpr);
    Q_INVOKABLE static QPointF mapFromPlatformPoint(const QPointF &point, qreal dpr);
    Q_INVOKABLE static QRectF mapFromPlatformRect(const QRectF &rect, qreal dpr);

    Q_INVOKABLE static QRectF logicalScreensRect();

    Q_INVOKABLE static qreal mapToPlatformValue(qreal value, qreal dpr);
    Q_INVOKABLE static QPointF mapToPlatformPoint(const QPointF &point, qreal dpr);
    Q_INVOKABLE static QRectF mapToPlatformRect(const QRectF &rect, qreal dpr);

    Q_INVOKABLE static QRectF platformUnifiedRect();
    Q_INVOKABLE static QSize rawSize(const QSizeF &size, qreal dpr);

    Q_INVOKABLE static QRectF rectNormalized(const QRectF &rect);
    Q_INVOKABLE static QRectF rectNormalized(qreal x, qreal y, qreal w, qreal h);
    Q_INVOKABLE static QRectF rectAdjusted(const QRectF &rect, qreal xp1, qreal yp1, qreal xp2, qreal yp2);
    Q_INVOKABLE static QRectF rectAdjustedVisually(const QRectF &rect, qreal xp1, qreal yp1, qreal xp2, qreal yp2);

    Q_INVOKABLE static QRectF rectScaled(const QRectF &rect, qreal scale);
    Q_INVOKABLE static QRectF rectIntersected(const QRectF &rect1, const QRectF &rect2);

    Q_INVOKABLE static QRectF rectBounded(const QRectF &rect, const QRectF &boundsRect, //
                                                        Qt::Orientations orientations = Qt::Horizontal | Qt::Vertical);
    Q_INVOKABLE static QRectF rectBounded(qreal x, qreal y, qreal width, qreal height, const QRectF &boundsRect, //
                                                        Qt::Orientations orientations = Qt::Horizontal | Qt::Vertical);


    Q_INVOKABLE static QRectF rectClipped(const QRectF &rect, const QRectF &clipRect, //
                                                        Qt::Orientations orientations = Qt::Horizontal | Qt::Vertical);
    Q_INVOKABLE static bool rectContains(const QRectF &rect, qreal v, //
                                                       Qt::Orientations orientations = Qt::Horizontal | Qt::Vertical);
    Q_INVOKABLE static bool rectContains(const QRectF &rect, qreal x, qreal y);
    Q_INVOKABLE static bool rectContains(const QRectF &rect, const QPointF& point);
    Q_INVOKABLE static bool rectContains(const QRectF &rect1, const QRectF& rect2);
    Q_INVOKABLE static bool rectContains(const QRectF &rect, qreal x, qreal y, qreal w, qreal h);

    Q_INVOKABLE static bool ellipseContains(qreal ellipseX, qreal ellipseY,
                                                          qreal ellipseWidth, qreal ellipseHeight,
                                                          qreal x, qreal y);
    Q_INVOKABLE static bool ellipseContains(const QRectF &rect, qreal x, qreal y);
    Q_INVOKABLE static bool ellipseContains(const QRectF &rect, const QPointF &point);

    Q_INVOKABLE static bool rectIntersects(const QRectF &rect1, const QRectF& rect2);
    Q_INVOKABLE static bool rectIntersects(const QRectF &rect, qreal x, qreal y, qreal w, qreal h);

    Q_INVOKABLE static bool isEmpty(qreal w, qreal h);
    Q_INVOKABLE static bool isEmpty(const QSizeF &size);
    Q_INVOKABLE static bool isEmpty(const QRectF &rect);

    Q_INVOKABLE static bool isNull(qreal w, qreal h);
    Q_INVOKABLE static bool isNull(const QSizeF &size);
    Q_INVOKABLE static bool isNull(const QRectF &rect);

private:
    explicit Global(QObject *parent = nullptr);
    Global(const Global &) = delete;
    Global(Global &&) = delete;
    Global &operator=(const Global &) = delete;
    Global &operator=(Global &&) = delete;

    static Global *s_self;
};
