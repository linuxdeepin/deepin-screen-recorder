// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENGRABBER_H
#define SCREENGRABBER_H

#include <QObject>

class ScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject *parent = nullptr);
    QPixmap grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio);
    
    /**
     * @brief 快速全屏截图，绕过MainWindow初始化，避免S3/S4问题
     * @return 截图成功返回true，失败返回false
     */
    static bool quickFullScreenshot();
};

#endif // SCREENGRABBER_H
