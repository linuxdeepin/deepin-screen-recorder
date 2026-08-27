// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Full coverage for qtwaylandscanner-generated protocol wrappers.
// The private static handle_* dispatchers are tested in
// protocols_handle_cov.cpp (uses #define private public).
//
// This header covers marshal methods, init, fromObject, and
// (registry)/(obj) constructors via wl_proxy stubs.

#pragma once
#include <gtest/gtest.h>
#include <wayland-client-core.h>
#include <dlfcn.h>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"
#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-foreign-toplevel-list-v1.h"
#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-image-capture-source-v1.h"
#include "../../src/protocols/linux-dmabuf/qwayland-linux-dmabuf-unstable-v1.h"
#include "../../src/qwayland-treeland-capture-unstable-v1.h"

using namespace testing;

// ============================================================
// wl_proxy stubs for marshal methods, init, fromObject
// ============================================================

static void stub_wl_proxy_marshal_flags(struct wl_proxy *, uint32_t,
    const struct wl_interface *, uint32_t, uint32_t, ...) {}
static struct wl_proxy *stub_wl_proxy_marshal_ctor(struct wl_proxy *, uint32_t,
    const struct wl_interface *, uint32_t, ...) {
    return reinterpret_cast<struct wl_proxy *>(0xDEAD); }
static int stub_wl_proxy_add_listener(struct wl_proxy *, const void *, void *) { return 0; }
static void *stub_wl_proxy_get_user_data(struct wl_proxy *) { return nullptr; }
static uint32_t stub_wl_proxy_get_version(struct wl_proxy *) { return 1; }
static const void *stub_wl_proxy_get_listener(struct wl_proxy *) {
    return reinterpret_cast<const void *>(0xBEEF); }
static void stub_wl_proxy_set_user_data(struct wl_proxy *, void *) {}
static void stub_wl_proxy_destroy(struct wl_proxy *) {}

class ProtocolsWlStubCovTest : public Test {
protected:
    Stub stub;
    void SetUp() override {
        auto p1 = dlsym(RTLD_DEFAULT, "wl_proxy_marshal_flags");
        auto p2 = dlsym(RTLD_DEFAULT, "wl_proxy_marshal_constructor_versioned");
        auto p3 = dlsym(RTLD_DEFAULT, "wl_proxy_add_listener");
        auto p4 = dlsym(RTLD_DEFAULT, "wl_proxy_get_user_data");
        auto p5 = dlsym(RTLD_DEFAULT, "wl_proxy_get_version");
        auto p6 = dlsym(RTLD_DEFAULT, "wl_proxy_get_listener");
        auto p7 = dlsym(RTLD_DEFAULT, "wl_proxy_set_user_data");
        auto p8 = dlsym(RTLD_DEFAULT, "wl_proxy_destroy");
        if (p1) stub.set(p1, (void*)stub_wl_proxy_marshal_flags);
        if (p2) stub.set(p2, (void*)stub_wl_proxy_marshal_ctor);
        if (p3) stub.set(p3, (void*)stub_wl_proxy_add_listener);
        if (p4) stub.set(p4, (void*)stub_wl_proxy_get_user_data);
        if (p5) stub.set(p5, (void*)stub_wl_proxy_get_version);
        if (p6) stub.set(p6, (void*)stub_wl_proxy_get_listener);
        if (p7) stub.set(p7, (void*)stub_wl_proxy_set_user_data);
        if (p8) stub.set(p8, (void*)stub_wl_proxy_destroy);
    }
};

static constexpr uint32_t kId = 0;
static constexpr int kVer = 1;

#define WL_STUB_TEST(Name, Class, ...) \
TEST_F(ProtocolsWlStubCovTest, Name) { \
    QtWayland::Class obj(nullptr, kId, kVer); \
    EXPECT_TRUE(obj.isInitialized()); \
    obj.destroy(); \
    obj.version(); \
    QtWayland::Class::interface(); \
    auto *fo = QtWayland::Class::fromObject(nullptr); \
    EXPECT_EQ(fo, nullptr); \
    obj.init(reinterpret_cast<struct ::Class*>(0x1000)); \
    __VA_ARGS__; \
    SUCCEED(); \
}

WL_STUB_TEST(IccManagerAll,
    ext_image_copy_capture_manager_v1,
    obj.create_session(nullptr, 0);
    obj.create_pointer_cursor_session(nullptr, nullptr))

WL_STUB_TEST(IccSessionAll,
    ext_image_copy_capture_session_v1,
    obj.create_frame())

WL_STUB_TEST(IccFrameAll,
    ext_image_copy_capture_frame_v1,
    obj.attach_buffer(nullptr);
    obj.damage_buffer(0, 0, 0, 0);
    obj.capture())

WL_STUB_TEST(IccCursorAll,
    ext_image_copy_capture_cursor_session_v1,
    obj.get_capture_session())

WL_STUB_TEST(ForeignListAll,
    ext_foreign_toplevel_list_v1,
    obj.stop())

WL_STUB_TEST(ForeignHandleAll,
    ext_foreign_toplevel_handle_v1, )

WL_STUB_TEST(CaptureSourceAll,
    ext_image_capture_source_v1, )

WL_STUB_TEST(OutputSrcMgrAll,
    ext_output_image_capture_source_manager_v1,
    obj.create_source(nullptr))

WL_STUB_TEST(ToplevelSrcMgrAll,
    ext_foreign_toplevel_image_capture_source_manager_v1,
    obj.create_source(nullptr))

WL_STUB_TEST(DmabufAll,
    zwp_linux_dmabuf_v1,
    obj.create_params();
    obj.get_default_feedback();
    obj.get_surface_feedback(nullptr))

WL_STUB_TEST(DmabufParamsAll,
    zwp_linux_buffer_params_v1,
    obj.add(0, 0, 0, 0, 0, 0);
    obj.create(1, 1, 0, 0);
    obj.create_immed(1, 1, 0, 0))

WL_STUB_TEST(DmabufFeedbackAll,
    zwp_linux_dmabuf_feedback_v1, )

WL_STUB_TEST(TreelandManagerAll,
    treeland_capture_manager_v1,
    obj.get_context())

WL_STUB_TEST(TreelandContextAll,
    treeland_capture_context_v1,
    obj.select_source(0, 0, 0, nullptr);
    obj.capture();
    obj.create_session())

WL_STUB_TEST(TreelandSessionAll,
    treeland_capture_session_v1,
    obj.start())

WL_STUB_TEST(TreelandFrameAll,
    treeland_capture_frame_v1,
    obj.copy(nullptr))
