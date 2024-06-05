// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "libdscreen_recorder_global.h"

class LIBDSCREENRECORDERSHARED_EXPORT Utils
{
public:
    // 检测是否开启Wayland
    static bool checkWaylandMode();
    // 设置Wayland
    static void setWaylandMode(bool mode);
    // 是否开启Wayland
    static bool isWaylandMode();
    // 检测是否存在FFmpeg
    static bool checkFFmpegEnv();
    // 设置Wayland
    static void setFFmpegMode(bool mode);
    // 是否开启Wayland
    static bool isFFmpegMode();

private:
    explicit Utils() = default;
    ~Utils() = default;
};
