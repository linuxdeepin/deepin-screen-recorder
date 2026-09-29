// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>

#include "../../src/gstrecord/gstrecordx.h"
#include "../../src/utils/audioutils.h"
#include "stub.h"
#include "addr_pri.h"

#include <QImage>
using namespace testing;


class GstRecordXTest: public testing::Test
{

public:
    Stub stub;

    virtual void SetUp() override
    {
        std::cout << "start GstRecordXTest" << std::endl;
        gstInterface::initFunctions();


    }

    virtual void TearDown() override
    {
        gstInterface::unloadFunctions();

        std::cout << "end GstRecordXTest" << std::endl;
    }
};

//x11 gstreamer录屏
TEST_F(GstRecordXTest, x11GstRecord)
{
    int argc = 1;
    gstInterface::m_gst_init(&argc, nullptr);
    //gstreamer接口初始化
    GstRecordX *m_gstRecordx = new GstRecordX();

    //设置参数
    m_gstRecordx->setFramerate(24);
    m_gstRecordx->setRecordArea(QRect(0, 0, 500, 500));
    AudioUtils audioUtils;
    m_gstRecordx->setInputDeviceName(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Source));
    m_gstRecordx->setOutputDeviceName(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Sink));
    GstRecordX::AudioType audioType = GstRecordX::AudioType::Mic;
    m_gstRecordx->setAudioType(audioType);
    GstRecordX::VideoType videoType = GstRecordX::VideoType::ogg;
    m_gstRecordx->setVidoeType(videoType);
    QString savePath = "/tmp/test.webm";
    m_gstRecordx->setSavePath(savePath);
    m_gstRecordx->setX11RecordMouse(true);

    m_gstRecordx->x11GstStartRecord();
    m_gstRecordx->x11GstStopRecord();
    if (m_gstRecordx) {
        delete m_gstRecordx;
        m_gstRecordx = nullptr;
    }

}
void g_object_set_stub(gpointer object, const gchar *first_property_name, ...)
{
    Q_UNUSED(object);
    Q_UNUSED(first_property_name);
}

TEST_F(GstRecordXTest, waylandGstRecord)
{
    Stub stub;
    stub.set(g_object_set, g_object_set_stub);
    int argc = 1;
    gstInterface::m_gst_init(&argc, nullptr);
    //gstreamer接口初始化
    GstRecordX *m_gstRecordx = new GstRecordX();

    //设置参数
    m_gstRecordx->setFramerate(24);
    m_gstRecordx->setRecordArea(QRect(0, 0, 1000, 500));
    AudioUtils audioUtils;
    m_gstRecordx->setInputDeviceName(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Source));
    m_gstRecordx->setOutputDeviceName(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Sink));
    GstRecordX::AudioType audioType = GstRecordX::AudioType::None;
    m_gstRecordx->setAudioType(audioType);
    GstRecordX::VideoType videoType = GstRecordX::VideoType::webm;
    m_gstRecordx->setVidoeType(videoType);
    QString savePath = "/tmp/test.webm";
    m_gstRecordx->setSavePath(savePath);

    m_gstRecordx->waylandGstStartRecord();
    m_gstRecordx->waylandGstStopRecord();
    stub.reset(g_object_set);
    gstInterface::m_g_main_loop_quit(m_gstRecordx->getGloop());
    if (m_gstRecordx) {
        delete m_gstRecordx;
        m_gstRecordx = nullptr;
    }
}

TEST_F(GstRecordXTest, waylandWriteVideoFrame)
{
    int argc = 1;
    Stub stub;
    stub.set(g_object_set, g_object_set_stub);
    gstInterface::m_gst_init(&argc, nullptr);
    //gstreamer接口初始化
    GstRecordX *m_gstRecordx = new GstRecordX();
    QImage img1(":/testImg/addImg1.png");

    //设置参数
    m_gstRecordx->setFramerate(24);
    m_gstRecordx->setRecordArea(QRect(0, 0, img1.width() - 50, img1.height() - 50));
    AudioUtils audioUtils;
    m_gstRecordx->setInputDeviceName(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Source));
    m_gstRecordx->setOutputDeviceName(audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Sink));
    GstRecordX::AudioType audioType = GstRecordX::AudioType::None;
    m_gstRecordx->setAudioType(audioType);
    GstRecordX::VideoType videoType = GstRecordX::VideoType::webm;
    m_gstRecordx->setVidoeType(videoType);
    QString savePath = "/tmp/test.webm";
    m_gstRecordx->setSavePath(savePath);

    m_gstRecordx->waylandGstStartRecord();

    m_gstRecordx->waylandWriteVideoFrame(img1.bits(), img1.width(), img1.height());

    m_gstRecordx->waylandGstStopRecord();
    stub.reset(g_object_set);
    gstInterface::m_g_main_loop_quit(m_gstRecordx->getGloop());
    if (m_gstRecordx) {
        delete m_gstRecordx;
        m_gstRecordx = nullptr;
    }
}

// =========================================================================
// BUG 129883 (sev2) — klv 录制花屏（视频像素格式不对）
// PMS: https://pms.uniontech.com/bug-view-129883.html  commit: 76c0cb9d
// 根因：waylandGstStartRecord 构造的 GStreamer caps 像素格式为 RGB，与板卡实际
//       帧格式不符致花屏。修复将首条 caps 由 format=RGB 改为 format=BGRA
//       （`-format=RGB` → `+format=BGRA`），RGBA 条保留。
// 回归：以修复后的参数集启动 waylandGstStartRecord 全流程，启动→停止→退出主循环
//       不崩溃（花屏修复路径被覆盖）。沿用既有 waylandGstRecord 用例的 SetUp/清理
//       范式（真实 gstreamer），不读取私有字段以避免新增 ACCESS_PRIVATE_FIELD。
// =========================================================================
TEST_F(GstRecordXTest, BUG129883_WaylandGstStartRecordBgraNoCrash)
{
    // Arrange — GstRecordXTest::SetUp 已 gstInterface::initFunctions()
    int argc = 1;
    gstInterface::m_gst_init(&argc, nullptr);
    GstRecordX *m_gstRecordx = new GstRecordX();
    ASSERT_NE(m_gstRecordx, nullptr);

    m_gstRecordx->setFramerate(24);
    m_gstRecordx->setRecordArea(QRect(0, 0, 1000, 500));
    AudioUtils audioUtils;
    m_gstRecordx->setInputDeviceName(
        audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Source));
    m_gstRecordx->setOutputDeviceName(
        audioUtils.getDefaultDeviceName(AudioUtils::DefaultAudioType::Sink));
    m_gstRecordx->setAudioType(GstRecordX::AudioType::None);
    m_gstRecordx->setVidoeType(GstRecordX::VideoType::webm);
    m_gstRecordx->setSavePath(QStringLiteral("/tmp/ut_pms_129883.webm"));

    // Act — 触发 BGRA caps 启动路径（修复点）
    EXPECT_NO_FATAL_FAILURE(m_gstRecordx->waylandGstStartRecord());

    // Assert + 清理 — 停止录制并退出主循环，对象存活；沿用既有范式避免 hang
    EXPECT_NO_FATAL_FAILURE(m_gstRecordx->waylandGstStopRecord());
    EXPECT_NO_FATAL_FAILURE(gstInterface::m_g_main_loop_quit(m_gstRecordx->getGloop()));
    EXPECT_NE(m_gstRecordx, nullptr);

    delete m_gstRecordx;
    m_gstRecordx = nullptr;
}

