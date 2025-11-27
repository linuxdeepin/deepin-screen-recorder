// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcapturemanager.h"
#include "../session/extcapturesession.h"
#include "extoutputsourcemanager.h"
#include "../../protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"
#include "../../utils/log.h"

#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <qpa/qplatformnativeinterface.h>

// Wayland 相关头文件
#include <wayland-client.h>

class ExtCaptureManager::Private : public QtWayland::ext_image_copy_capture_manager_v1
{
public:
    Private(ExtCaptureManager *q) : q_ptr(q), outputSourceManager(nullptr) {}

    ExtCaptureManager *q_ptr;
    bool protocolActive = false;
    int version = 0;
    ExtOutputSourceManager *outputSourceManager;
};

ExtCaptureManager::ExtCaptureManager(QObject *parent)
    : QWaylandClientExtension(1) // 协议版本
    , d(new Private(this))
{
    qCWarning(dsrApp) << "ExtCaptureManager: Creating manager...";
    
    // 注册 ext-image-copy-capture-manager-v1 协议
    initialize();
    
    // 创建 output source manager
    d->outputSourceManager = new ExtOutputSourceManager(this);
    
    qCWarning(dsrApp) << "ExtCaptureManager: Output source manager created";
    
    // 监听 output source manager 的协议状态变化
    connect(d->outputSourceManager, &ExtOutputSourceManager::protocolAvailable,
            this, [this]() {
                qCWarning(dsrApp) << "ExtCaptureManager: Output source manager protocol available";
                emit protocolAvailable();
            });
    
    connect(d->outputSourceManager, &ExtOutputSourceManager::protocolUnavailable,
            this, [this]() {
                qCWarning(dsrApp) << "ExtCaptureManager: Output source manager protocol unavailable";
                emit protocolUnavailable();
            });
    
    connect(this, &QWaylandClientExtension::activeChanged,
            this, &ExtCaptureManager::onActiveChanged);
}

ExtCaptureManager::~ExtCaptureManager()
{
    delete d;
}

const wl_interface *ExtCaptureManager::extensionInterface() const
{
    return &ext_image_copy_capture_manager_v1_interface;
}

bool ExtCaptureManager::isProtocolAvailable() const
{
    bool managerAvailable = isActive() && d->protocolActive;
    bool outputSourceAvailable = d->outputSourceManager && d->outputSourceManager->isProtocolAvailable();
    
    qCWarning(dsrApp) << "ExtCaptureManager::isProtocolAvailable - manager:" << managerAvailable 
                      << "outputSource:" << outputSourceAvailable;
    
    return managerAvailable && outputSourceAvailable;
}

ExtCaptureSession* ExtCaptureManager::createScreenCaptureSession(QScreen *screen, bool paintCursors)
{
    if (!isProtocolAvailable()) {
        qWarning() << "ext-image-copy-capture protocol is not available";
        return nullptr;
    }

    if (!screen) {
        qWarning() << "Invalid screen provided";
        return nullptr;
    }

    // 获取 Wayland output
    wl_output *waylandOutput = getWaylandOutput(screen);
    if (!waylandOutput) {
        qWarning() << "Failed to get Wayland output for screen";
        return nullptr;
    }

    // 检查 output source manager 是否可用
    if (!d->outputSourceManager || !d->outputSourceManager->isProtocolAvailable()) {
        qWarning() << "Output source manager is not available";
        return nullptr;
    }

    // 为输出创建图像捕获源
    void *imageSource = d->outputSourceManager->createSourceForOutput(waylandOutput);
    if (!imageSource) {
        qWarning() << "Failed to create image capture source for output";
        return nullptr;
    }

    // 创建捕获会话
    ExtCaptureSession *session = new ExtCaptureSession(this);
    if (!session->initialize(d, imageSource, paintCursors)) {
        delete session;
        return nullptr;
    }

    return session;
}

int ExtCaptureManager::protocolVersion() const
{
    return d->version;
}

void ExtCaptureManager::bind(wl_registry *registry, int id, int version)
{
    qCWarning(dsrApp) << "ExtCaptureManager::bind called - registry:" << registry 
                      << "id:" << id << "version:" << version;
    d->version = version;
    
    if (registry) {
        // 初始化协议绑定
        d->init(registry, id, qMin(version, 1));
        d->protocolActive = true;
        
        qCWarning(dsrApp) << "ext-image-copy-capture-manager-v1 bound, id:" << id << "version:" << version;
        emit protocolAvailable();
    } else {
        qWarning() << "Failed to get Wayland registry for ext-image-copy-capture-manager-v1";
        emit protocolUnavailable();
    }
}


void ExtCaptureManager::onActiveChanged()
{
    if (isActive()) {
        qCWarning(dsrApp) << "ExtCaptureManager activated";
        setupWaylandIntegration();
    } else {
        qCWarning(dsrApp) << "ExtCaptureManager deactivated";
        if (d->protocolActive) {
            d->protocolActive = false;
            emit protocolUnavailable();
        }
    }
}

void ExtCaptureManager::setupWaylandIntegration()
{
    // 协议绑定由 QWaylandClientExtension 自动处理
}

wl_output* ExtCaptureManager::getWaylandOutput(QScreen *screen)
{
    auto *nativeInterface = QGuiApplication::platformNativeInterface();
    auto *output = static_cast<wl_output*>(
        nativeInterface->nativeResourceForScreen("output", screen));
    
    return output;
}

