// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cameraavailabilityprobe.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

QStringList CameraAvailabilityProbe::videoDevicePaths()
{
    QStringList paths;
    const QDir devDir(QStringLiteral("/dev"));
    const QStringList entries = devDir.entryList(
            QStringList() << QStringLiteral("video*"),
            QDir::System | QDir::Files,
            QDir::Name);

    for (const QString &entry : entries) {
        const QString path = devDir.absoluteFilePath(entry);
        if (QFileInfo(path).exists())
            paths.append(path);
    }

    return paths;
}

QString CameraAvailabilityProbe::resolveDevicePath(const QString &deviceId,
                                                   const QString &description)
{
    if (deviceId.startsWith(QStringLiteral("/dev/")))
        return deviceId;

    // Qt 的 QCameraDevice::id() 在部分平台不是设备节点，此时通过 sysfs
    // 的设备名称与 QCameraDevice::description() 做一次映射。
    const QStringList paths = videoDevicePaths();
    for (const QString &path : paths) {
        const QString namePath = QStringLiteral("/sys/class/video4linux/%1/name")
                                         .arg(QFileInfo(path).fileName());
        QFile nameFile(namePath);
        if (!nameFile.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        const QString cameraName = QString::fromLocal8Bit(nameFile.readAll()).trimmed();
        if (!description.isEmpty()
                && (cameraName == description
                    || cameraName.contains(description, Qt::CaseInsensitive)
                    || description.contains(cameraName, Qt::CaseInsensitive))) {
            return path;
        }
    }

    return {};
}

bool CameraAvailabilityProbe::probeDevicePath(const QString &devicePath)
{
    if (devicePath.isEmpty())
        return false;

    const QByteArray path = QFile::encodeName(devicePath);
    const int fd = ::open(path.constData(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0)
        return false;

    struct v4l2_capability capability;
    memset(&capability, 0, sizeof(capability));
    const int result = ::ioctl(fd, VIDIOC_QUERYCAP, &capability);
    ::close(fd);

    if (result < 0)
        return false;

    const quint32 caps = (capability.capabilities & V4L2_CAP_DEVICE_CAPS)
            ? capability.device_caps
            : capability.capabilities;

    return (caps & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE)) != 0;
}

bool CameraAvailabilityProbe::isDeviceAvailable(const QString &deviceId,
                                                const QString &description)
{
    const QString resolvedPath = resolveDevicePath(deviceId, description);
    if (!resolvedPath.isEmpty())
        return probeDevicePath(resolvedPath);

    // 无法映射到具体节点时，只要系统存在至少一个可打开的 V4L2 捕获设备，
    // 就认为摄像头可用。摄像头具体选择仍由 Qt Multimedia 完成。
    const QStringList paths = videoDevicePaths();
    for (const QString &path : paths) {
        if (probeDevicePath(path))
            return true;
    }

    return false;
}
