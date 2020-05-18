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

#include <gbm.h>

#include <epoxy/egl.h>
#include <epoxy/gl.h>
#include <QMutex>

class MainLoop;
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

namespace WaylandIntegration
{
class WriteFrameToStreamThread;
class WaylandIntegrationPrivate : public WaylandIntegration::WaylandIntegration
{
    Q_OBJECT
public:

    //缓存帧
    typedef struct waylandFrame
    {
        int width;
        int height;
        int stride;
        unsigned char* frame;
    };

    typedef struct {
        uint nodeId;
        QVariantMap map;
    } Stream;
    typedef QList<Stream> Streams;

    WaylandIntegrationPrivate();
    ~WaylandIntegrationPrivate();

    void initDrm();
    void initEGL();
    void initWayland(int &argc, char **argv);

    bool isEGLInitialized() const;

    void bindOutput(int outputName, int outputVersion);
    bool startStreaming(const WaylandOutput &output);
    void stopStreaming();
    QMap<quint32, WaylandOutput> screens();
    QVariant streams();

    bool recordStreamMutexInit(int fps,RecordAudioType type,int cx,int cy,int cw,int ch,QString outfile);
    void steamMutexStopInit();
    bool stopStreamMutex();
    inline bool writeFrameToStream();
     MainLoop * m_recordStreamObjPtr;

     pthread_mutex_t m_mtx_stream = PTHREAD_MUTEX_INITIALIZER;
     pthread_cond_t m_cond_stream = PTHREAD_COND_INITIALIZER;
     int tempFps;
     RecordAudioType tempType;
     int tempCx;
     int tempCy;
     int tempCw;
     int tempCh;
     QString tempFilePath;
     int m_recordTIme;
     bool m_isStreamObjCreat;
     bool m_isStreamInit;
     WriteFrameToStreamThread * m_writeFrameThread;
//     pthread_t m_writeFrameThread;
protected Q_SLOTS:
    void addOutput(quint32 name, quint32 version);
    void removeOutput(quint32 name);
    void processBuffer(const KWayland::Client::RemoteBuffer *rbuf);
    void setupRegistry();

private:
    /**
     * @brief appendBuffer:存视频帧
     * @param frame:视频帧
     * @param width:视频帧宽
     * @param height:视频帧高
     * @param stride:通道数
     */
    void appendBuffer(unsigned char* frame,int width,int height,int stride);
public:
    /**
     * @ 内存由getFrame函数内部申请
     * @ 保存视频帧之后，delete <unsigned char* frame>
     * @brief getFrame:获取帧
     * @return
     */
    bool getFrame(waylandFrame &f);

private:
    //参数列表：程序名称，视频类型，视频宽，视频高，视频x坐标，视频y坐标，视频帧率，视频保存路径，音频类型
    QList<QString> argvList;
    //缓存帧容量
    int m_bufferSize;
    //帧宽
    int m_width;
    //帧高
    int m_height;
    //通道数
    int m_stride;
    bool m_bInit;
    QMutex m_mutex;
    //wayland缓冲区
    QList<unsigned char*> m_waylandList;
    QList<unsigned char*> m_catchList;
    unsigned char* cacheFrame;

    bool m_eglInitialized;
    bool m_streamingEnabled;
    bool m_registryInitialized;

    quint32 m_output;
    QDateTime m_lastFrameTime;
    ScreenCastStream *m_stream;

    QThread *m_thread;

    QMap<quint32, WaylandOutput> m_outputMap;
    QList<KWayland::Client::Output*> m_bindOutputs;

    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    KWayland::Client::Registry *m_registry;
    KWayland::Client::RemoteAccessManager *m_remoteAccessManager;

    qint32 m_drmFd = 0; // for GBM buffer mmap
    gbm_device *m_gbmDevice = nullptr; // for passed GBM buffer retrieval
    pthread_t  m_initRecordStreamThread;
    pthread_t m_stopRecordstreamThread2;
    struct {
        QList<QByteArray> extensions;
        EGLDisplay display = EGL_NO_DISPLAY;
        EGLContext context = EGL_NO_CONTEXT;
        gbm_device* gbm;

        PFNEGLCREATEIMAGEKHRPROC create_image;
        PFNEGLDESTROYIMAGEKHRPROC destroy_image;
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d;
    } m_egl;
};

}

#endif // XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_P_H


