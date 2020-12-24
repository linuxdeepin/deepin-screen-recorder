/*
 * XGifRecord
 * 描述：x11平台gif录屏模块
 * 时间：2020/10/23
 */

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

//gif库
extern "C"
{
#include "lib/GifH/gif.h"
}

class XGifRecord : public QThread
{
    Q_OBJECT

public:
    explicit XGifRecord(QRect rect,QString savePath,QObject *parent = nullptr);

    /**
     * @brief stop:停止录屏
     */
    void stop();

protected:
    void run();

    /**
     * @brief appendBuffer:存视频帧
     * @param QImage:视频帧
     */
    void appendBuffer(QImage img);

    /**
     * @brief screenshots:截屏
     */
    void screenshots();

    /**
     * @brief getFrame:获取视频帧
     * @param img:获取的视频帧
     * @return:是否获取成功
     */
    bool getFrame(QImage &img);

    /**
     * @brief bWriteFrame:是否写视频帧
     * @return
     */
    bool bWriteFrame();

    /**
     * @brief setBWriteFrame:设置是否写视频帧
     * @param bWriteFrame
     */
    void setBWriteFrame(bool bWriteFrame);

private:
    //录屏区域
    QRect m_recordRect;
    //cppcheck误报：未在构造函数中初始化
    GifWriter m_gifWrite;
    int m_delay;
    //视频帧缓存
    QList<QImage> m_imageList;
    //缓存区大小
    int m_bufferSize;
    //缓存区数据锁
    QMutex m_mutex;
    //是否写帧状态锁
    QMutex m_writeFrameMutex;
    //是否写帧
    bool m_bWriteFrame;
    //录屏文件保存路径
    QString m_savePath;
    //屏幕放大倍数
    qreal m_pixelRatio;
};

#endif // XGIFRECORD_H
