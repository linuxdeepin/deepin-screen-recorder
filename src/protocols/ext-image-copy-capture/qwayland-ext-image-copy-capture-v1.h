// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QT_WAYLAND_EXT_IMAGE_COPY_CAPTURE_V1
#define QT_WAYLAND_EXT_IMAGE_COPY_CAPTURE_V1

#include "wayland-ext-image-copy-capture-v1-client-protocol.h"
#include <QByteArray>
#include <QString>

struct wl_registry;

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")
QT_WARNING_DISABLE_CLANG("-Wmissing-field-initializers")

namespace QtWayland {
    class  ext_image_copy_capture_manager_v1
    {
    public:
        ext_image_copy_capture_manager_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_image_copy_capture_manager_v1(struct ::ext_image_copy_capture_manager_v1 *object);
        ext_image_copy_capture_manager_v1();

        virtual ~ext_image_copy_capture_manager_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_image_copy_capture_manager_v1 *object);

        struct ::ext_image_copy_capture_manager_v1 *object() { return m_ext_image_copy_capture_manager_v1; }
        const struct ::ext_image_copy_capture_manager_v1 *object() const { return m_ext_image_copy_capture_manager_v1; }
        static ext_image_copy_capture_manager_v1 *fromObject(struct ::ext_image_copy_capture_manager_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        enum error {
            error_invalid_option = 1, // invalid option flag
        };

        enum options {
            options_paint_cursors = 1, // paint cursors onto captured frames
        };

        struct ::ext_image_copy_capture_session_v1 *create_session(struct ::ext_image_capture_source_v1 *source, uint32_t options);
        struct ::ext_image_copy_capture_cursor_session_v1 *create_pointer_cursor_session(struct ::ext_image_capture_source_v1 *source, struct ::wl_pointer *pointer);
        void destroy();

    private:
        struct ::ext_image_copy_capture_manager_v1 *m_ext_image_copy_capture_manager_v1;
    };

    class  ext_image_copy_capture_session_v1
    {
    public:
        ext_image_copy_capture_session_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_image_copy_capture_session_v1(struct ::ext_image_copy_capture_session_v1 *object);
        ext_image_copy_capture_session_v1();

        virtual ~ext_image_copy_capture_session_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_image_copy_capture_session_v1 *object);

        struct ::ext_image_copy_capture_session_v1 *object() { return m_ext_image_copy_capture_session_v1; }
        const struct ::ext_image_copy_capture_session_v1 *object() const { return m_ext_image_copy_capture_session_v1; }
        static ext_image_copy_capture_session_v1 *fromObject(struct ::ext_image_copy_capture_session_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        enum error {
            error_duplicate_frame = 1, // create_frame sent before destroying previous frame
        };

        struct ::ext_image_copy_capture_frame_v1 *create_frame();
        void destroy();

    protected:
        virtual void ext_image_copy_capture_session_v1_buffer_size(uint32_t width, uint32_t height);
        virtual void ext_image_copy_capture_session_v1_shm_format(uint32_t format);
        virtual void ext_image_copy_capture_session_v1_dmabuf_device(wl_array *device);
        virtual void ext_image_copy_capture_session_v1_dmabuf_format(uint32_t format, wl_array *modifiers);
        virtual void ext_image_copy_capture_session_v1_done();
        virtual void ext_image_copy_capture_session_v1_stopped();

    private:
        void init_listener();
        static const struct ext_image_copy_capture_session_v1_listener m_ext_image_copy_capture_session_v1_listener;
        static void handle_buffer_size(
            void *data,
            struct ::ext_image_copy_capture_session_v1 *object,
            uint32_t width,
            uint32_t height);
        static void handle_shm_format(
            void *data,
            struct ::ext_image_copy_capture_session_v1 *object,
            uint32_t format);
        static void handle_dmabuf_device(
            void *data,
            struct ::ext_image_copy_capture_session_v1 *object,
            wl_array *device);
        static void handle_dmabuf_format(
            void *data,
            struct ::ext_image_copy_capture_session_v1 *object,
            uint32_t format,
            wl_array *modifiers);
        static void handle_done(
            void *data,
            struct ::ext_image_copy_capture_session_v1 *object);
        static void handle_stopped(
            void *data,
            struct ::ext_image_copy_capture_session_v1 *object);
        struct ::ext_image_copy_capture_session_v1 *m_ext_image_copy_capture_session_v1;
    };

    class  ext_image_copy_capture_frame_v1
    {
    public:
        ext_image_copy_capture_frame_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_image_copy_capture_frame_v1(struct ::ext_image_copy_capture_frame_v1 *object);
        ext_image_copy_capture_frame_v1();

        virtual ~ext_image_copy_capture_frame_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_image_copy_capture_frame_v1 *object);

        struct ::ext_image_copy_capture_frame_v1 *object() { return m_ext_image_copy_capture_frame_v1; }
        const struct ::ext_image_copy_capture_frame_v1 *object() const { return m_ext_image_copy_capture_frame_v1; }
        static ext_image_copy_capture_frame_v1 *fromObject(struct ::ext_image_copy_capture_frame_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        enum error {
            error_no_buffer = 1, // capture sent without attach_buffer
            error_invalid_buffer_damage = 2, // invalid buffer damage
            error_already_captured = 3, // capture request has been sent
        };

        enum failure_reason {
            failure_reason_unknown = 0,
            failure_reason_buffer_constraints = 1,
            failure_reason_stopped = 2,
        };

        void destroy();
        void attach_buffer(struct ::wl_buffer *buffer);
        void damage_buffer(int32_t x, int32_t y, int32_t width, int32_t height);
        void capture();

    protected:
        virtual void ext_image_copy_capture_frame_v1_transform(uint32_t transform);
        virtual void ext_image_copy_capture_frame_v1_damage(int32_t x, int32_t y, int32_t width, int32_t height);
        virtual void ext_image_copy_capture_frame_v1_presentation_time(uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec);
        virtual void ext_image_copy_capture_frame_v1_ready();
        virtual void ext_image_copy_capture_frame_v1_failed(uint32_t reason);

    private:
        void init_listener();
        static const struct ext_image_copy_capture_frame_v1_listener m_ext_image_copy_capture_frame_v1_listener;
        static void handle_transform(
            void *data,
            struct ::ext_image_copy_capture_frame_v1 *object,
            uint32_t transform);
        static void handle_damage(
            void *data,
            struct ::ext_image_copy_capture_frame_v1 *object,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height);
        static void handle_presentation_time(
            void *data,
            struct ::ext_image_copy_capture_frame_v1 *object,
            uint32_t tv_sec_hi,
            uint32_t tv_sec_lo,
            uint32_t tv_nsec);
        static void handle_ready(
            void *data,
            struct ::ext_image_copy_capture_frame_v1 *object);
        static void handle_failed(
            void *data,
            struct ::ext_image_copy_capture_frame_v1 *object,
            uint32_t reason);
        struct ::ext_image_copy_capture_frame_v1 *m_ext_image_copy_capture_frame_v1;
    };

    class  ext_image_copy_capture_cursor_session_v1
    {
    public:
        ext_image_copy_capture_cursor_session_v1(struct ::wl_registry *registry, uint32_t id, int version);
        ext_image_copy_capture_cursor_session_v1(struct ::ext_image_copy_capture_cursor_session_v1 *object);
        ext_image_copy_capture_cursor_session_v1();

        virtual ~ext_image_copy_capture_cursor_session_v1();

        void init(struct ::wl_registry *registry, uint32_t id, int version);
        void init(struct ::ext_image_copy_capture_cursor_session_v1 *object);

        struct ::ext_image_copy_capture_cursor_session_v1 *object() { return m_ext_image_copy_capture_cursor_session_v1; }
        const struct ::ext_image_copy_capture_cursor_session_v1 *object() const { return m_ext_image_copy_capture_cursor_session_v1; }
        static ext_image_copy_capture_cursor_session_v1 *fromObject(struct ::ext_image_copy_capture_cursor_session_v1 *object);

        bool isInitialized() const;

        uint32_t version() const;
        static const struct ::wl_interface *interface();

        enum error {
            error_duplicate_session = 1, // get_capture_session sent twice
        };

        void destroy();
        struct ::ext_image_copy_capture_session_v1 *get_capture_session();

    protected:
        virtual void ext_image_copy_capture_cursor_session_v1_enter();
        virtual void ext_image_copy_capture_cursor_session_v1_leave();
        virtual void ext_image_copy_capture_cursor_session_v1_position(int32_t x, int32_t y);
        virtual void ext_image_copy_capture_cursor_session_v1_hotspot(int32_t x, int32_t y);

    private:
        void init_listener();
        static const struct ext_image_copy_capture_cursor_session_v1_listener m_ext_image_copy_capture_cursor_session_v1_listener;
        static void handle_enter(
            void *data,
            struct ::ext_image_copy_capture_cursor_session_v1 *object);
        static void handle_leave(
            void *data,
            struct ::ext_image_copy_capture_cursor_session_v1 *object);
        static void handle_position(
            void *data,
            struct ::ext_image_copy_capture_cursor_session_v1 *object,
            int32_t x,
            int32_t y);
        static void handle_hotspot(
            void *data,
            struct ::ext_image_copy_capture_cursor_session_v1 *object,
            int32_t x,
            int32_t y);
        struct ::ext_image_copy_capture_cursor_session_v1 *m_ext_image_copy_capture_cursor_session_v1;
    };
}

QT_WARNING_POP
QT_END_NAMESPACE

#endif
