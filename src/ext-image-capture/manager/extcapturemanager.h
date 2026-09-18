// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTCAPTUREMANAGER_H
#define EXTCAPTUREMANAGER_H

#include <QObject>
#include <QWaylandClientExtension>
#include <QScreen>

// 前向声明 Wayland 类型
struct wl_output;

class ExtCaptureSession;
class ExtOutputSourceManager;

/**
 * @brief ext-image-copy-capture协议管理器
 * 
 * 负责初始化协议、管理全局状态和创建捕获会话
 */
class ExtCaptureManager : public QWaylandClientExtension
{
    Q_OBJECT

public:
    explicit ExtCaptureManager(QObject *parent = nullptr);
    ~ExtCaptureManager();

    /**
     * @brief 检查协议是否可用
     */
    bool isProtocolAvailable() const;

    /**
     * @brief 创建屏幕捕获会话
     * @param screen 要捕获的屏幕
     * @param paintCursors 是否包含光标
     * @return 捕获会话对象，失败返回nullptr
     */
    ExtCaptureSession* createScreenCaptureSession(QScreen *screen, bool paintCursors = false);

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
    void setupWaylandIntegration();
    wl_output* getWaylandOutput(QScreen *screen);

    class Private;
    Private *d;
};

#endif // EXTCAPTUREMANAGER_H
