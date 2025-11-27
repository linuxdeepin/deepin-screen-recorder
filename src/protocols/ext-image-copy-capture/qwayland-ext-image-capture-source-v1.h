// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QT_WAYLAND_EXT_IMAGE_CAPTURE_SOURCE_V1
#define QT_WAYLAND_EXT_IMAGE_CAPTURE_SOURCE_V1

#include "wayland-ext-image-capture-source-v1-client-protocol.h"
#include <QByteArray>
#include <QString>

struct wl_registry;

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")
QT_WARNING_DISABLE_CLANG("-Wmissing-field-initializers")

namespace QtWayland {
    class  ext_image_capture_source_v1
    {
    public:
        ext_image_capture_source_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_image_capture_source_v1(struct ::ext_image_capture_source_v1 *object);
        ext_image_capture_source_v1();

        virtual ~ext_image_capture_source_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_image_capture_source_v1 *object);

        struct ::ext_image_capture_source_v1 *object() { return m_ext_image_capture_source_v1; }
        const struct ::ext_image_capture_source_v1 *object() const { return m_ext_image_capture_source_v1; }
        static ext_image_capture_source_v1 *fromObject(struct ::ext_image_capture_source_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        void destroy();

    private:
        struct ::ext_image_capture_source_v1 *m_ext_image_capture_source_v1;
    };

    class  ext_output_image_capture_source_manager_v1
    {
    public:
        ext_output_image_capture_source_manager_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_output_image_capture_source_manager_v1(struct ::ext_output_image_capture_source_manager_v1 *object);
        ext_output_image_capture_source_manager_v1();

        virtual ~ext_output_image_capture_source_manager_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_output_image_capture_source_manager_v1 *object);

        struct ::ext_output_image_capture_source_manager_v1 *object() { return m_ext_output_image_capture_source_manager_v1; }
        const struct ::ext_output_image_capture_source_manager_v1 *object() const { return m_ext_output_image_capture_source_manager_v1; }
        static ext_output_image_capture_source_manager_v1 *fromObject(struct ::ext_output_image_capture_source_manager_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        struct ::ext_image_capture_source_v1 *create_source(struct ::wl_output *output);
        void destroy();

    private:
        struct ::ext_output_image_capture_source_manager_v1 *m_ext_output_image_capture_source_manager_v1;
    };

    class  ext_foreign_toplevel_image_capture_source_manager_v1
    {
    public:
        ext_foreign_toplevel_image_capture_source_manager_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_foreign_toplevel_image_capture_source_manager_v1(struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *object);
        ext_foreign_toplevel_image_capture_source_manager_v1();

        virtual ~ext_foreign_toplevel_image_capture_source_manager_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *object);

        struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *object() { return m_ext_foreign_toplevel_image_capture_source_manager_v1; }
        const struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *object() const { return m_ext_foreign_toplevel_image_capture_source_manager_v1; }
        static ext_foreign_toplevel_image_capture_source_manager_v1 *fromObject(struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        struct ::ext_image_capture_source_v1 *create_source(struct ::ext_foreign_toplevel_handle_v1 *toplevel_handle);
        void destroy();

    private:
        struct ::ext_foreign_toplevel_image_capture_source_manager_v1 *m_ext_foreign_toplevel_image_capture_source_manager_v1;
    };
}

QT_WARNING_POP
QT_END_NAMESPACE

#endif
