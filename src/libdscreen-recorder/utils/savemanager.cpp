// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "savemanager.h"

#include <QFile>

SaveManager::SaveManager(QObject *parent)
    : QObject(parent)
    , m_imageSavedNotInTemp(false)
    , m_saveImage(QImage())
    , m_tempFile(QUrl())
{

}

SaveManager::~SaveManager() = default;

SaveManager *SaveManager::instance()
{
    static SaveManager instance;
    return &instance;
}

QImage SaveManager::image() const
{
    return m_saveImage;
}

void SaveManager::setImage(const QImage &image)
{
    m_saveImage = image;

    // reset our saved tempfile
    if (m_tempFile.isValid()) {
        m_usedTempFileNames.append(m_tempFile);
        QFile file(m_tempFile.toLocalFile());
        file.remove();
        m_tempFile = QUrl();
    }

    // since the image was modified, we now consider the image unsaved
    m_imageSavedNotInTemp = false;

    Q_EMIT imageChanged();
}

void SaveManager::saveImage(ExportActions actions, QUrl url)
{
    if (m_saveImage.isNull() && actions & (Save | SaveAs | CopyImage)) {
        Q_EMIT errorMessage("Cannot save an empty screenshot image.");
        return;
    }
}

void SaveManager::saveVideo(ExportActions actions, const QUrl &inputUrl, QUrl outputUrl)
{

}
