/*
* Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* Maintainer: shicetu <shicetu@uniontech.com>
*             hujianbo <hujianbo@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "majorimageprocessingthread.h"


extern "C" {
#include <libimagevisualresult/visualresult.h>
}

#include <QFile>
#include <QDate>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
MajorImageProcessingThread::MajorImageProcessingThread()
{
    init();
}

void MajorImageProcessingThread::stop()
{
    //只有线程还是运行状态时才会退出
    if (m_stopped) {
        qDebug() << "正在停止传递摄像头画面...";
        m_stopped = 0;
        QEventLoop loop;
        connect(this, SIGNAL(isEnd()), &loop, SLOT(quit()));
        loop.exec();
        qDebug() << "已停止采集摄像头画面";
    }
}

void MajorImageProcessingThread::init()
{
    m_stopped = 0;
    m_frame = nullptr;
    m_videoDevice = nullptr;
}

void MajorImageProcessingThread::setCameraDevice(v4l2_dev_t *camDevice)
{
    m_videoDevice = camDevice;
}

void MajorImageProcessingThread::run()
{
    m_stopped = 1;
    v4l2_dev_t *vd =  get_v4l2_device_handler();
    if (vd == nullptr) {
        qWarning() << "启动采集摄像头画面线程失败！摄像头设备不存在！";
        return;
    }
    qDebug() << __FUNCTION__ << __LINE__ << "启动采集摄像头(" << vd->videodevice << ")画面线程...";
    v4l2core_start_stream(vd);
    while (m_stopped) {
        m_frame = v4l2core_get_decoded_frame(vd);
        if (m_frame == nullptr) {
            continue;
        }
        QImage jpgImage;
        QByteArray temp;
        temp.append(reinterpret_cast<const char *>(m_frame->raw_frame), static_cast<int>(m_frame->raw_frame_max_size));
        jpgImage.loadFromData(temp);
        jpgImage = jpgImage.convertToFormat(QImage::Format_RGB888);
        if (jpgImage.isNull()) {
            qWarning() << "未采集到摄像头画面！" ;
        } else {
            //jpgImage.save(QString("/home/wangcong/Desktop/test/test_%1.png").arg(QDateTime::currentMSecsSinceEpoch()));
            emit SendMajorImageProcessing(jpgImage);
            //qInfo() << "已采集摄像头画面，正在传递摄像头画面";
        }
        v4l2core_release_frame(vd, m_frame);
    }
    v4l2core_stop_stream(vd);
    close_v4l2_device_handler();
    emit isEnd();
}

MajorImageProcessingThread::~MajorImageProcessingThread()
{
    stop();
    qDebug() << __FUNCTION__;
}
