// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qwayland-ext-image-capture-source-v1.h"

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")
QT_WARNING_DISABLE_CLANG("-Wmissing-field-initializers")

namespace QtWayland {

static inline void *wlRegistryBind(struct ::wl_registry *registry, uint32_t name, const struct ::wl_interface *interface, uint32_t version)
{
    const uint32_t bindOpCode = 0;
    return (void *) wl_proxy_marshal_constructor_versioned((struct wl_proxy *) registry,
    bindOpCode, interface, version, name, interface->name, version, nullptr);
}

    ext_image_capture_source_v1::ext_image_capture_source_v1(struct ::wl_registry *registry, uint32_t id, int version)
    {
        init(registry, id, version);
    }

    ext_image_capture_source_v1::ext_image_capture_source_v1(struct ::ext_image_capture_source_v1 *obj)
        : m_ext_image_capture_source_v1(obj)
    {
    }

    ext_image_capture_source_v1::ext_image_capture_source_v1()
        : m_ext_image_capture_source_v1(nullptr)
    {
    }

    ext_image_capture_source_v1::~ext_image_capture_source_v1()
    {
    }

    void ext_image_capture_source_v1::init(struct ::wl_registry *registry, uint32_t id, int version)
    {
        m_ext_image_capture_source_v1 = static_cast<struct ::ext_image_capture_source_v1 *>(wlRegistryBind(registry, id, &ext_image_capture_source_v1_interface, version));
    }

    void ext_image_capture_source_v1::init(struct ::ext_image_capture_source_v1 *obj)
    {
        m_ext_image_capture_source_v1 = obj;
    }

    ext_image_capture_source_v1 *ext_image_capture_source_v1::fromObject(struct ::ext_image_capture_source_v1 *object)
    {
        return static_cast<ext_image_capture_source_v1 *>(ext_image_capture_source_v1_get_user_data(object));
    }

    bool ext_image_capture_source_v1::isInitialized() const
    {
        return m_ext_image_capture_source_v1 != nullptr;
    }

    uint32_t ext_image_capture_source_v1::version() const
    {
        return wl_proxy_get_version(reinterpret_cast<wl_proxy*>(m_ext_image_capture_source_v1));
    }

    const struct wl_interface *ext_image_capture_source_v1::interface()
    {
        return &::ext_image_capture_source_v1_interface;
    }

    void ext_image_capture_source_v1::destroy()
    {
        ::ext_image_capture_source_v1_destroy(
            m_ext_image_capture_source_v1);
        m_ext_image_capture_source_v1 = nullptr;
    }

    ext_output_image_capture_source_manager_v1::ext_output_image_capture_source_manager_v1(struct ::wl_registry *registry, uint32_t id, int version)
    {
        init(registry, id, version);
    }

    ext_output_image_capture_source_manager_v1::ext_output_image_capture_source_manager_v1(struct ::ext_output_image_capture_source_manager_v1 *obj)
        : m_ext_output_image_capture_source_manager_v1(obj)
    {
    }

    ext_output_image_capture_source_manager_v1::ext_output_image_capture_source_manager_v1()
        : m_ext_output_image_capture_source_manager_v1(nullptr)
    {
    }

    ext_output_image_capture_source_manager_v1::~ext_output_image_capture_source_manager_v1()
    {
    }

    void ext_output_image_capture_source_manager_v1::init(struct ::wl_registry *registry, uint32_t id, int version)
    {
        m_ext_output_image_capture_source_manager_v1 = static_cast<struct ::ext_output_image_capture_source_manager_v1 *>(wlRegistryBind(registry, id, &ext_output_image_capture_source_manager_v1_interface, version));
    }

    void ext_output_image_capture_source_manager_v1::init(struct ::ext_output_image_capture_source_manager_v1 *obj)
    {
        m_ext_output_image_capture_source_manager_v1 = obj;
    }

    ext_output_image_capture_source_manager_v1 *ext_output_image_capture_source_manager_v1::fromObject(struct ::ext_output_image_capture_source_manager_v1 *object)
    {
        return static_cast<ext_output_image_capture_source_manager_v1 *>(ext_output_image_capture_source_manager_v1_get_user_data(object));
    }

    bool ext_output_image_capture_source_manager_v1::isInitialized() const
    {
        return m_ext_output_image_capture_source_manager_v1 != nullptr;
    }

    uint32_t ext_output_image_capture_source_manager_v1::version() const
    {
        return wl_proxy_get_version(reinterpret_cast<wl_proxy*>(m_ext_output_image_capture_source_manager_v1));
    }

    const struct wl_interface *ext_output_image_capture_source_manager_v1::interface()
    {
        return &::ext_output_image_capture_source_manager_v1_interface;
    }

    struct ::ext_image_capture_source_v1 *ext_output_image_capture_source_manager_v1::create_source(struct ::wl_output *output)
    {
        return ::ext_output_image_capture_source_manager_v1_create_source(
            m_ext_output_image_capture_source_manager_v1,
            output);
    }

    void ext_output_image_capture_source_manager_v1::destroy()
    {
        ::ext_output_image_capture_source_manager_v1_destroy(
            m_ext_output_image_capture_source_manager_v1);
        m_ext_output_image_capture_source_manager_v1 = nullptr;
    }

    ext_foreign_toplevel_image_capture_source_manager_v1::ext_foreign_toplevel_image_capture_source_manager_v1(struct ::wl_registry *registry, uint32_t id, int version)
    {
        init(registry, id, version);
    }

    ext_foreign_toplevel_image_capture_source_manager_v1::ext_foreign_toplevel_image_capture_source_manager_v1(struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *obj)
        : m_ext_foreign_toplevel_image_capture_source_manager_v1(obj)
    {
    }

    ext_foreign_toplevel_image_capture_source_manager_v1::ext_foreign_toplevel_image_capture_source_manager_v1()
        : m_ext_foreign_toplevel_image_capture_source_manager_v1(nullptr)
    {
    }

    ext_foreign_toplevel_image_capture_source_manager_v1::~ext_foreign_toplevel_image_capture_source_manager_v1()
    {
    }

    void ext_foreign_toplevel_image_capture_source_manager_v1::init(struct ::wl_registry *registry, uint32_t id, int version)
    {
        m_ext_foreign_toplevel_image_capture_source_manager_v1 = static_cast<struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *>(wlRegistryBind(registry, id, &ext_foreign_toplevel_image_capture_source_manager_v1_interface, version));
    }

    void ext_foreign_toplevel_image_capture_source_manager_v1::init(struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *obj)
    {
        m_ext_foreign_toplevel_image_capture_source_manager_v1 = obj;
    }

    ext_foreign_toplevel_image_capture_source_manager_v1 *ext_foreign_toplevel_image_capture_source_manager_v1::fromObject(struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *object)
    {
        return static_cast<ext_foreign_toplevel_image_capture_source_manager_v1 *>(ext_foreign_toplevel_image_capture_source_manager_v1_get_user_data(object));
    }

    bool ext_foreign_toplevel_image_capture_source_manager_v1::isInitialized() const
    {
        return m_ext_foreign_toplevel_image_capture_source_manager_v1 != nullptr;
    }

    uint32_t ext_foreign_toplevel_image_capture_source_manager_v1::version() const
    {
        return wl_proxy_get_version(reinterpret_cast<wl_proxy*>(m_ext_foreign_toplevel_image_capture_source_manager_v1));
    }

    const struct wl_interface *ext_foreign_toplevel_image_capture_source_manager_v1::interface()
    {
        return &::ext_foreign_toplevel_image_capture_source_manager_v1_interface;
    }

    struct ::ext_image_capture_source_v1 *ext_foreign_toplevel_image_capture_source_manager_v1::create_source(struct ::ext_foreign_toplevel_handle_v1 *toplevel_handle)
    {
        return ::ext_foreign_toplevel_image_capture_source_manager_v1_create_source(
            m_ext_foreign_toplevel_image_capture_source_manager_v1,
            toplevel_handle);
    }

    void ext_foreign_toplevel_image_capture_source_manager_v1::destroy()
    {
        ::ext_foreign_toplevel_image_capture_source_manager_v1_destroy(
            m_ext_foreign_toplevel_image_capture_source_manager_v1);
        m_ext_foreign_toplevel_image_capture_source_manager_v1 = nullptr;
    }
}

QT_WARNING_POP
QT_END_NAMESPACE
