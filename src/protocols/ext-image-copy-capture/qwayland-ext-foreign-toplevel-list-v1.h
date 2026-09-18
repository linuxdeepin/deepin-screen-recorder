// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QT_WAYLAND_EXT_FOREIGN_TOPLEVEL_LIST_V1
#define QT_WAYLAND_EXT_FOREIGN_TOPLEVEL_LIST_V1

#include "wayland-ext-foreign-toplevel-list-v1-client-protocol.h"
#include <QByteArray>
#include <QString>

struct wl_registry;

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")
QT_WARNING_DISABLE_CLANG("-Wmissing-field-initializers")

namespace QtWayland {
    class  ext_foreign_toplevel_list_v1
    {
    public:
        ext_foreign_toplevel_list_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_foreign_toplevel_list_v1(struct ::ext_foreign_toplevel_list_v1 *object);
        ext_foreign_toplevel_list_v1();

        virtual ~ext_foreign_toplevel_list_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_foreign_toplevel_list_v1 *object);

        struct ::ext_foreign_toplevel_list_v1 *object() { return m_ext_foreign_toplevel_list_v1; }
        const struct ::ext_foreign_toplevel_list_v1 *object() const { return m_ext_foreign_toplevel_list_v1; }
        static ext_foreign_toplevel_list_v1 *fromObject(struct ::ext_foreign_toplevel_list_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        void stop();
        void destroy();

    protected:
        virtual void ext_foreign_toplevel_list_v1_toplevel(struct ::ext_foreign_toplevel_handle_v1 *toplevel);
        virtual void ext_foreign_toplevel_list_v1_finished();

    private:
        void init_listener();
        static const struct ext_foreign_toplevel_list_v1_listener m_ext_foreign_toplevel_list_v1_listener;
        static void handle_toplevel(
            void *data,
            struct ::ext_foreign_toplevel_list_v1 *object,
            struct ::ext_foreign_toplevel_handle_v1 *toplevel);
        static void handle_finished(
            void *data,
            struct ::ext_foreign_toplevel_list_v1 *object);
        struct ::ext_foreign_toplevel_list_v1 *m_ext_foreign_toplevel_list_v1;
    };

    class  ext_foreign_toplevel_handle_v1
    {
    public:
        ext_foreign_toplevel_handle_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_foreign_toplevel_handle_v1(struct ::ext_foreign_toplevel_handle_v1 *object);
        ext_foreign_toplevel_handle_v1();

        virtual ~ext_foreign_toplevel_handle_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_foreign_toplevel_handle_v1 *object);

        struct ::ext_foreign_toplevel_handle_v1 *object() { return m_ext_foreign_toplevel_handle_v1; }
        const struct ::ext_foreign_toplevel_handle_v1 *object() const { return m_ext_foreign_toplevel_handle_v1; }
        static ext_foreign_toplevel_handle_v1 *fromObject(struct ::ext_foreign_toplevel_handle_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        void destroy();

    protected:
        virtual void ext_foreign_toplevel_handle_v1_closed();
        virtual void ext_foreign_toplevel_handle_v1_done();
        virtual void ext_foreign_toplevel_handle_v1_title(const QString &title);
        virtual void ext_foreign_toplevel_handle_v1_app_id(const QString &app_id);
        virtual void ext_foreign_toplevel_handle_v1_identifier(const QString &identifier);

    private:
        void init_listener();
        static const struct ext_foreign_toplevel_handle_v1_listener m_ext_foreign_toplevel_handle_v1_listener;
        static void handle_closed(
            void *data,
            struct ::ext_foreign_toplevel_handle_v1 *object);
        static void handle_done(
            void *data,
            struct ::ext_foreign_toplevel_handle_v1 *object);
        static void handle_title(
            void *data,
            struct ::ext_foreign_toplevel_handle_v1 *object,
            const char *title);
        static void handle_app_id(
            void *data,
            struct ::ext_foreign_toplevel_handle_v1 *object,
            const char *app_id);
        static void handle_identifier(
            void *data,
            struct ::ext_foreign_toplevel_handle_v1 *object,
            const char *identifier);
        struct ::ext_foreign_toplevel_handle_v1 *m_ext_foreign_toplevel_handle_v1;
    };
}

QT_WARNING_POP
QT_END_NAMESPACE

#endif
