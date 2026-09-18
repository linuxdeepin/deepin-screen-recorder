// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tempfile.h"

#include <QTemporaryFile>
#include <QDebug>
#include "log.h"

TempFile *TempFile::instance()
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Getting TempFile instance.";
    static TempFile *tempFile = new TempFile;
    return tempFile;
}

TempFile::TempFile(QObject *parent)
    : QObject(parent)
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "TempFile object initialized.";
}

TempFile::~TempFile()
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "TempFile destructor called.";
}

void TempFile::setFullScreenPixmap(const QPixmap &pixmap)
{
    m_fullscreenPixmap = pixmap;
    qCDebug(dsrApp) << Q_FUNC_INFO << "Fullscreen pixmap set.";
}

void TempFile::setBlurPixmap(const QPixmap &pixmap, const int radius)
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Setting blur pixmap for radius:" << radius;
    if (m_blurPixmap.contains(radius)) {
        qCDebug(dsrApp) << "Blur pixmap already contains radius:" << radius << ", returning.";
        return;
    }
    m_blurPixmap[radius] = pixmap;
    qCDebug(dsrApp) << "Blur pixmap added for radius:" << radius;
}

void TempFile::setMosaicPixmap(const QPixmap &pixmap, const int radius)
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Setting mosaic pixmap for radius:" << radius;
    if(m_mosaicPixmap.contains(radius)) {
        qCDebug(dsrApp) << "Mosaic pixmap already contains radius:" << radius << ", returning.";
        return;
    }
    m_mosaicPixmap[radius] = pixmap;
    qCDebug(dsrApp) << "Mosaic pixmap added for radius:" << radius;
}

bool TempFile::isContainsPix(const QString effect, const int radius)
{
    qCDebug(dsrApp) << Q_FUNC_INFO << "Checking if pixmap contains effect:" << effect << ", radius:" << radius;
    if (effect == "blur") {
        bool contains = m_blurPixmap.contains(radius);
        qCDebug(dsrApp) << "Effect is blur, contains:" << contains;
        return contains;
    } else {
        bool contains = m_mosaicPixmap.contains(radius);
        qCDebug(dsrApp) << "Effect is mosaic, contains:" << contains;
        return contains;
    }
}
