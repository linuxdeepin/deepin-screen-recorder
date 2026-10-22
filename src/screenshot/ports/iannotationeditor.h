// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IANNOTATION_EDITOR_H
#define IANNOTATION_EDITOR_H

#include <QObject>
#include <QRect>
#include <QRectF>

class QPainter;

namespace Dsr::Screenshot {

/**
 * Platform-independent annotation editing port.
 *
 * Implementations own their editing technology (for example ShapesWidget),
 * but all geometry crossing this boundary is in global logical coordinates.
 */
class IAnnotationEditor : public QObject
{
    Q_OBJECT
public:
    explicit IAnnotationEditor(QObject *parent = nullptr) : QObject(parent) {}
    ~IAnnotationEditor() override = default;

    virtual QRectF contentBoundsGlobal() const = 0;
    virtual bool isInteracting() const = 0;

    /**
     * Render annotations into the result image.
     *
     * The painter coordinate system is the result image. Implementations map
     * from globalSelection to image-local coordinates and apply pixelScale.
     */
    virtual void render(QPainter &painter,
                        const QRect &globalSelection,
                        qreal pixelScale) const = 0;

signals:
    void contentBoundsChanged(const QRectF &globalBounds);
};

} // namespace Dsr::Screenshot

#endif // IANNOTATION_EDITOR_H
