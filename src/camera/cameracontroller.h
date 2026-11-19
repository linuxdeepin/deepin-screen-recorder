// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QImage>
#include <QList>
#include <QObject>
#include <QString>

struct CameraDeviceDescriptor
{
    QString id;
    QString description;
    bool isDefault = false;
};

/**
 * @brief CameraController 摄像头采集后端接口
 *
 * CameraWidget 只依赖该接口，便于使用 Qt Multimedia 实现生产后端，
 * 并在单元测试中注入 Fake 后端。
 */
class CameraController : public QObject
{
    Q_OBJECT
public:
    explicit CameraController(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~CameraController() override = default;

    virtual QList<CameraDeviceDescriptor> availableDevices() const = 0;
    virtual bool start(const QString &deviceId) = 0;
    virtual void stop() = 0;
    virtual bool isActive() const = 0;
    virtual QString currentDeviceId() const = 0;

signals:
    void frameReady(const QImage &image);
    void errorOccurred(const QString &message);
};

#endif // CAMERACONTROLLER_H
