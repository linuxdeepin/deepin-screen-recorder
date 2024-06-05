// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "libdscreen_recorder_global.h"

#include <QUrl>
#include <QMap>
#include <QObject>
#include <QImage>

class LIBDSCREENRECORDERSHARED_EXPORT SaveManager : public QObject
{
    Q_OBJECT

public:
    enum ExportAction {
        NoActions   = 0b00000,
        Save        = 0b00001,
        SaveAs      = 0b00010,
        CopyImage   = 0b00100,
        CopyPath    = 0b01000,
        UserAction  = 0b10000,
        AnySave     = Save | SaveAs,
        AnyAction   = AnySave | CopyImage | CopyPath,
    };
    Q_DECLARE_FLAGS(ExportActions, ExportAction)
    Q_FLAG(ExportAction)

    static SaveManager *instance();

    void setImage(const QImage &image);
    QImage image() const;
    void saveImage(ExportActions actions, QUrl url = {});
    void saveVideo(ExportActions actions, const QUrl &inputUrl, QUrl outputUrl = {});

Q_SIGNALS:
    void imageChanged();
    void errorMessage(const QString &str);

private:
    explicit SaveManager(QObject *parent = nullptr);
    ~SaveManager() override;

    SaveManager(SaveManager const &) = delete;
    void operator=(SaveManager const &) = delete;

    bool m_imageSavedNotInTemp;
    QImage m_saveImage;
    QUrl m_tempFile;
    QList<QUrl> m_usedTempFileNames;
};
