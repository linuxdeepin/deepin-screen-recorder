// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAMERAAVAILABILITYPROBE_H
#define CAMERAAVAILABILITYPROBE_H

#include <QString>
#include <QStringList>

/**
 * @brief 轻量 V4L2 摄像头可用性探测
 *
 * 仅使用 Linux V4L2 UAPI 判断设备节点是否存在、是否可打开以及是否具备
 * 视频捕获能力，不依赖任何三方源码。
 */
class CameraAvailabilityProbe
{
public:
    static bool isDeviceAvailable(const QString &deviceId,
                                  const QString &description = QString());
    static bool probeDevicePath(const QString &devicePath);

private:
    static QStringList videoDevicePaths();
    static QString resolveDevicePath(const QString &deviceId,
                                     const QString &description);
};

#endif // CAMERAAVAILABILITYPROBE_H
