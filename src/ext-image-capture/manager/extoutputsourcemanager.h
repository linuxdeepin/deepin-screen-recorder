// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTOUTPUTSOURCEMANAGER_H
#define EXTOUTPUTSOURCEMANAGER_H

#include <QObject>
#include <QWaylandClientExtension>

// 前向声明 Wayland 类型
struct wl_output;

/**
 * @brief ext-output-image-capture-source-manager协议管理器
 * 
 * 负责为wl_output创建ext_image_capture_source_v1对象
 */
class ExtOutputSourceManager : public QWaylandClientExtension
{
    Q_OBJECT

public:
    explicit ExtOutputSourceManager(QObject *parent = nullptr);
    ~ExtOutputSourceManager();

    /**
     * @brief 检查协议是否可用
     */
    bool isProtocolAvailable() const;

    /**
     * @brief 为输出创建图像捕获源
     * @param output Wayland输出对象
     * @return 图像捕获源对象，失败返回nullptr
     */
    void* createSourceForOutput(wl_output *output);

    /**
     * @brief 获取协议版本
     */
    int protocolVersion() const;

signals:
    /**
     * @brief 协议变为可用时发出
     */
    void protocolAvailable();

    /**
     * @brief 协议变为不可用时发出
     */
    void protocolUnavailable();

protected:
    // QWaylandClientExtension interface
    const wl_interface *extensionInterface() const override;
    void bind(wl_registry *registry, int id, int version) override;

private slots:
    void onActiveChanged();

private:
    class Private;
    Private *d;
};

#endif // EXTOUTPUTSOURCEMANAGER_H
