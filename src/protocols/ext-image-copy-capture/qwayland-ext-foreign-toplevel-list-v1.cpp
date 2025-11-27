// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qwayland-ext-foreign-toplevel-list-v1.h"

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

    ext_foreign_toplevel_list_v1::ext_foreign_toplevel_list_v1(struct ::wl_registry *registry, uint32_t id, int version)
    {
        init(registry, id, version);
    }

    ext_foreign_toplevel_list_v1::ext_foreign_toplevel_list_v1(struct ::ext_foreign_toplevel_list_v1 *obj)
        : m_ext_foreign_toplevel_list_v1(obj)
    {
        init_listener();
    }

    ext_foreign_toplevel_list_v1::ext_foreign_toplevel_list_v1()
        : m_ext_foreign_toplevel_list_v1(nullptr)
    {
    }

    ext_foreign_toplevel_list_v1::~ext_foreign_toplevel_list_v1()
    {
    }

    void ext_foreign_toplevel_list_v1::init(struct ::wl_registry *registry, uint32_t id, int version)
    {
        m_ext_foreign_toplevel_list_v1 = static_cast<struct ::ext_foreign_toplevel_list_v1 *>(wlRegistryBind(registry, id, &ext_foreign_toplevel_list_v1_interface, version));
        init_listener();
    }

    void ext_foreign_toplevel_list_v1::init(struct ::ext_foreign_toplevel_list_v1 *obj)
    {
        m_ext_foreign_toplevel_list_v1 = obj;
        init_listener();
    }

    ext_foreign_toplevel_list_v1 *ext_foreign_toplevel_list_v1::fromObject(struct ::ext_foreign_toplevel_list_v1 *object)
    {
        if (wl_proxy_get_listener((struct ::wl_proxy *)object) != (void *)&m_ext_foreign_toplevel_list_v1_listener)
            return nullptr;
        return static_cast<ext_foreign_toplevel_list_v1 *>(ext_foreign_toplevel_list_v1_get_user_data(object));
    }

    bool ext_foreign_toplevel_list_v1::isInitialized() const
    {
        return m_ext_foreign_toplevel_list_v1 != nullptr;
    }

    uint32_t ext_foreign_toplevel_list_v1::version() const
    {
        return wl_proxy_get_version(reinterpret_cast<wl_proxy*>(m_ext_foreign_toplevel_list_v1));
    }

    const struct wl_interface *ext_foreign_toplevel_list_v1::interface()
    {
        return &::ext_foreign_toplevel_list_v1_interface;
    }

    void ext_foreign_toplevel_list_v1::stop()
    {
        ::ext_foreign_toplevel_list_v1_stop(
            m_ext_foreign_toplevel_list_v1);
    }

    void ext_foreign_toplevel_list_v1::destroy()
    {
        ::ext_foreign_toplevel_list_v1_destroy(
            m_ext_foreign_toplevel_list_v1);
        m_ext_foreign_toplevel_list_v1 = nullptr;
    }

    void ext_foreign_toplevel_list_v1::ext_foreign_toplevel_list_v1_toplevel(struct ::ext_foreign_toplevel_handle_v1 *)
    {
    }

    void ext_foreign_toplevel_list_v1::handle_toplevel(
        void *data,
        struct ::ext_foreign_toplevel_list_v1 *object,
        struct ::ext_foreign_toplevel_handle_v1 *toplevel)
    {
        Q_UNUSED(object);
        static_cast<ext_foreign_toplevel_list_v1 *>(data)->ext_foreign_toplevel_list_v1_toplevel(
            toplevel);
    }

    void ext_foreign_toplevel_list_v1::ext_foreign_toplevel_list_v1_finished()
    {
    }

    void ext_foreign_toplevel_list_v1::handle_finished(
        void *data,
        struct ::ext_foreign_toplevel_list_v1 *object)
    {
        Q_UNUSED(object);
        static_cast<ext_foreign_toplevel_list_v1 *>(data)->ext_foreign_toplevel_list_v1_finished();
    }

    const struct ext_foreign_toplevel_list_v1_listener ext_foreign_toplevel_list_v1::m_ext_foreign_toplevel_list_v1_listener = {
        ext_foreign_toplevel_list_v1::handle_toplevel,
        ext_foreign_toplevel_list_v1::handle_finished,
    };

    void ext_foreign_toplevel_list_v1::init_listener()
    {
        ext_foreign_toplevel_list_v1_add_listener(m_ext_foreign_toplevel_list_v1, &m_ext_foreign_toplevel_list_v1_listener, this);
    }

    ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1(struct ::wl_registry *registry, uint32_t id, int version)
    {
        init(registry, id, version);
    }

    ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1(struct ::ext_foreign_toplevel_handle_v1 *obj)
        : m_ext_foreign_toplevel_handle_v1(obj)
    {
        init_listener();
    }

    ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1()
        : m_ext_foreign_toplevel_handle_v1(nullptr)
    {
    }

    ext_foreign_toplevel_handle_v1::~ext_foreign_toplevel_handle_v1()
    {
    }

    void ext_foreign_toplevel_handle_v1::init(struct ::wl_registry *registry, uint32_t id, int version)
    {
        m_ext_foreign_toplevel_handle_v1 = static_cast<struct ::ext_foreign_toplevel_handle_v1 *>(wlRegistryBind(registry, id, &ext_foreign_toplevel_handle_v1_interface, version));
        init_listener();
    }

    void ext_foreign_toplevel_handle_v1::init(struct ::ext_foreign_toplevel_handle_v1 *obj)
    {
        m_ext_foreign_toplevel_handle_v1 = obj;
        init_listener();
    }

    ext_foreign_toplevel_handle_v1 *ext_foreign_toplevel_handle_v1::fromObject(struct ::ext_foreign_toplevel_handle_v1 *object)
    {
        if (wl_proxy_get_listener((struct ::wl_proxy *)object) != (void *)&m_ext_foreign_toplevel_handle_v1_listener)
            return nullptr;
        return static_cast<ext_foreign_toplevel_handle_v1 *>(ext_foreign_toplevel_handle_v1_get_user_data(object));
    }

    bool ext_foreign_toplevel_handle_v1::isInitialized() const
    {
        return m_ext_foreign_toplevel_handle_v1 != nullptr;
    }

    uint32_t ext_foreign_toplevel_handle_v1::version() const
    {
        return wl_proxy_get_version(reinterpret_cast<wl_proxy*>(m_ext_foreign_toplevel_handle_v1));
    }

    const struct wl_interface *ext_foreign_toplevel_handle_v1::interface()
    {
        return &::ext_foreign_toplevel_handle_v1_interface;
    }

    void ext_foreign_toplevel_handle_v1::destroy()
    {
        ::ext_foreign_toplevel_handle_v1_destroy(
            m_ext_foreign_toplevel_handle_v1);
        m_ext_foreign_toplevel_handle_v1 = nullptr;
    }

    void ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_closed()
    {
    }

    void ext_foreign_toplevel_handle_v1::handle_closed(
        void *data,
        struct ::ext_foreign_toplevel_handle_v1 *object)
    {
        Q_UNUSED(object);
        static_cast<ext_foreign_toplevel_handle_v1 *>(data)->ext_foreign_toplevel_handle_v1_closed();
    }

    void ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_done()
    {
    }

    void ext_foreign_toplevel_handle_v1::handle_done(
        void *data,
        struct ::ext_foreign_toplevel_handle_v1 *object)
    {
        Q_UNUSED(object);
        static_cast<ext_foreign_toplevel_handle_v1 *>(data)->ext_foreign_toplevel_handle_v1_done();
    }

    void ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_title(const QString &)
    {
    }

    void ext_foreign_toplevel_handle_v1::handle_title(
        void *data,
        struct ::ext_foreign_toplevel_handle_v1 *object,
        const char *title)
    {
        Q_UNUSED(object);
        static_cast<ext_foreign_toplevel_handle_v1 *>(data)->ext_foreign_toplevel_handle_v1_title(
            QString::fromUtf8(title));
    }

    void ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_app_id(const QString &)
    {
    }

    void ext_foreign_toplevel_handle_v1::handle_app_id(
        void *data,
        struct ::ext_foreign_toplevel_handle_v1 *object,
        const char *app_id)
    {
        Q_UNUSED(object);
        static_cast<ext_foreign_toplevel_handle_v1 *>(data)->ext_foreign_toplevel_handle_v1_app_id(
            QString::fromUtf8(app_id));
    }

    void ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_identifier(const QString &)
    {
    }

    void ext_foreign_toplevel_handle_v1::handle_identifier(
        void *data,
        struct ::ext_foreign_toplevel_handle_v1 *object,
        const char *identifier)
    {
        Q_UNUSED(object);
        static_cast<ext_foreign_toplevel_handle_v1 *>(data)->ext_foreign_toplevel_handle_v1_identifier(
            QString::fromUtf8(identifier));
    }

    const struct ext_foreign_toplevel_handle_v1_listener ext_foreign_toplevel_handle_v1::m_ext_foreign_toplevel_handle_v1_listener = {
        ext_foreign_toplevel_handle_v1::handle_closed,
        ext_foreign_toplevel_handle_v1::handle_done,
        ext_foreign_toplevel_handle_v1::handle_title,
        ext_foreign_toplevel_handle_v1::handle_app_id,
        ext_foreign_toplevel_handle_v1::handle_identifier,
    };

    void ext_foreign_toplevel_handle_v1::init_listener()
    {
        ext_foreign_toplevel_handle_v1_add_listener(m_ext_foreign_toplevel_handle_v1, &m_ext_foreign_toplevel_handle_v1_listener, this);
    }
}

QT_WARNING_POP
QT_END_NAMESPACE
