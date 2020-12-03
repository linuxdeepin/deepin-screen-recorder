#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QRect>
#include <QTimer>
#include "../src/record_process.h"
#include "stub.h"


using namespace testing;


int RecordProcess_quit_stub(void* obj)
{
    qDebug() <<"I am QCoreApplication quit";
    return 0;
}

class RecordProcessTest:public testing::Test{

public:

    Stub stub;
    virtual void SetUp() override{
        std::cout << "start RecordProcessTest" << std::endl;

        stub.set(ADDR(QCoreApplication, quit), RecordProcess_quit_stub);
    }

    virtual void TearDown() override{

        std::cout << "end RecordProcessTest" << std::endl;
        stub.reset(ADDR(QCoreApplication, quit));
        stub.reset(ADDR(ConfigSettings, value));
    }
};


QVariant getOption_gif_stub(void* obj, const QString &group, const QString &key)
{
    if(key == "save_as_gif") {
        return true;
    }else if(key == "save_directory"){
        return  "";
    }else if(key == "lossless_recording") {
        return false;
    }
    return true;
}

QVariant getOption_mp4_stub(void* obj, const QString &group, const QString &key)
{
    if(key == "save_as_gif") {
        return false;
    }else if(key == "save_directory"){
        return  "";
    }else if(key == "mkv_framerate"){
        return 24;
    }
    return true;
}

QVariant getOption_mkv_stub(void* obj, const QString &group, const QString &key)
{
    if(key == "save_as_gif") {
        return false;
    }else if(key == "mkv_framerate"){
        return 24;
    }else if(key == "save_directory"){
        return  "";
    }
    return  false;
}

TEST_F(RecordProcessTest, recordGIF)
{
    stub.set(ADDR(ConfigSettings, value), getOption_gif_stub);
    RecordProcess *m_process;
    m_process = new RecordProcess;
    m_process->setRecordAudioInputType(RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO);
    m_process->setRecordInfo(QRect(0, 0, 1920, 1080), "test");
    m_process->startRecord();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    m_process->stopRecord();
    delete  m_process;
}

TEST_F(RecordProcessTest, recordVideoMp4)
{
    stub.set(ADDR(ConfigSettings, value), getOption_mp4_stub);
    RecordProcess *m_process;
    m_process = new RecordProcess;
    m_process->setRecordAudioInputType(RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO);
    m_process->setRecordInfo(QRect(0, 0, 1920, 1080), "test");
    m_process->startRecord();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    m_process->stopRecord();
    delete  m_process;
}

TEST_F(RecordProcessTest, recordVideoMKV)
{
    stub.set(ADDR(ConfigSettings, value), getOption_mkv_stub);
    RecordProcess *m_process;
    m_process = new RecordProcess;
    m_process->setRecordAudioInputType(RecordProcess::RECORD_AUDIO_INPUT_SYSTEMAUDIO);
    m_process->setRecordInfo(QRect(0, 0, 1920, 1080), "test");
    m_process->startRecord();
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();
    m_process->stopRecord();
    delete  m_process;
    sleep(1);
}
