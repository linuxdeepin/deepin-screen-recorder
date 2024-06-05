// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFlags>
#include <QImage>
#include <QObject>

#include "libdscreen_recorder_global.h"

class LIBDSCREENRECORDERSHARED_EXPORT VideoPlatform : public QObject
{
    Q_OBJECT

public:
    enum RecordingMode : char {
        NoRecordingModes = 0b000,
        FullScreen =       0b001, //< records a specific output, provided its QScreen::name()
        Window =           0b010, //< records a specific window, provided its uuid
        Region =           0b100, //< records the provided region rectangle
    };

    explicit VideoPlatform(QObject *parent = nullptr);
    ~VideoPlatform() override = default;

protected:
    void setRecording(bool recording);

public Q_SLOTS:
    virtual void startRecording(const QUrl &fileUrl,
                                VideoPlatform::RecordingMode recordingMode,
                                const QVariantMap &options,
                                bool includePointer) = 0;
    virtual void finishRecording() = 0;
    
Q_SIGNALS:
    void recordingSaved(const QUrl &fileUrl);
    void recordingFailed(const QString &message);
    void recordingCanceled(const QString &message);
};