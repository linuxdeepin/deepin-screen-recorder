/* This file is part of Spectacle, the KDE screenshot utility
 * SPDX-FileCopyrightText: 2019 Boudhayan Gupta <bgupta@kde.org>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "imageplatform.h"
#include "videoplatform.h"

class ImageDefaultPlatform final : public ImagePlatform
{
    Q_OBJECT

public:
    explicit ImageDefaultPlatform(QObject *parent = nullptr);
    ~ImageDefaultPlatform() override = default;

    public Q_SLOTS:

    void grab( ImagePlatform::GrabMode grabMode, bool includePointer) override final;
};

class VideoDefaultPlatform final : public VideoPlatform
{
    Q_OBJECT

public:
    explicit VideoDefaultPlatform(const QString &unavailableMessage = {}, QObject *parent = nullptr);
    ~VideoDefaultPlatform() override = default;

    void startRecording(const QUrl &fileUrl, RecordingMode recordingMode, const QVariantMap &options, bool includePointer) override;
    void finishRecording() override;
};
