// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tempfile.h"

#include <QTemporaryFile>
#include <QDebug>

TempFile *TempFile::instance()
{
    static TempFile *tempFile = new TempFile;
    return tempFile;
}

TempFile::TempFile(QObject *parent)
    : QObject(parent)
{
}

TempFile::~TempFile()
{
}

void TempFile::setFullScreenPixmap(const QPixmap &pixmap)
{
    m_fullscreenPixmap = pixmap;
}

void TempFile::setBlurPixmap(const QPixmap &pixmap)
{
    m_blurPixmap = pixmap;
}

void TempFile::setMosaicPixmap(const QPixmap &pixmap)
{
    m_mosaicPixmap = pixmap;
}
