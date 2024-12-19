// This file was generated by qtwaylandscanner
// source file is /usr/share/treeland-protocols//treeland-capture-unstable-v1.xml
// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QT_WAYLAND_TREELAND_CAPTURE_UNSTABLE_V1
#define QT_WAYLAND_TREELAND_CAPTURE_UNSTABLE_V1

#include "wayland-treeland-capture-unstable-v1-client-protocol.h"
#include <QByteArray>
#include <QString>

struct wl_registry;

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")
QT_WARNING_DISABLE_CLANG("-Wmissing-field-initializers")

namespace QtWayland {
    class  treeland_capture_session_v1
    {
    public:
        treeland_capture_session_v1(struct ::wl_registry *registry, uint32_t id, int version);
        treeland_capture_session_v1(struct ::treeland_capture_session_v1 *object);
        treeland_capture_session_v1();

        virtual ~treeland_capture_session_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::treeland_capture_session_v1 *object);

        struct ::treeland_capture_session_v1 *object() { return m_treeland_capture_session_v1; }
        const struct ::treeland_capture_session_v1 *object() const { return m_treeland_capture_session_v1; }
        static treeland_capture_session_v1 *fromObject(struct ::treeland_capture_session_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        enum cancel_reason {
            cancel_reason_temporary = 0, // temporary error, source will produce more frames
            cancel_reason_permanent = 1, // fatal error, source will not produce frames
            cancel_reason_resizing = 2, // temporary error, source will produce more frames
        };

        enum flags {
            flags_transient = 0x1, // clients should copy frame before processing
        };

        void destroy();
        void start();

    protected:
        virtual void treeland_capture_session_v1_frame(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height, uint32_t buffer_flags, uint32_t flags, uint32_t format, uint32_t mod_high, uint32_t mod_low, uint32_t num_objects);
        virtual void treeland_capture_session_v1_object(uint32_t index, int32_t fd, uint32_t size, uint32_t offset, uint32_t stride, uint32_t plane_index);
        virtual void treeland_capture_session_v1_ready(uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec);
        virtual void treeland_capture_session_v1_cancel(uint32_t reason);

    private:
        void init_listener();
        static const struct treeland_capture_session_v1_listener m_treeland_capture_session_v1_listener;
        static void handle_frame(
            void *data,
            struct ::treeland_capture_session_v1 *object,
            int32_t offset_x,
            int32_t offset_y,
            uint32_t width,
            uint32_t height,
            uint32_t buffer_flags,
            uint32_t flags,
            uint32_t format,
            uint32_t mod_high,
            uint32_t mod_low,
            uint32_t num_objects);
        static void handle_object(
            void *data,
            struct ::treeland_capture_session_v1 *object,
            uint32_t index,
            int32_t fd,
            uint32_t size,
            uint32_t offset,
            uint32_t stride,
            uint32_t plane_index);
        static void handle_ready(
            void *data,
            struct ::treeland_capture_session_v1 *object,
            uint32_t tv_sec_hi,
            uint32_t tv_sec_lo,
            uint32_t tv_nsec);
        static void handle_cancel(
            void *data,
            struct ::treeland_capture_session_v1 *object,
            uint32_t reason);
        struct ::treeland_capture_session_v1 *m_treeland_capture_session_v1;
    };

    class  treeland_capture_frame_v1
    {
    public:
        treeland_capture_frame_v1(struct ::wl_registry *registry, uint32_t id, int version);
        treeland_capture_frame_v1(struct ::treeland_capture_frame_v1 *object);
        treeland_capture_frame_v1();

        virtual ~treeland_capture_frame_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::treeland_capture_frame_v1 *object);

        struct ::treeland_capture_frame_v1 *object() { return m_treeland_capture_frame_v1; }
        const struct ::treeland_capture_frame_v1 *object() const { return m_treeland_capture_frame_v1; }
        static treeland_capture_frame_v1 *fromObject(struct ::treeland_capture_frame_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        enum flags {
            flags_y_inverted = 0x1, // contents are y-inverted
        };

        void destroy();
        void copy(struct ::wl_buffer *buffer);

    protected:
        virtual void treeland_capture_frame_v1_buffer(uint32_t format, uint32_t width, uint32_t height, uint32_t stride);
        virtual void treeland_capture_frame_v1_buffer_done();
        virtual void treeland_capture_frame_v1_flags(uint32_t flags);
        virtual void treeland_capture_frame_v1_ready();
        virtual void treeland_capture_frame_v1_failed();

    private:
        void init_listener();
        static const struct treeland_capture_frame_v1_listener m_treeland_capture_frame_v1_listener;
        static void handle_buffer(
            void *data,
            struct ::treeland_capture_frame_v1 *object,
            uint32_t format,
            uint32_t width,
            uint32_t height,
            uint32_t stride);
        static void handle_buffer_done(
            void *data,
            struct ::treeland_capture_frame_v1 *object);
        static void handle_flags(
            void *data,
            struct ::treeland_capture_frame_v1 *object,
            uint32_t flags);
        static void handle_ready(
            void *data,
            struct ::treeland_capture_frame_v1 *object);
        static void handle_failed(
            void *data,
            struct ::treeland_capture_frame_v1 *object);
        struct ::treeland_capture_frame_v1 *m_treeland_capture_frame_v1;
    };

    class  treeland_capture_context_v1
    {
    public:
        treeland_capture_context_v1(struct ::wl_registry *registry, uint32_t id, int version);
        treeland_capture_context_v1(struct ::treeland_capture_context_v1 *object);
        treeland_capture_context_v1();

        virtual ~treeland_capture_context_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::treeland_capture_context_v1 *object);

        struct ::treeland_capture_context_v1 *object() { return m_treeland_capture_context_v1; }
        const struct ::treeland_capture_context_v1 *object() const { return m_treeland_capture_context_v1; }
        static treeland_capture_context_v1 *fromObject(struct ::treeland_capture_context_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        enum source_type {
            source_type_output = 0x1, // output source type
            source_type_window = 0x2, // window source type
            source_type_region = 0x4, // region source type
        };

        enum source_failure {
            source_failure_selector_busy = 1, // selector is occupied by other context
            source_failure_other = 2, // other failure
        };

        void destroy();
        void select_source(uint32_t source_hint, uint32_t freeze, uint32_t with_cursor, struct ::wl_surface *mask);
        struct ::treeland_capture_frame_v1 *capture();
        struct ::treeland_capture_session_v1 *create_session();

    protected:
        virtual void treeland_capture_context_v1_source_ready(int32_t region_x, int32_t region_y, uint32_t region_width, uint32_t region_height, uint32_t source_type);
        virtual void treeland_capture_context_v1_source_failed(uint32_t reason);

    private:
        void init_listener();
        static const struct treeland_capture_context_v1_listener m_treeland_capture_context_v1_listener;
        static void handle_source_ready(
            void *data,
            struct ::treeland_capture_context_v1 *object,
            int32_t region_x,
            int32_t region_y,
            uint32_t region_width,
            uint32_t region_height,
            uint32_t source_type);
        static void handle_source_failed(
            void *data,
            struct ::treeland_capture_context_v1 *object,
            uint32_t reason);
        struct ::treeland_capture_context_v1 *m_treeland_capture_context_v1;
    };

    class  treeland_capture_manager_v1
    {
    public:
        treeland_capture_manager_v1(struct ::wl_registry *registry, uint32_t id, int version);
        treeland_capture_manager_v1(struct ::treeland_capture_manager_v1 *object);
        treeland_capture_manager_v1();

        virtual ~treeland_capture_manager_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::treeland_capture_manager_v1 *object);

        struct ::treeland_capture_manager_v1 *object() { return m_treeland_capture_manager_v1; }
        const struct ::treeland_capture_manager_v1 *object() const { return m_treeland_capture_manager_v1; }
        static treeland_capture_manager_v1 *fromObject(struct ::treeland_capture_manager_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        void destroy();
        struct ::treeland_capture_context_v1 *get_context();

    private:
        struct ::treeland_capture_manager_v1 *m_treeland_capture_manager_v1;
    };
}

QT_WARNING_POP
QT_END_NAMESPACE

#endif
