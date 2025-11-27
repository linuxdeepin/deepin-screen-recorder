// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extoutputsourcemanager.h"
#include "../../protocols/ext-image-copy-capture/qwayland-ext-image-capture-source-v1.h"
#include "../../protocols/ext-image-copy-capture/wayland-ext-image-capture-source-v1-client-protocol.h"
#include "../../utils/log.h"

#include <QDebug>
#include <QTimer>
#include <wayland-client.h>

class ExtOutputSourceManager::Private : public QtWayland::ext_output_image_capture_source_manager_v1
{
public:
    Private(ExtOutputSourceManager *qq) : q(qq), protocolActive(false), version(0) {}
    
    ExtOutputSourceManager *q;
    bool protocolActive;
    int version;
    
    void* currentSource = nullptr;  // 保存最近创建的source
};

ExtOutputSourceManager::ExtOutputSourceManager(QObject *parent)
    : QWaylandClientExtension(1), d(new Private(this))
{
    setParent(parent);
    
    // 监听协议状态变化
    connect(this, &QWaylandClientExtension::activeChanged, 
            this, &ExtOutputSourceManager::onActiveChanged);
    
    qCWarning(dsrApp) << "ExtOutputSourceManager created";
}

ExtOutputSourceManager::~ExtOutputSourceManager()
{
    qCWarning(dsrApp) << "ExtOutputSourceManager destroyed";
    delete d;
}

bool ExtOutputSourceManager::isProtocolAvailable() const
{
    return isActive() && d->protocolActive;
}

void* ExtOutputSourceManager::createSourceForOutput(wl_output *output)
{
    if (!isProtocolAvailable()) {
        qWarning() << "Output source manager protocol is not available";
        return nullptr;
    }
    
    if (!output) {
        qWarning() << "Invalid output provided";
        return nullptr;
    }
    
    qCWarning(dsrApp) << "Creating image capture source for output:" << output;
    
    // 创建图像捕获源
    auto *source = d->create_source(output);
    d->currentSource = source;
    
    qCWarning(dsrApp) << "Image capture source created:" << source;
    return source;
}

int ExtOutputSourceManager::protocolVersion() const
{
    return d->version;
}

const wl_interface *ExtOutputSourceManager::extensionInterface() const
{
    return &ext_output_image_capture_source_manager_v1_interface;
}

void ExtOutputSourceManager::bind(wl_registry *registry, int id, int version)
{
    qCWarning(dsrApp) << "ExtOutputSourceManager::bind called - registry:" << registry 
                      << "id:" << id << "version:" << version;
    d->version = version;
    
    if (registry) {
        // 初始化协议绑定
        d->init(registry, id, qMin(version, 1));
        d->protocolActive = true;
        qCWarning(dsrApp) << "ExtOutputSourceManager bound to registry, version:" << version;
    }
}

void ExtOutputSourceManager::onActiveChanged()
{
    qCWarning(dsrApp) << "ExtOutputSourceManager active changed:" << isActive();
    
    if (isActive() && d->protocolActive) {
        emit protocolAvailable();
    } else {
        emit protocolUnavailable();
    }
}
