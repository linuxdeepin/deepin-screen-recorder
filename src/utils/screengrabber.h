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
     * @brief 通过 KWin ScreenShot2 接口按 native-resolution 抓取当前激活屏的
     *        原生分辨率帧。分数缩放下 kwin screenshotFullscreen 返回的是原生帧
     *        左下角裁剪图（内容被放大），需改走此接口
     * @param ok 抓取成功返回 true
     * @return 原生分辨率图像，失败返回空 QPixmap
     */
    static QPixmap grabNativeFramebuffer(bool &ok);
    
    /**
     * @brief 快速全屏截图，绕过MainWindow初始化，避免S3/S4问题
     * @return 截图成功返回true，失败返回false
     */
    static bool quickFullScreenshot();
};

#endif // SCREENGRABBER_H
