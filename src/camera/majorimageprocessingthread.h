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

#ifndef MajorImageProcessingThread_H
#define MajorImageProcessingThread_H

#include <QThread>
#include <QPixmap>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>

#ifdef __cplusplus
extern "C" {
#endif
#include "core_io.h"
#include "LPF_V4L2.h"
#include "camview.h"
#include "stdlib.h"
#include "malloc.h"
#include "stdio.h"
#include "math.h"
#include "save_image.h"
#include "colorspaces.h"
#include "render.h"
#ifdef __cplusplus
}
#endif

/**
 * @brief stop 线程处理图片
 */
class MajorImageProcessingThread: public QThread
{
    Q_OBJECT
public:

    enum EncodeEnv {
        FFmpeg_Env = 0,
        QCamera_Env = 1,
        GStreamer_Env = 2
    };
    MajorImageProcessingThread();

    ~MajorImageProcessingThread();


    /**
     * @brief stop 停止线程
     */
    void stop();

    /**
     * @brief init 线程初始化
     */
    void init();

    void setCameraDevice(v4l2_dev_t *camDevice);

    /**
     * @brief getStatus 获取状态
     */
    QAtomicInt getStatus()
    {
        return m_stopped;
    }

protected:
    /**
     * @brief run 运行线程
     */
    void run();

signals:
    /**
     * @brief SendMajorImageProcessing 向预览界面发送帧数据  mips平台、wayland下使用该接口
     * @param image 图像
     * @param result 结果
     */
    void SendMajorImageProcessing(QImage image);

    void isEnd();

private:
    QAtomicInt        m_stopped;
    v4l2_dev_t        *m_videoDevice;
    v4l2_frame_buff_t *m_frame;
};

#endif // MajorImageProcessingThread_H
