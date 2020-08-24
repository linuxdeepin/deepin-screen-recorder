#ifndef GIFRECORD_H
#define GIFRECORD_H
#include <QThread>
#include "waylandintegration.h"
#include "waylandintegration_p.h"

extern "C"
{
#include "lib/GifH/gif.h"
}

class GifRecord : public QThread
{
    Q_OBJECT
public:
    explicit GifRecord(WaylandIntegration::WaylandIntegrationPrivate* context,QObject *parent = nullptr);
    ~GifRecord();

    void init(int screenWidth,
              int screenHeight,
              int x,
              int y,
              int selectWidth,
              int selectHeight,
              int fps,
              QString filePath);

    void run();
    void stop();
    bool bWriteFrame();
    void setBWriteFrame(bool bWriteFrame);

signals:

public slots:

private:
    WaylandIntegration::WaylandIntegrationPrivate * m_context;
    GifWriter m_gitWrite;
    QString m_filePath;
    int m_x;
    int m_y;
    int m_fps;
    int m_screenWidth;
    int m_screenHeight;
    int m_selectWidth;
    int m_selectHeight;
    int m_delay;
    //是否写视频帧
    bool m_bWriteFrame;
    QMutex m_writeFrameMutex;
//    int m_index;
    //static int test;
};
#endif // GIFRECORD_H
