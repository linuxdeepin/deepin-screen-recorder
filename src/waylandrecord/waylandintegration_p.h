/*
 * Copyright © 2018 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_P_H
#define XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_P_H

#include "waylandintegration.h"
#include <QDateTime>
#include <QObject>
#include <QMap>
#include <QImage>
#include <gbm.h>
#include <epoxy/egl.h>
#include <epoxy/gl.h>
#include <QMutex>
#include <EGL/egl.h>

enum audioType {
    //麦克风
    MIC = 2,
    //系统音频
    SYS,
    //麦克风+系统音频
    MIC_SYS,
    //不录音
    NOS
};

enum videoType {
    GIF = 1,
    MP4,
    MKV
};

class RecordAdmin;
class ScreenCastStream;

namespace KWayland {
namespace Client {
class ConnectionThread;
class EventQueue;
class OutputDevice;
class Registry;
class RemoteAccessManager;
class RemoteBuffer;
class Output;
}
}

namespace WaylandIntegration {
class WaylandIntegrationPrivate : public WaylandIntegration::WaylandIntegration
{
    Q_OBJECT

public:
    struct EglStruct {
        EGLDisplay dpy;
        EGLContext ctx;
        EGLConfig conf;
    };
    //缓存帧
    struct waylandFrame {
        //时间戳
        int64_t _time;
        //索引
        int _index;
        int _width;
        int _height;
        int _stride;
        unsigned char *_frame;
    };

    typedef struct {
        uint nodeId;
        QVariantMap map;
    } Stream;
    typedef QList<Stream> Streams;

    WaylandIntegrationPrivate();
    ~WaylandIntegrationPrivate();

    //void initDrm();
    //void initEGL();
    void initWayland(QStringList list);

    bool isEGLInitialized() const;

    void bindOutput(int outputName, int outputVersion);
    //bool startStreaming(const WaylandOutput &output);
    void stopStreaming();
    QMap<quint32, WaylandOutput> screens();

    /**
     * @brief stopVideoRecord:停止获取视频流
     * @return
     */
    bool stopVideoRecord();
    inline bool writeFrameToStream();
    //录屏管理器
    RecordAdmin *m_recordAdmin;
    //是否初始化录屏管理
    bool m_bInitRecordAdmin;
    //WriteFrameThread *m_writeFrameThread;
    //     pthread_t m_writeFrameThread;
protected Q_SLOTS:
    void addOutput(quint32 name, quint32 version);
    void removeOutput(quint32 name);
    /**
     * @brief 通过mmap的方式获取视频画面帧
     * @param rbuf
     */
    void processBuffer(const KWayland::Client::RemoteBuffer *rbuf);
    /**
     * @brief 此接口为了解决x86架构录屏mmap失败及花屏问题
     * @param rbuf
     */
    void processBufferX86(const KWayland::Client::RemoteBuffer *rbuf);

    /**
     * @brief 从wayland客户端获取当前屏幕的截图
     * @param fd
     * @param width
     * @param height
     * @param stride
     * @param format
     * @return
     */
    unsigned char *getImageData(int32_t fd, uint32_t width, uint32_t height, uint32_t stride,
                                uint32_t format);
    /**
        * @brief 从wayland客户端获取当前屏幕的截图
        * @param fd
        * @param width
        * @param height
        * @param stride
        * @param format
        * @return
        */
    QImage getImage(int32_t fd, uint32_t width, uint32_t height, uint32_t stride, uint32_t format);

    /**
     * @brief 安装注册wayland客户服务
     */
    void setupRegistry();

    /**
     * @brief 初始化EGL
     */
    void initEgl();

private:
    /**
     * @brief appendBuffer:存视频帧
     * @param frame:视频帧
     * @param width:视频帧宽
     * @param height:视频帧高
     * @param stride:通道数
     * @param time:时间戳
     */
    void appendBuffer(unsigned char *frame, int width, int height, int stride, int64_t time);
public:
    /**
     * @ 内存由getFrame函数内部申请
     * @ 保存视频帧之后，无需：delete <unsigned char* frame>
     * @brief getFrame:获取帧
     * @param frame:视频帧
     * @return
     */
    bool getFrame(waylandFrame &frame);

    bool isWriteVideo();

    bool bGetFrame();
    void setBGetFrame(bool bGetFrame);

    int m_fps = 0;
private:
    //缓存帧容量
    int m_bufferSize;
    int m_width = 0;
    int m_height = 0;
    //通道数
    int m_stride = 0;
    bool m_bInit;
    QMutex m_mutex;
    //wayland缓冲区
    QList<waylandFrame> m_waylandList;
    //空闲内存
    QList<unsigned char *> m_freeList;
    //ffmpeg视频帧
    unsigned char *m_ffmFrame;
    //起始时间戳
    int64_t frameStartTime = 0;
    //是否获取视频帧
    bool m_bGetFrame;
    QMutex m_bGetFrameMutex;
    static int frameIndex;

    bool m_eglInitialized;
    bool m_streamingEnabled = false;
    bool m_registryInitialized;

    quint32 m_output = 0;
    QDateTime m_lastFrameTime;
    //ScreenCastStream *m_stream;

    QThread *m_thread = nullptr;

    QMap<quint32, WaylandOutput> m_outputMap;
    QList<KWayland::Client::Output *> m_bindOutputs;

    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    KWayland::Client::Registry *m_registry;
    KWayland::Client::RemoteAccessManager *m_remoteAccessManager;

    qint32 m_drmFd = 0; // for GBM buffer mmap
    gbm_device *m_gbmDevice = nullptr; // for passed GBM buffer retrieval
    pthread_t  m_initRecordThread;

    /**
     * @brief 自定义egl的结构体
     */
    struct EglStruct m_eglstruct;

};

}

#endif // XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_P_H


