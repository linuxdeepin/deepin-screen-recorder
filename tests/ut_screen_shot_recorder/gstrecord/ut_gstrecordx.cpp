// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

