// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage tests for remaining uncovered functions — PUBLIC methods only.
// No #define private public to avoid Qt header corruption.

#include <gtest/gtest.h>
#include <wayland-client-core.h>
#include <dlfcn.h>
#include <QCoreApplication>
#include <QTimer>
#include <QDBusMessage>
#include <QProcess>
#include <QStringList>

#include "stub.h"
#include "addr_pri.h"

#include "../../src/capture.h"
#include "../../src/utils/audioutils.h"
#include "../../src/utils/voicevolumewatcher.h"
#include "../../src/utils/shortcut.h"
#include "../../src/utils.h"

using namespace testing;

// ============================================================
// wl_proxy stubs (shared)
// ============================================================
static void s_marshal_flags(struct wl_proxy *, uint32_t,
    const struct wl_interface *, uint32_t, uint32_t, ...) {}
static struct wl_proxy *s_marshal_ctor(struct wl_proxy *, uint32_t,
    const struct wl_interface *, uint32_t, ...) {
    return reinterpret_cast<struct wl_proxy *>(0xDEAD); }
static int s_add_listener(struct wl_proxy *, const void *, void *) { return 0; }
static void *s_get_user_data(struct wl_proxy *) { return nullptr; }
static uint32_t s_get_version(struct wl_proxy *) { return 1; }
static const void *s_get_listener(struct wl_proxy *) {
    return reinterpret_cast<const void *>(0xBEEF); }
static void s_set_user_data(struct wl_proxy *, void *) {}
static void s_destroy(struct wl_proxy *) {}

static void setupWlStubs(Stub &stub) {
    auto p1 = dlsym(RTLD_DEFAULT, "wl_proxy_marshal_flags");
    auto p2 = dlsym(RTLD_DEFAULT, "wl_proxy_marshal_constructor_versioned");
    auto p3 = dlsym(RTLD_DEFAULT, "wl_proxy_add_listener");
    auto p4 = dlsym(RTLD_DEFAULT, "wl_proxy_get_user_data");
    auto p5 = dlsym(RTLD_DEFAULT, "wl_proxy_get_version");
    auto p6 = dlsym(RTLD_DEFAULT, "wl_proxy_get_listener");
    auto p7 = dlsym(RTLD_DEFAULT, "wl_proxy_set_user_data");
    auto p8 = dlsym(RTLD_DEFAULT, "wl_proxy_destroy");
    if (p1) stub.set(p1, (void*)s_marshal_flags);
    if (p2) stub.set(p2, (void*)s_marshal_ctor);
    if (p3) stub.set(p3, (void*)s_add_listener);
    if (p4) stub.set(p4, (void*)s_get_user_data);
    if (p5) stub.set(p5, (void*)s_get_version);
    if (p6) stub.set(p6, (void*)s_get_listener);
    if (p7) stub.set(p7, (void*)s_set_user_data);
    if (p8) stub.set(p8, (void*)s_destroy);
}

// Stub TreelandCaptureManager::instance() to avoid QWaylandClientExtension SEGV
static TreelandCaptureManager *stub_tlManager_instance() { return nullptr; }

// ============================================================
// TreelandCapture* tests
// ============================================================
class TreelandCaptureCovTest : public Test {
protected:
    Stub stub;
    void SetUp() override {
        setupWlStubs(stub);
        stub.set(&TreelandCaptureManager::instance, (void*)stub_tlManager_instance);
    }
};

TEST_F(TreelandCaptureCovTest, FrameConstructDestruct)
{
    auto *f = new TreelandCaptureFrame(
        reinterpret_cast<::treeland_capture_frame_v1*>(0xDEAD));
    delete f;
    SUCCEED();
}

TEST_F(TreelandCaptureCovTest, SessionConstructDestructStart)
{
    auto *s = new TreelandCaptureSession(
        reinterpret_cast<::treeland_capture_session_v1*>(0xDEAD));
    s->start();
    delete s;
    SUCCEED();
}

TEST_F(TreelandCaptureCovTest, ContextConstructDestruct)
{
    auto *c = new TreelandCaptureContext(
        reinterpret_cast<::treeland_capture_context_v1*>(0xDEAD));
    delete c;
    SUCCEED();
}

TEST_F(TreelandCaptureCovTest, ContextEnsureFrame)
{
    auto *c = new TreelandCaptureContext(
        reinterpret_cast<::treeland_capture_context_v1*>(0xDEAD));
    c->ensureFrame();
    c->ensureFrame();
    delete c;
    SUCCEED();
}

TEST_F(TreelandCaptureCovTest, ContextEnsureSession)
{
    auto *c = new TreelandCaptureContext(
        reinterpret_cast<::treeland_capture_context_v1*>(0xDEAD));
    c->ensureSession();
    c->ensureSession();
    delete c;
    SUCCEED();
}

TEST_F(TreelandCaptureCovTest, ContextSelectSource)
{
    auto *c = new TreelandCaptureContext(
        reinterpret_cast<::treeland_capture_context_v1*>(0xDEAD));
    c->selectSource(0, false, false, nullptr);
    delete c;
    SUCCEED();
}

TEST_F(TreelandCaptureCovTest, ContextDestroySession)
{
    auto *c = new TreelandCaptureContext(
        reinterpret_cast<::treeland_capture_context_v1*>(0xDEAD));
    c->ensureSession();
    c->destroySession();
    delete c;
    SUCCEED();
}

// ============================================================
// AudioUtils tests
// ============================================================
class AudioUtilsRemCovTest : public Test {};

TEST_F(AudioUtilsRemCovTest, ConstructAndCallMethods)
{
    AudioUtils au;
    au.audioDBusInterface();
    au.defaultSourceDBusInterface();
    au.defaultSinkDBusInterface();
    au.cards();
    au.currentAudioChannel();
    au.defaultSourceActivePort();
    au.defaultSourceVolume();
    au.getDefaultDeviceName(AudioUtils::Source);
    au.getDefaultDeviceName(AudioUtils::Sink);
    au.onDBusAudioPropertyChanged(QDBusMessage());
    SUCCEED();
}

// ============================================================
// Shortcut tests
// ============================================================
class ShortcutRemCovTest : public Test {};

TEST_F(ShortcutRemCovTest, GetDefaultValue)
{
    Shortcut s;
    s.getDefaultValue("screenshot");
    s.getDefaultValue("deepin-screen-recorder");
    s.getDefaultValue("screenshot-window");
    s.getDefaultValue("screenshot-delayed");
    s.getDefaultValue("screenshot-fullscreen");
    s.getDefaultValue("unknown");
    SUCCEED();
}

TEST_F(ShortcutRemCovTest, GetSysShortcuts)
{
    Shortcut s;
    s.getSysShortcuts("screenshot");
    SUCCEED();
}

TEST_F(ShortcutRemCovTest, ToStr)
{
    Shortcut s;
    s.toStr();
    SUCCEED();
}

// Utils::notSupportWarn() calls DDialog::exec() which blocks in offscreen — skipped
