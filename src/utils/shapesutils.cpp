// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shapesutils.h"
#include "utils/log.h"

#include <QDebug>
#include <QPolygonF>

#include <algorithm>

Toolshape::Toolshape()
{
    qCDebug(dsrApp) << "Toolshape object initialized.";
    mainPoints.append(QPointF(0, 0));
    mainPoints.append(QPointF(0, 0));
    mainPoints.append(QPointF(0, 0));
    mainPoints.append(QPointF(0, 0));
    portion.clear();
}

QRectF toolshapeContentBounds(const Toolshape &shape)
{
    QPolygonF geometry;
    const bool usesMainPoints = shape.type == "rectangle"
            || shape.type == "oval"
            || shape.type == "text"
            || shape.type == "sequence"
            || shape.type == "pen"
            || (shape.type == "effect" && shape.isOval != 2);
    const bool usesPoints = shape.type == "arrow"
            || shape.type == "line"
            || shape.type == "pen"
            || (shape.type == "effect" && shape.isOval == 2);

    if (usesMainPoints) {
        geometry = QPolygonF(shape.mainPoints);
    }
    if (usesPoints) {
        for (const QPointF &point : shape.points)
            geometry.append(point);
    }
    if (!shape.arrowRotatePos.isNull())
        geometry.append(shape.arrowRotatePos);

    if (geometry.isEmpty()) {
        const bool hasNonZeroMainPoint = std::any_of(shape.mainPoints.cbegin(),
                                                     shape.mainPoints.cend(),
                                                     [](const QPointF &point) {
            return !point.isNull();
        });
        if (shape.type.isEmpty()
                || (!hasNonZeroMainPoint && shape.points.isEmpty()
                    && shape.arrowRotatePos.isNull())) {
            return QRectF();
        }
        // A single-point pen or sequence can legitimately start at (0, 0);
        // fall back to the stored point set instead of treating it as empty.
        geometry = QPolygonF(shape.points);
        if (geometry.isEmpty() && !shape.mainPoints.isEmpty())
            geometry = QPolygonF(shape.mainPoints);
    }
    if (geometry.isEmpty())
        return QRectF();

    QRectF bounds = geometry.boundingRect().normalized();
    const qreal effectiveLineWidth = qBound<qreal>(1.0, shape.lineWidth, 32.0);
    const qreal strokePadding = qMax<qreal>(2.0, effectiveLineWidth * 0.5 + 1.0);
    qreal padding = strokePadding;
    if (shape.type == "arrow")
        padding = qMax<qreal>(padding, 10.0 + shape.lineWidth);
    else if (shape.type == "text")
        padding = qMax<qreal>(padding, 4.0);
    return bounds.adjusted(-padding, -padding, padding, padding);
}

QImage effectAnnotationImage(const QImage &source, bool blur, int radius)
{
    if (source.isNull())
        return QImage();

    const int safeRadius = qMax(1, radius);
    const QSize targetSize = source.size();
    const QSize reducedSize(qMax(1, targetSize.width() / safeRadius),
                            qMax(1, targetSize.height() / safeRadius));
    QImage reduced = source.scaled(reducedSize,
                                   Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
    if (blur) {
        return reduced.scaled(targetSize,
                              Qt::IgnoreAspectRatio,
                              Qt::SmoothTransformation);
    }
    return reduced.scaled(targetSize,
                          Qt::IgnoreAspectRatio,
                          Qt::FastTransformation);
}

//Toolshape::~Toolshape()
//{
//}
/*
void Toolshape::registerMetaType()
{
    qRegisterMetaType<Toolshape>();
}

QDebug &operator<<(QDebug &argument, const Toolshape &obj)
{
    argument.nospace()
            << obj.type << ","
            << "[" << obj.mainPoints << "]" << ","
            << obj.index << ","
            << obj.lineWidth << ","
            << obj.colorIndex << ","
            << obj.isBlur << ","
            << obj.isMosaic << ","
            << obj.isStraight << ","
            << obj.isShiftPressed << ","
            << obj.fontSize << ","
            << obj.points;
    return argument.space();
}

QDataStream &operator>>(QDataStream &in, Toolshape &obj)
{
    in >> obj.points;
    in >> obj.fontSize;
    in >> obj.isShiftPressed;
    in >> obj.isBlur;
    in >> obj.isMosaic;
    in >> obj.isStraight;
    in >> obj.colorIndex;
    in >> obj.lineWidth;
    in >> obj.index;
    in >> obj.mainPoints;
    in >> obj.type;

    return in;
}

Toolshape Toolshape::operator=(Toolshape obj)
{
    type = obj.type;
    mainPoints = obj.mainPoints;
    index = obj.index;
    lineWidth = obj.lineWidth;
    colorIndex = obj.colorIndex;
    isBlur = obj.isBlur;
    isMosaic = obj.isMosaic;
    isStraight = obj.isStraight;
    isShiftPressed = obj.isShiftPressed;
    fontSize = obj.fontSize;
    points = obj.points;

    return (*this);
}

bool Toolshape::operator==(const Toolshape &other) const
{
    if (this->mainPoints == other.mainPoints && this->index == other.index
            && this->colorIndex == other.colorIndex && this->fontSize == other.fontSize
            && this->isBlur == other.isBlur && this->isMosaic == other.isMosaic
            && this->isShiftPressed == other.isShiftPressed
            && this->isStraight == other.isStraight && this->points == other.points) {
        return true;
    } else {
        return false;
    }
}
*/
