// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage round 2 for the qtwaylandscanner-generated protocol wrappers.
//
// Round 1 (ut_protocols_cov.h) covered:
//   - default ctor, isInitialized/object/interface/fromObject/version
//   - the protected virtual no-op overrides (ext_*_v1_xxx, zwp_*_xxx,
//     treeland_*_xxx) via public-subclass using-declarations.
//
// What round 1 deliberately skipped and this file targets:
//   - the PRIVATE static `handle_*` listener dispatchers (4-7 per class).
//     These are ~50% of every generated .cpp body and were 100% uncovered.
//     They are pure data-marshalling: Q_UNUSED(object), then a
//     static_cast<ThisClass*>(data)->virtual_noop(...).  Calling them with
//     data = &defaultConstructedInstance and object = nullptr is safe --
//     the virtual bodies are empty and object is never dereferenced.
//
// Risk note: these `handle_*` are PRIVATE static. ACCESS_PRIVATE_FUN is
// required.  SameTU dedup: NONE of these signatures are declared anywhere
// else (verified via grep).  We never touch wl_proxy_* paths.
//
// NOT targeted here:
//   - treeland_capture_frame_v1/context_v1/manager_v1 (covered enough by
//     round 1; manager has no listener; frame/context destroy() is guarded
//     and safe but already exercised via the default-ctor path).
//   - ext-image-capture non-wrapper classes: ext_capture_cov2.h plus the
//     long list of ut_extcapture*_cov.h already exhaust their public API.

#pragma once
#include <gtest/gtest.h>
#include <wayland-client-core.h>
#include "addr_pri.h"

#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"
#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-foreign-toplevel-list-v1.h"
#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-image-capture-source-v1.h"
#include "../../src/protocols/linux-dmabuf/qwayland-linux-dmabuf-unstable-v1.h"
#include "../../src/qwayland-treeland-capture-unstable-v1.h"

using namespace testing;

// ============================================================
// ext-image-copy-capture-v1 -- private static handle_* dispatchers
// ============================================================
// Public subclasses expose the protected virtual no-op overrides (round 1).
// The handle_* dispatchers are PRIVATE static -- ACCESS_PRIVATE_FUN here.
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_session_v1 *, uint32_t, uint32_t),
    handle_buffer_size);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_session_v1 *, uint32_t),
    handle_shm_format);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_session_v1 *, wl_array *),
    handle_dmabuf_device);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_session_v1 *, uint32_t, wl_array *),
    handle_dmabuf_format);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_session_v1 *),
    handle_done);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_session_v1 *),
    handle_stopped);

ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_frame_v1,
    void(*)(void *, struct ::ext_image_copy_capture_frame_v1 *, uint32_t),
    handle_transform);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_frame_v1,
    void(*)(void *, struct ::ext_image_copy_capture_frame_v1 *, int32_t, int32_t, int32_t, int32_t),
    handle_damage);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_frame_v1,
    void(*)(void *, struct ::ext_image_copy_capture_frame_v1 *, uint32_t, uint32_t, uint32_t),
    handle_presentation_time);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_frame_v1,
    void(*)(void *, struct ::ext_image_copy_capture_frame_v1 *),
    handle_ready);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_frame_v1,
    void(*)(void *, struct ::ext_image_copy_capture_frame_v1 *, uint32_t),
    handle_failed);

ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_cursor_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_cursor_session_v1 *),
    handle_enter);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_cursor_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_cursor_session_v1 *),
    handle_leave);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_cursor_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_cursor_session_v1 *, int32_t, int32_t),
    handle_position);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_image_copy_capture_cursor_session_v1,
    void(*)(void *, struct ::ext_image_copy_capture_cursor_session_v1 *, int32_t, int32_t),
    handle_hotspot);

// Subclasses that re-expose the protected virtuals (same shape as round 1).
class Cov2IccSession : public QtWayland::ext_image_copy_capture_session_v1 {};
class Cov2IccFrame   : public QtWayland::ext_image_copy_capture_frame_v1 {};
class Cov2IccCursor  : public QtWayland::ext_image_copy_capture_cursor_session_v1 {};

class ProtocolsExtImgCov2Test : public Test {};

// --- session_v1 handle_* ---
TEST_F(ProtocolsExtImgCov2Test, IccSessionHandlersDispatch)
{
    Cov2IccSession s;
    void *data = &s;
    wl_array arr{}; arr.size = 0; arr.alloc = 0; arr.data = nullptr;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1handle_buffer_size(
            data, nullptr, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1handle_shm_format(
            data, nullptr, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1handle_dmabuf_device(
            data, nullptr, &arr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1handle_dmabuf_format(
            data, nullptr, 0, &arr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1handle_done(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1handle_stopped(
            data, nullptr));
}

// --- frame_v1 handle_* ---
TEST_F(ProtocolsExtImgCov2Test, IccFrameHandlersDispatch)
{
    Cov2IccFrame f;
    void *data = &f;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1handle_transform(
            data, nullptr, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1handle_damage(
            data, nullptr, 0, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1handle_presentation_time(
            data, nullptr, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1handle_ready(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1handle_failed(
            data, nullptr, 0));
}

// --- cursor_session_v1 handle_* ---
TEST_F(ProtocolsExtImgCov2Test, IccCursorHandlersDispatch)
{
    Cov2IccCursor c;
    void *data = &c;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_cursor_session_v1::ext_image_copy_capture_cursor_session_v1handle_enter(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_cursor_session_v1::ext_image_copy_capture_cursor_session_v1handle_leave(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_cursor_session_v1::ext_image_copy_capture_cursor_session_v1handle_position(
            data, nullptr, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_image_copy_capture_cursor_session_v1::ext_image_copy_capture_cursor_session_v1handle_hotspot(
            data, nullptr, 0, 0));
}

// ============================================================
// ext-foreign-toplevel-list-v1 -- private static handle_* dispatchers
// ============================================================
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_foreign_toplevel_list_v1,
    void(*)(void *, struct ::ext_foreign_toplevel_list_v1 *, struct ::ext_foreign_toplevel_handle_v1 *),
    handle_toplevel);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_foreign_toplevel_list_v1,
    void(*)(void *, struct ::ext_foreign_toplevel_list_v1 *),
    handle_finished);

ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_foreign_toplevel_handle_v1,
    void(*)(void *, struct ::ext_foreign_toplevel_handle_v1 *),
    handle_closed);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_foreign_toplevel_handle_v1,
    void(*)(void *, struct ::ext_foreign_toplevel_handle_v1 *),
    handle_done);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_foreign_toplevel_handle_v1,
    void(*)(void *, struct ::ext_foreign_toplevel_handle_v1 *, const char *),
    handle_title);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_foreign_toplevel_handle_v1,
    void(*)(void *, struct ::ext_foreign_toplevel_handle_v1 *, const char *),
    handle_app_id);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::ext_foreign_toplevel_handle_v1,
    void(*)(void *, struct ::ext_foreign_toplevel_handle_v1 *, const char *),
    handle_identifier);

class Cov2ForeignList   : public QtWayland::ext_foreign_toplevel_list_v1 {};
class Cov2ForeignHandle : public QtWayland::ext_foreign_toplevel_handle_v1 {};

TEST_F(ProtocolsExtImgCov2Test, ForeignListHandlersDispatch)
{
    Cov2ForeignList l;
    void *data = &l;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_foreign_toplevel_list_v1::ext_foreign_toplevel_list_v1handle_toplevel(
            data, nullptr, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_foreign_toplevel_list_v1::ext_foreign_toplevel_list_v1handle_finished(
            data, nullptr));
}

TEST_F(ProtocolsExtImgCov2Test, ForeignHandleHandlersDispatch)
{
    Cov2ForeignHandle h;
    void *data = &h;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1handle_closed(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1handle_done(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1handle_title(
            data, nullptr, "t"));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1handle_app_id(
            data, nullptr, "a"));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1handle_identifier(
            data, nullptr, "i"));
}

// ============================================================
// linux-dmabuf-unstable-v1 -- handle_* for zwp_linux_dmabuf_v1 and
// zwp_linux_buffer_params_v1 (both have listeners).
// ============================================================
ACCESS_PRIVATE_STATIC_FUN(QtWayland::zwp_linux_dmabuf_v1,
    void(*)(void *, struct ::zwp_linux_dmabuf_v1 *, uint32_t),
    handle_format);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::zwp_linux_dmabuf_v1,
    void(*)(void *, struct ::zwp_linux_dmabuf_v1 *, uint32_t, uint32_t, uint32_t),
    handle_modifier);

ACCESS_PRIVATE_STATIC_FUN(QtWayland::zwp_linux_buffer_params_v1,
    void(*)(void *, struct ::zwp_linux_buffer_params_v1 *, struct ::wl_buffer *),
    handle_created);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::zwp_linux_buffer_params_v1,
    void(*)(void *, struct ::zwp_linux_buffer_params_v1 *),
    handle_failed);

class Cov2Dmabuf        : public QtWayland::zwp_linux_dmabuf_v1 {};
class Cov2DmabufParams  : public QtWayland::zwp_linux_buffer_params_v1 {};

TEST_F(ProtocolsExtImgCov2Test, DmabufHandlersDispatch)
{
    Cov2Dmabuf d;
    void *data = &d;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::zwp_linux_dmabuf_v1::zwp_linux_dmabuf_v1handle_format(data, nullptr, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::zwp_linux_dmabuf_v1::zwp_linux_dmabuf_v1handle_modifier(
            data, nullptr, 0, 0, 0));
}

TEST_F(ProtocolsExtImgCov2Test, DmabufParamsHandlersDispatch)
{
    Cov2DmabufParams p;
    void *data = &p;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::zwp_linux_buffer_params_v1::zwp_linux_buffer_params_v1handle_created(
            data, nullptr, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::zwp_linux_buffer_params_v1::zwp_linux_buffer_params_v1handle_failed(
            data, nullptr));
}

// ============================================================
// treeland-capture-unstable-v1 -- handle_* for session/frame/context.
// session:  frame/object/ready/cancel.
// frame:    buffer/buffer_done/flags/ready/failed.
// context:  source_ready/source_failed.
// Additionally treeland destroy() is guarded by isInitialized() and is
// safe to call on a default-constructed instance (early-returns).
// ============================================================
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_session_v1,
    void(*)(void *, struct ::treeland_capture_session_v1 *,
            int32_t, int32_t, uint32_t, uint32_t, uint32_t, uint32_t,
            uint32_t, uint32_t, uint32_t),
    handle_frame);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_session_v1,
    void(*)(void *, struct ::treeland_capture_session_v1 *,
            uint32_t, int32_t, uint32_t, uint32_t, uint32_t, uint32_t),
    handle_object);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_session_v1,
    void(*)(void *, struct ::treeland_capture_session_v1 *,
            uint32_t, uint32_t, uint32_t),
    handle_ready);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_session_v1,
    void(*)(void *, struct ::treeland_capture_session_v1 *, uint32_t),
    handle_cancel);

ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_frame_v1,
    void(*)(void *, struct ::treeland_capture_frame_v1 *,
            uint32_t, uint32_t, uint32_t, uint32_t),
    handle_buffer);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_frame_v1,
    void(*)(void *, struct ::treeland_capture_frame_v1 *),
    handle_buffer_done);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_frame_v1,
    void(*)(void *, struct ::treeland_capture_frame_v1 *, uint32_t),
    handle_flags);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_frame_v1,
    void(*)(void *, struct ::treeland_capture_frame_v1 *),
    handle_ready);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_frame_v1,
    void(*)(void *, struct ::treeland_capture_frame_v1 *),
    handle_failed);

ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_context_v1,
    void(*)(void *, struct ::treeland_capture_context_v1 *,
            int32_t, int32_t, uint32_t, uint32_t, uint32_t),
    handle_source_ready);
ACCESS_PRIVATE_STATIC_FUN(QtWayland::treeland_capture_context_v1,
    void(*)(void *, struct ::treeland_capture_context_v1 *, uint32_t),
    handle_source_failed);

class Cov2TreelandSession : public QtWayland::treeland_capture_session_v1 {};
class Cov2TreelandFrame   : public QtWayland::treeland_capture_frame_v1 {};
class Cov2TreelandContext : public QtWayland::treeland_capture_context_v1 {};

TEST_F(ProtocolsExtImgCov2Test, TreelandSessionHandlersDispatch)
{
    Cov2TreelandSession s;
    void *data = &s;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_session_v1::treeland_capture_session_v1handle_frame(
            data, nullptr, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_session_v1::treeland_capture_session_v1handle_object(
            data, nullptr, 0, 0, 0, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_session_v1::treeland_capture_session_v1handle_ready(
            data, nullptr, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_session_v1::treeland_capture_session_v1handle_cancel(
            data, nullptr, 0));
    // destroy() is guarded by isInitialized() -> safe early-return here.
    EXPECT_NO_FATAL_FAILURE(s.destroy());
}

TEST_F(ProtocolsExtImgCov2Test, TreelandFrameHandlersDispatch)
{
    Cov2TreelandFrame f;
    void *data = &f;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_frame_v1::treeland_capture_frame_v1handle_buffer(
            data, nullptr, 0, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_frame_v1::treeland_capture_frame_v1handle_buffer_done(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_frame_v1::treeland_capture_frame_v1handle_flags(
            data, nullptr, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_frame_v1::treeland_capture_frame_v1handle_ready(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_frame_v1::treeland_capture_frame_v1handle_failed(
            data, nullptr));
    EXPECT_NO_FATAL_FAILURE(f.destroy());
}

TEST_F(ProtocolsExtImgCov2Test, TreelandContextHandlersDispatch)
{
    Cov2TreelandContext c;
    void *data = &c;
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_context_v1::treeland_capture_context_v1handle_source_ready(
            data, nullptr, 0, 0, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(
        call_private_static_fun::QtWayland::treeland_capture_context_v1::treeland_capture_context_v1handle_source_failed(
            data, nullptr, 0));
    EXPECT_NO_FATAL_FAILURE(c.destroy());
}
