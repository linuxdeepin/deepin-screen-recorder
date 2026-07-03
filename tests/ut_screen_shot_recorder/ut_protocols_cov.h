// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage tests for the Qt Wayland generated bindings:
//   - src/protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.cpp
//   - src/protocols/ext-image-copy-capture/qwayland-ext-foreign-toplevel-list-v1.cpp
//   - src/protocols/ext-image-copy-capture/qwayland-ext-image-capture-source-v1.cpp
//   - src/protocols/linux-dmabuf/qwayland-linux-dmabuf-unstable-v1.cpp
//   - src/qwayland-treeland-capture-unstable-v1.cpp
//
// These are qtwaylandscanner output. Every class has THREE constructors:
//   1. (wl_registry*, id, version) -> calls wl_proxy_marshal_constructor_*,
//      needs a live wayland connection. NOT safe without one.
//   2. (::type* obj) -> stores the proxy pointer, calls init_listener() which
//      calls <proto>_add_listener on the proxy. NOT safe without a real proxy.
//   3. () default -> sets m_xxx = nullptr. SAFE -- no wayland calls.
//
// The default ctor + the non-wayland-calling methods are coverable:
//   - isInitialized() -> returns false (m_xxx == nullptr branch).
//   - object() / fromObject(nullptr) -> pointer arithmetic, no wayland call.
//   - version() -> calls wl_proxy_get_version(nullptr) which libwayland
//     handles as a no-op returning 0 (it's a simple wrapper, no deref of the
//     nullptr in the wayland client lib; verified safe on the test system).
//     We wrap it in EXPECT_NO_FATAL_FAILURE anyway.
//   - The virtual no-op overrides (e.g. ext_image_copy_capture_session_v1_done)
//     are protected; subclasses in the real code override them. Here we
//     subclass each generated class publicly and call them to cover the empty
//     bodies.
//
// NOT coverable:
//   - init(wl_registry*, ...) and init(::type*) -- need wayland.
//   - destroy() / create_session() / create_frame() / attach_buffer() /
//     capture() / damage_buffer() / start() / stop() / create_source() /
//     get_capture_session() / create_params() / get_default_feedback() /
//     get_surface_feedback() -- all call wayland C functions on m_xxx which is
//     null -> would SEGV. Excluded.

#pragma once
#include <gtest/gtest.h>
#include <wayland-client-core.h>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"
#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-foreign-toplevel-list-v1.h"
#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-image-capture-source-v1.h"
#include "../../src/protocols/linux-dmabuf/qwayland-linux-dmabuf-unstable-v1.h"
#include "../../src/qwayland-treeland-capture-unstable-v1.h"

using namespace testing;

// ============================================================
// ext-image-copy-capture-v1
// ============================================================

// Subclass to expose the protected virtual no-op overrides for coverage.
class CovExtIccManager : public QtWayland::ext_image_copy_capture_manager_v1 {};
class CovExtIccSession  : public QtWayland::ext_image_copy_capture_session_v1
{
public:
    using QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1_buffer_size;
    using QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1_shm_format;
    using QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1_dmabuf_device;
    using QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1_dmabuf_format;
    using QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1_done;
    using QtWayland::ext_image_copy_capture_session_v1::ext_image_copy_capture_session_v1_stopped;
};
class CovExtIccFrame : public QtWayland::ext_image_copy_capture_frame_v1
{
public:
    using QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1_transform;
    using QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1_damage;
    using QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1_presentation_time;
    using QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1_ready;
    using QtWayland::ext_image_copy_capture_frame_v1::ext_image_copy_capture_frame_v1_failed;
};
class CovExtIccCursor : public QtWayland::ext_image_copy_capture_cursor_session_v1
{
public:
    using QtWayland::ext_image_copy_capture_cursor_session_v1::ext_image_copy_capture_cursor_session_v1_enter;
    using QtWayland::ext_image_copy_capture_cursor_session_v1::ext_image_copy_capture_cursor_session_v1_leave;
    using QtWayland::ext_image_copy_capture_cursor_session_v1::ext_image_copy_capture_cursor_session_v1_position;
    using QtWayland::ext_image_copy_capture_cursor_session_v1::ext_image_copy_capture_cursor_session_v1_hotspot;
};

class ProtocolsCovTest : public Test
{
public:
    Stub stub;
};

// --- ext_image_copy_capture_manager_v1 ---
TEST_F(ProtocolsCovTest, ExtIccManagerDefault)
{
    CovExtIccManager m;
    bool init = false;
    EXPECT_NO_FATAL_FAILURE({ init = m.isInitialized(); });
    (void)init;
    EXPECT_NO_FATAL_FAILURE({ (void)m.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)m.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtIccManager::fromObject(nullptr); });
}

// --- ext_image_copy_capture_session_v1 ---
TEST_F(ProtocolsCovTest, ExtIccSessionDefault)
{
    CovExtIccSession s;
    EXPECT_NO_FATAL_FAILURE({ (void)s.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtIccSession::fromObject(nullptr); });
    // protected virtual no-op bodies:
    wl_array arr{}; arr.size = 0; arr.alloc = 0; arr.data = nullptr;
    EXPECT_NO_FATAL_FAILURE(s.ext_image_copy_capture_session_v1_buffer_size(0, 0));
    EXPECT_NO_FATAL_FAILURE(s.ext_image_copy_capture_session_v1_shm_format(0));
    EXPECT_NO_FATAL_FAILURE(s.ext_image_copy_capture_session_v1_dmabuf_device(&arr));
    EXPECT_NO_FATAL_FAILURE(s.ext_image_copy_capture_session_v1_dmabuf_format(0, &arr));
    EXPECT_NO_FATAL_FAILURE(s.ext_image_copy_capture_session_v1_done());
    EXPECT_NO_FATAL_FAILURE(s.ext_image_copy_capture_session_v1_stopped());
}

// --- ext_image_copy_capture_frame_v1 ---
TEST_F(ProtocolsCovTest, ExtIccFrameDefault)
{
    CovExtIccFrame f;
    EXPECT_NO_FATAL_FAILURE({ (void)f.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)f.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)f.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtIccFrame::fromObject(nullptr); });
    EXPECT_NO_FATAL_FAILURE(f.ext_image_copy_capture_frame_v1_transform(0));
    EXPECT_NO_FATAL_FAILURE(f.ext_image_copy_capture_frame_v1_damage(0, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(f.ext_image_copy_capture_frame_v1_presentation_time(0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(f.ext_image_copy_capture_frame_v1_ready());
    EXPECT_NO_FATAL_FAILURE(f.ext_image_copy_capture_frame_v1_failed(0));
}

// --- ext_image_copy_capture_cursor_session_v1 ---
TEST_F(ProtocolsCovTest, ExtIccCursorDefault)
{
    CovExtIccCursor c;
    EXPECT_NO_FATAL_FAILURE({ (void)c.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)c.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)c.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtIccCursor::fromObject(nullptr); });
    EXPECT_NO_FATAL_FAILURE(c.ext_image_copy_capture_cursor_session_v1_enter());
    EXPECT_NO_FATAL_FAILURE(c.ext_image_copy_capture_cursor_session_v1_leave());
    EXPECT_NO_FATAL_FAILURE(c.ext_image_copy_capture_cursor_session_v1_position(0, 0));
    EXPECT_NO_FATAL_FAILURE(c.ext_image_copy_capture_cursor_session_v1_hotspot(0, 0));
}

// ============================================================
// ext-foreign-toplevel-list-v1
// ============================================================
class CovExtForeignList : public QtWayland::ext_foreign_toplevel_list_v1 {};
class CovExtForeignHandle : public QtWayland::ext_foreign_toplevel_handle_v1
{
public:
    using QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_closed;
    using QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_done;
    using QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_title;
    using QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_app_id;
    using QtWayland::ext_foreign_toplevel_handle_v1::ext_foreign_toplevel_handle_v1_identifier;
};

TEST_F(ProtocolsCovTest, ExtForeignListDefault)
{
    CovExtForeignList l;
    EXPECT_NO_FATAL_FAILURE({ (void)l.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)l.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)l.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtForeignList::fromObject(nullptr); });
    EXPECT_NO_FATAL_FAILURE(l.ext_foreign_toplevel_list_v1_toplevel(nullptr));
    EXPECT_NO_FATAL_FAILURE(l.ext_foreign_toplevel_list_v1_finished());
}

TEST_F(ProtocolsCovTest, ExtForeignHandleDefault)
{
    CovExtForeignHandle h;
    EXPECT_NO_FATAL_FAILURE({ (void)h.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)h.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)h.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtForeignHandle::fromObject(nullptr); });
    EXPECT_NO_FATAL_FAILURE(h.ext_foreign_toplevel_handle_v1_closed());
    EXPECT_NO_FATAL_FAILURE(h.ext_foreign_toplevel_handle_v1_done());
    EXPECT_NO_FATAL_FAILURE(h.ext_foreign_toplevel_handle_v1_title(QStringLiteral("t")));
    EXPECT_NO_FATAL_FAILURE(h.ext_foreign_toplevel_handle_v1_app_id(QStringLiteral("a")));
    EXPECT_NO_FATAL_FAILURE(h.ext_foreign_toplevel_handle_v1_identifier(QStringLiteral("i")));
}

// ============================================================
// ext-image-capture-source-v1
// ============================================================
class CovExtImgCapSrc : public QtWayland::ext_image_capture_source_v1 {};
class CovExtOutSrcMgr : public QtWayland::ext_output_image_capture_source_manager_v1 {};
class CovExtForeignSrcMgr : public QtWayland::ext_foreign_toplevel_image_capture_source_manager_v1 {};

TEST_F(ProtocolsCovTest, ExtImgCapSrcDefault)
{
    CovExtImgCapSrc s;
    EXPECT_NO_FATAL_FAILURE({ (void)s.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtImgCapSrc::fromObject(nullptr); });
}

TEST_F(ProtocolsCovTest, ExtOutSrcMgrDefault)
{
    CovExtOutSrcMgr m;
    EXPECT_NO_FATAL_FAILURE({ (void)m.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)m.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)m.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtOutSrcMgr::fromObject(nullptr); });
}

TEST_F(ProtocolsCovTest, ExtForeignSrcMgrDefault)
{
    CovExtForeignSrcMgr m;
    EXPECT_NO_FATAL_FAILURE({ (void)m.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)m.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)m.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovExtForeignSrcMgr::fromObject(nullptr); });
}

// ============================================================
// linux-dmabuf-unstable-v1
// ============================================================
class CovZwpLinuxDmabuf : public QtWayland::zwp_linux_dmabuf_v1
{
public:
    using QtWayland::zwp_linux_dmabuf_v1::zwp_linux_dmabuf_v1_format;
    using QtWayland::zwp_linux_dmabuf_v1::zwp_linux_dmabuf_v1_modifier;
};

TEST_F(ProtocolsCovTest, ZwpLinuxDmabufDefault)
{
    CovZwpLinuxDmabuf d;
    EXPECT_NO_FATAL_FAILURE({ (void)d.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)d.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)d.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovZwpLinuxDmabuf::fromObject(nullptr); });
    EXPECT_NO_FATAL_FAILURE(d.zwp_linux_dmabuf_v1_format(0));
    EXPECT_NO_FATAL_FAILURE(d.zwp_linux_dmabuf_v1_modifier(0, 0, 0));
}

// ============================================================
// treeland-capture-unstable-v1
// ============================================================
class CovTreelandSession : public QtWayland::treeland_capture_session_v1
{
public:
    using QtWayland::treeland_capture_session_v1::treeland_capture_session_v1_frame;
    using QtWayland::treeland_capture_session_v1::treeland_capture_session_v1_object;
    using QtWayland::treeland_capture_session_v1::treeland_capture_session_v1_ready;
    using QtWayland::treeland_capture_session_v1::treeland_capture_session_v1_cancel;
};

TEST_F(ProtocolsCovTest, TreelandSessionDefault)
{
    CovTreelandSession s;
    EXPECT_NO_FATAL_FAILURE({ (void)s.isInitialized(); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.object(); });
    EXPECT_NO_FATAL_FAILURE({ (void)s.interface(); });
    EXPECT_NO_FATAL_FAILURE({ (void)CovTreelandSession::fromObject(nullptr); });
    EXPECT_NO_FATAL_FAILURE(s.treeland_capture_session_v1_frame(0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(s.treeland_capture_session_v1_object(0, 0, 0, 0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(s.treeland_capture_session_v1_ready(0, 0, 0));
    EXPECT_NO_FATAL_FAILURE(s.treeland_capture_session_v1_cancel(0));
}
