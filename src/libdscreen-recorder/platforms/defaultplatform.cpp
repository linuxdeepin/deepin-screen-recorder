/* This file is part of Spectacle, the KDE screenshot utility
 * SPDX-FileCopyrightText: 2019 Boudhayan Gupta <bgupta@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "defaultplatform.h"

#include <QDebug>
#include <QPixmap>

/* -- Null Platform ---------------------------------------------------------------------------- */

ImageDefaultPlatform::ImageDefaultPlatform(QObject *parent)
    : ImagePlatform(parent)
{
}

void ImageDefaultPlatform::grab( GrabMode grabMode, bool includePointer)
{
    Q_UNUSED(grabMode)
    Q_UNUSED(includePointer)
}

VideoDefaultPlatform::VideoDefaultPlatform(const QString &unavailableMessage, QObject *parent)
    : VideoPlatform(parent)
{
}


void VideoDefaultPlatform::startRecording(const QUrl &fileUrl, RecordingMode mode, const QVariantMap &options, bool withPointer)
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(mode)
    Q_UNUSED(options)
    Q_UNUSED(withPointer)
}

void VideoDefaultPlatform::finishRecording()
{
    setRecording(false);
}

