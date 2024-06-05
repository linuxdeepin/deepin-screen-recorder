// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "basewindow.h"

class RecorderWindow : public BaseWindow
{
    Q_OBJECT

public:
    /**
     * @brief 截图录屏工具存在的功能类型(0: record, 1: shot , 2: scrollshot , 3: ocr , 4: pinscreenshots)
     */
    enum Status {
        record = 0,
        shot,
        scrollshot,
        ocr,
        pinscreenshots
    };

    //滚动截图的滚动模式
    /**
     * @brief 滚动截图的滚动模式 (0: AutoScroll, 1: ManualScroll, 2: Unknow)
     */
    enum ScrollShotType {
        AutoScroll = 0, //自动滚动
        ManualScroll,  //手动滚动
        Unknow
    };

    explicit RecorderWindow(Status status, QScreen *screen, QQmlEngine *engine, QWindow *parent = nullptr);
    ~RecorderWindow();

private:
    void setStatus(RecorderWindow::Status status);

private:
    QPointer<QScreen> m_screenToFollow;
};
