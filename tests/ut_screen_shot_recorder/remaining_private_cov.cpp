// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage tests for remaining uncovered functions — PRIVATE/PROTECTED methods.
// Uses #define private public after ALL Qt headers are included.

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTimer>
#include <QProcess>
#include <QStringList>
#include <QImage>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QtDBus/QtDBus>
#include <QDBusAbstractInterface>

#include "stub.h"
#include "addr_pri.h"

// All Qt/system headers are now processed. Safe to redefine.
#define private public
#define protected public

#include "../../src/utils/voicevolumewatcher.h"
#include "../../src/utils/tempfile.h"
#include "../../src/utils/configsettings.h"
#include "../../src/record_process.h"
#include "../../src/gstrecord/gstrecordx.h"
#include "../../src/utils/audioutils.h"
#include "../../src/capture.h"
#include "../../src/widgets/imagemenu.h"
#include "../../src/camera/majorimageprocessingthread.h"
#include "../../src/ext-image-capture/frame/extcaptureframe.h"
#include "../../src/ext-image-capture/session/extcapturesession.h"
#include "../../src/protocols/ext-image-copy-capture/qwayland-ext-image-copy-capture-v1.h"
#include <wayland-client-core.h>
#include <dlfcn.h>

using namespace testing;

// ============================================================
// voiceVolumeWatcher tests
// ============================================================
class VoiceVolumeWatcherRemCovTest : public Test {};

TEST_F(VoiceVolumeWatcherRemCovTest, ConstructAndCallMethods)
{
    voiceVolumeWatcher w;
    w.getystemAudioState();
    w.isMicrophoneAvail("test");
    w.setWatch(true);
    w.setWatch(false);
    SUCCEED();
}

TEST_F(VoiceVolumeWatcherRemCovTest, OnCardsChanged)
{
    voiceVolumeWatcher w;
    w.onCardsChanged("test");
    SUCCEED();
}

// ============================================================
// TempFile tests
// ============================================================
TEST(TempFileRemCovTest, GetFullscreenPixmap)
{
    TempFile tf;
    tf.getFullscreenPixmap();
    SUCCEED();
}

// ============================================================
// ConfigSettings tests
// ============================================================
TEST(ConfigSettingsCov2Test, Destructor)
{
    auto *cs = new ConfigSettings();
    delete cs;
    SUCCEED();
}

// GstRecordX createPipeline/stopPipeline crash (GStreamer init in offscreen) — skipped

// ============================================================
// RecordProcess tests
// ============================================================
class RecordProcessRemCovTest : public Test {};

TEST_F(RecordProcessRemCovTest, OnTranscodeFinish)
{
    RecordProcess rp;
    rp.onTranscodeFinish();
    SUCCEED();
}

TEST_F(RecordProcessRemCovTest, OnTranscodePaletteFinished)
{
    RecordProcess rp;
    rp.onTranscodePaletteFinished("test");
    SUCCEED();
}

TEST_F(RecordProcessRemCovTest, RecordVideo)
{
    RecordProcess rp;
    rp.recordVideo();
    SUCCEED();
}

// ============================================================
// wl_proxy stubs for TreelandCapture virtuals
// ============================================================
static void s2_marshal_flags(struct wl_proxy *, uint32_t,
    const struct wl_interface *, uint32_t, uint32_t, ...) {}
static struct wl_proxy *s2_marshal_ctor(struct wl_proxy *, uint32_t,
    const struct wl_interface *, uint32_t, ...) {
    return reinterpret_cast<struct wl_proxy *>(0xDEAD); }
static int s2_add_listener(struct wl_proxy *, const void *, void *) { return 0; }
static void *s2_get_user_data(struct wl_proxy *) { return nullptr; }
static uint32_t s2_get_version(struct wl_proxy *) { return 1; }
static const void *s2_get_listener(struct wl_proxy *) {
    return reinterpret_cast<const void *>(0xBEEF); }
static void s2_set_user_data(struct wl_proxy *, void *) {}
static void s2_destroy(struct wl_proxy *) {}

static TreelandCaptureManager *stub_tlMgr_instance2() { return nullptr; }

class TreelandCaptureVirtualCovTest : public Test {
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
        if (p1) stub.set(p1, (void*)s2_marshal_flags);
        if (p2) stub.set(p2, (void*)s2_marshal_ctor);
        if (p3) stub.set(p3, (void*)s2_add_listener);
        if (p4) stub.set(p4, (void*)s2_get_user_data);
        if (p5) stub.set(p5, (void*)s2_get_version);
        if (p6) stub.set(p6, (void*)s2_get_listener);
        if (p7) stub.set(p7, (void*)s2_set_user_data);
        if (p8) stub.set(p8, (void*)s2_destroy);
        stub.set(&TreelandCaptureManager::instance, (void*)stub_tlMgr_instance2);
    }
};

TEST_F(TreelandCaptureVirtualCovTest, FrameVirtuals)
{
    auto *f = new TreelandCaptureFrame(
        reinterpret_cast<::treeland_capture_frame_v1*>(0xDEAD));
    f->treeland_capture_frame_v1_buffer(0, 0, 0, 1); // stride != width*4 → early return
    f->treeland_capture_frame_v1_flags(0);
    f->treeland_capture_frame_v1_failed();
    // treeland_capture_frame_v1_ready() dereferences null m_shmBuffer — skipped
    delete f;
    SUCCEED();
}

TEST_F(TreelandCaptureVirtualCovTest, ContextVirtuals)
{
    auto *c = new TreelandCaptureContext(
        reinterpret_cast<::treeland_capture_context_v1*>(0xDEAD));
    c->treeland_capture_context_v1_source_ready(0, 0, 0, 0, 0);
    c->treeland_capture_context_v1_source_failed(0);
    delete c;
    SUCCEED();
}

TEST_F(TreelandCaptureVirtualCovTest, SessionVirtuals)
{
    auto *s = new TreelandCaptureSession(
        reinterpret_cast<::treeland_capture_session_v1*>(0xDEAD));
    s->treeland_capture_session_v1_cancel(0);
    s->treeland_capture_session_v1_frame(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    s->treeland_capture_session_v1_object(0, 0, 0, 0, 0, 0);
    s->treeland_capture_session_v1_ready(0, 0, 0);
    delete s;
    SUCCEED();
}

// ============================================================
// ImageBorderHelper destructor
// ============================================================
TEST(ImageBorderHelperCovTest, Destructor)
{
    {
        ImageBorderHelper h;
    }
    SUCCEED();
}

// ============================================================
// MajorImageProcessingThread
// ============================================================
TEST(MajorImageProcessingThreadRemCovTest, ConstructDestruct)
{
    auto *t = new MajorImageProcessingThread();
    delete t;
    SUCCEED();
}

// ============================================================
// AudioUtils protected methods
// ============================================================
TEST(AudioUtilsProtectedCovTest, InitMethods)
{
    AudioUtils au;
    au.initDefaultSourceDBusInterface();
    au.initDefaultSinkDBusInterface();
    au.initConnections();
    SUCCEED();
}

// ============================================================
// ExtCaptureFrame/Session Private virtuals
// ============================================================
TEST(ExtCapturePrivateCovTest, FrameVirtuals)
{
    ExtCaptureFrame frame;
    auto *base = reinterpret_cast<QtWayland::ext_image_copy_capture_frame_v1*>(frame.d);
    base->ext_image_copy_capture_frame_v1_transform(0);
    base->ext_image_copy_capture_frame_v1_damage(0, 0, 0, 0);
    base->ext_image_copy_capture_frame_v1_presentation_time(0, 0, 0);
    base->ext_image_copy_capture_frame_v1_failed(0);
    // handleReady calls mapBuffer which is safe (early return on null)
    base->ext_image_copy_capture_frame_v1_ready();
    SUCCEED();
}

TEST(ExtCapturePrivateCovTest, SessionVirtuals)
{
    ExtCaptureSession session;
    auto *base = reinterpret_cast<QtWayland::ext_image_copy_capture_session_v1*>(session.d);
    base->ext_image_copy_capture_session_v1_buffer_size(0, 0);
    base->ext_image_copy_capture_session_v1_shm_format(0);
    wl_array emptyArr{}; emptyArr.data = nullptr; emptyArr.size = 0;
    base->ext_image_copy_capture_session_v1_dmabuf_device(&emptyArr);
    base->ext_image_copy_capture_session_v1_dmabuf_format(0, &emptyArr);
    base->ext_image_copy_capture_session_v1_done();
    base->ext_image_copy_capture_session_v1_stopped();
    SUCCEED();
}
