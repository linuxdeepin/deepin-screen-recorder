// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbuspinscreenshotsadaptor.h"
#include <QtCore/QMetaObject>
#include <QImage>
#include "../../utils/log.h"

DbusPinScreenShotsAdaptor::DbusPinScreenShotsAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(dsrApp) << "DbusPinScreenShotsAdaptor constructor called.";
    // constructor
    setAutoRelaySignals(true);
    qCDebug(dsrApp) << "Auto relay signals set to true.";
}

DbusPinScreenShotsAdaptor::~DbusPinScreenShotsAdaptor()
{
    qCDebug(dsrApp) << "DbusPinScreenShotsAdaptor destructor called.";
    // destructor
}

bool DbusPinScreenShotsAdaptor::openFile(QString filePath)
{
    qCDebug(dsrApp) << "Received DBus call to open file:" << filePath;
    QMetaObject::invokeMethod(parent(), "openFile", Q_ARG(QString, filePath));
    qCDebug(dsrApp) << "Invoked openFile method on parent object.";
    return true;
}


void DbusPinScreenShotsAdaptor::openImageAndName(QByteArray images, QString imageName, QPoint point)
{
    qCDebug(dsrApp) << "Received DBus call to open image with name and point. Image name:" << imageName << ", Point:" << point;
    QByteArray data = images;
    QString tmp_data = QString::fromLatin1(data.data(), data.size());
    QByteArray srcData = QByteArray::fromBase64(tmp_data.toLatin1());
    data = qUncompress(srcData);
    QImage image;
    image.loadFromData(data);
    QMetaObject::invokeMethod(parent(), "openImageAndName", Q_ARG(QImage, image), Q_ARG(QString, imageName), Q_ARG(QPoint, point));
    qCDebug(dsrApp) << "Invoked openImageAndName method on parent object.";
}

void DbusPinScreenShotsAdaptor::openImage(QByteArray images)
{
    qCDebug(dsrApp) << "Received DBus call to open image.";
    QByteArray data = images;
    QString tmp_data = QString::fromLatin1(data.data(), data.size());
    QByteArray srcData = QByteArray::fromBase64(tmp_data.toLatin1());
    data = qUncompress(srcData);
    QImage image;
    image.loadFromData(data);
    QMetaObject::invokeMethod(parent(), "openImage", Q_ARG(QImage, image));
    qCDebug(dsrApp) << "Invoked openImage method on parent object.";
}
