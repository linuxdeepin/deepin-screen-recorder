// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTCAPTUREFRAMEBUFFER_H
#define EXTCAPTUREFRAMEBUFFER_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QTimer>

/**
 * @brief ext-capture帧数据结构 (类似waylandFrame)
 */
struct ExtFrameData {
    int64_t timestamp;      // 时间戳
    int index;              // 索引
    int width;              // 宽度
    int height;             // 高度
    int stride;             // 行字节数
    unsigned char *data;    // 帧数据

    ExtFrameData() : timestamp(0), index(0), width(0), height(0), stride(0), data(nullptr) {}
    
    ExtFrameData(int64_t ts, int idx, int w, int h, int s, unsigned char *d)
        : timestamp(ts), index(idx), width(w), height(h), stride(s), data(d) {}
};

/**
 * @brief ext-capture帧缓冲区管理器
 * 
 * 模拟waylandrecord中的帧缓冲区管理，提供相同的接口
 * 用于桥接ext-capture和现有的视频编码系统
 */
class ExtCaptureFrameBuffer : public QObject
{
    Q_OBJECT

public:
    explicit ExtCaptureFrameBuffer(QObject *parent = nullptr);
    ~ExtCaptureFrameBuffer();

    /**
     * @brief 初始化缓冲区
     * @param width 帧宽度
     * @param height 帧高度
     * @param stride 行字节数
     * @param bufferSize 缓冲区大小
     */
    bool initialize(int width, int height, int stride, int bufferSize = 10);

    /**
     * @brief 添加帧数据
     * @param data 帧数据
     * @param size 数据大小
     * @param width 帧宽度
     * @param height 帧高度
     * @param stride 行字节数
     * @param timestamp 时间戳
     */
    void appendBuffer(const unsigned char *data, size_t size, 
                     int width, int height, int stride, int64_t timestamp);

    /**
     * @brief 获取帧数据 (类似waylandintegration_p.h中的getFrame)
     * @param frame 输出帧数据
     * @return 是否成功获取
     */
    bool getFrame(ExtFrameData &frame);

    /**
     * @brief 检查是否有可用帧
     */
    bool hasFrames() const;

    /**
     * @brief 获取队列中的帧数量
     */
    int frameCount() const;

    /**
     * @brief 清空缓冲区
     */
    void clear();

    /**
     * @brief 设置是否获取帧 (类似waylandintegration_p.h中的bGetFrame)
     */
    void setGetFrame(bool enabled);

    /**
     * @brief 检查是否允许获取帧
     */
    bool isGetFrameEnabled() const;
    
    /**
     * @brief 重置缓冲区状态，为下次录制做准备
     */
    void reset();

signals:
    /**
     * @brief 新帧可用
     */
    void frameAvailable();

    /**
     * @brief 缓冲区状态变化
     */
    void bufferStatusChanged(int frameCount, bool isFull);

private:
    void recycleMemory();
    void cleanupBuffers();

    QQueue<ExtFrameData> m_frameQueue;      // 帧队列
    QList<unsigned char*> m_freeBuffers;    // 空闲缓冲区列表
    mutable QMutex m_mutex;                         // 线程安全锁
    
    int m_frameWidth;                       // 帧宽度
    int m_frameHeight;                      // 帧高度
    int m_frameStride;                      // 行字节数
    int m_bufferSize;                       // 缓冲区大小
    int m_frameIndex;                       // 帧索引计数器
    size_t m_frameDataSize;                 // 单帧数据大小
    bool m_initialized;                     // 是否已初始化
    bool m_getFrameEnabled;                 // 是否允许获取帧
    
    unsigned char *m_workingBuffer;         // 工作缓冲区 (类似m_ffmFrame)
};

#endif // EXTCAPTUREFRAMEBUFFER_H
