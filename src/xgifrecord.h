#ifndef XGIFRECORD_H
#define XGIFRECORD_H

#include <QThread>
#include <QList>
#include <QImage>
#include <QMutex>
#include <QScreen>
#include <QGuiApplication>
#include <QtConcurrent>
#include <QPixmap>
#include <QRect>
#include <QDesktopWidget>
#include <QApplication>

extern "C"
{
#include "lib/GifH/gif.h"
}

class XGifRecord : public QThread
{
    Q_OBJECT

public:
    explicit XGifRecord(QRect rect,QString savePath,QObject *parent = nullptr);
    void stop();

protected:
    void run();
    void appendBuffer(QImage img);
    void screenshots();
    bool getFrame(QImage &img);
    bool bWriteFrame();
    void setBWriteFrame(bool bWriteFrame);

private:
    QRect m_recordRect;
    GifWriter m_gifWrite;
    int m_delay;
    QList<QImage> m_imageList;
    int m_bufferSize;
    QMutex m_mutex;
    QMutex m_writeFrameMutex;
    bool m_bWriteFrame;
    QString m_savePath;
};

#endif // XGIFRECORD_H
