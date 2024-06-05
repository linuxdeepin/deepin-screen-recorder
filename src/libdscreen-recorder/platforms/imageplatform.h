// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFlags>
#include <QImage>
#include <QObject>

#include "libdscreen_recorder_global.h"

class LIBDSCREENRECORDERSHARED_EXPORT ImagePlatform : public QObject
{
    Q_OBJECT

public:
    enum GrabMode {
        NoGrabModes =           0b0000000,
        AllScreens =            0b0000001,
        ActiveWindow =          0b0000100,
    };

    explicit ImagePlatform(QObject *parent = nullptr);
    ~ImagePlatform() override = default;

public Q_SLOTS:
    virtual void grab(ImagePlatform::GrabMode grabMode, bool includePointer) = 0;

Q_SIGNALS:
    void newScreenshotTaken(const QImage &image = {});
    void newScreenshotFailed(const QString &message = {});
};