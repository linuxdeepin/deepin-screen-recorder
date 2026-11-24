// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbuspinscreenshotsadaptor.h"
#include <QtCore/QMetaObject>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QRect>
#include "../../utils/log.h"

#include <fcntl.h>
#include <sys/stat.h>

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

bool DbusPinScreenShotsAdaptor::openImageAtGeometry(const QDBusUnixFileDescriptor &descriptor,
                                                   const QString &imageName, const QRect &geometry)
{
    if (!descriptor.isValid() || !geometry.isValid())
        return false;

    const int fd = descriptor.fileDescriptor();
    const int seals = fcntl(fd, F_GET_SEALS);
    constexpr int RequiredSeals = F_SEAL_WRITE | F_SEAL_GROW | F_SEAL_SHRINK;
    struct stat info {};
    if (seals < 0 || (seals & RequiredSeals) != RequiredSeals
            || fstat(fd, &info) != 0 || !S_ISREG(info.st_mode)
            || info.st_size <= 0) {
        return false;
    }

    // The received descriptor shares the sender's offset, which may be at EOF.
    QFile file;
    if (!file.open(fd, QIODevice::ReadOnly, QFileDevice::DontCloseHandle) || !file.seek(0))
        return false;
    QImageReader reader(&file, "PNG");
    const QImage image = reader.read();
    return !image.isNull() && QMetaObject::invokeMethod(parent(), "openImageAtGeometry",
                            Q_ARG(QImage, image), Q_ARG(QString, imageName), Q_ARG(QRect, geometry));
}

#ifdef DSR_HAVE_TABLE_RECOGNIZER
void DbusPinScreenShotsAdaptor::openImageAndNameForTable(QByteArray images, QString imageName, QPoint point)
{
    qCDebug(dsrApp) << "Received DBus call to open image for table recognition. Image name:" << imageName << ", Point:" << point;
    QByteArray data = images;
    QString tmp_data = QString::fromLatin1(data.data(), data.size());
    QByteArray srcData = QByteArray::fromBase64(tmp_data.toLatin1());
    data = qUncompress(srcData);
    QImage image;
    image.loadFromData(data);
    QMetaObject::invokeMethod(parent(), "openImageForTable", Q_ARG(QImage, image), Q_ARG(QString, imageName), Q_ARG(QPoint, point));
    qCDebug(dsrApp) << "Invoked openImageForTable method on parent object.";
}
#endif

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
