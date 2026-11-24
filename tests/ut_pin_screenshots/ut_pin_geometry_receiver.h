// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PIN_GEOMETRY_RECEIVER_H
#define UT_PIN_GEOMETRY_RECEIVER_H

#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>

// Stands in for the PinScreenShots owner object and records what the adaptor
// forwards after it accepts a file descriptor.
class PinGeometryReceiver : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE bool openImageAtGeometry(const QImage &image, const QString &name,
                                         const QRect &geometry)
    {
        ++calls;
        receivedImage = image;
        receivedName = name;
        receivedGeometry = geometry;
        return true;
    }

    int calls = 0;
    QImage receivedImage;
    QString receivedName;
    QRect receivedGeometry;
};

#endif // UT_PIN_GEOMETRY_RECEIVER_H
