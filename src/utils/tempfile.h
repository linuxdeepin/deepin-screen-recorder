// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPFILE_H
#define TEMPFILE_H

#include <QObject>
#include <QWindow>

class TempFile : public QObject
{
    Q_OBJECT
public:
    static TempFile *instance();

public slots:
    inline const QPixmap getFullscreenPixmap() const
    {
        return m_fullscreenPixmap;
    }
    inline const QPixmap getMosaicPixmap() const
    {
        return m_mosaicPixmap;
    }
    inline const QPixmap getBlurPixmap() const
    {
        return m_blurPixmap;
    }

    void setFullScreenPixmap(const QPixmap &pixmap);
    void setBlurPixmap(const QPixmap &pixmap);
    void setMosaicPixmap(const QPixmap &pixmap);

private:
    explicit TempFile(QObject *parent = 0);
    ~TempFile();

    QPixmap m_fullscreenPixmap;
    QPixmap m_mosaicPixmap;
    QPixmap m_blurPixmap;
};
#endif // TEMPFILE_H
