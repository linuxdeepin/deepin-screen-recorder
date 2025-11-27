// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extcaptureframebuffer.h"

#include <QDebug>
#include <cstring>

ExtCaptureFrameBuffer::ExtCaptureFrameBuffer(QObject *parent)
    : QObject(parent)
    , m_frameWidth(0)
    , m_frameHeight(0)
    , m_frameStride(0)
    , m_bufferSize(10)
    , m_frameIndex(0)
    , m_frameDataSize(0)
    , m_initialized(false)
    , m_getFrameEnabled(true)
    , m_workingBuffer(nullptr)
{
}

ExtCaptureFrameBuffer::~ExtCaptureFrameBuffer()
{
    cleanupBuffers();
}

bool ExtCaptureFrameBuffer::initialize(int width, int height, int stride, int bufferSize)
{
    if (m_initialized) {
        qWarning() << "ExtCaptureFrameBuffer already initialized";
        return false;
    }

    if (width <= 0 || height <= 0 || stride <= 0) {
        qWarning() << "Invalid frame dimensions:" << width << "x" << height << "stride:" << stride;
        return false;
    }

    m_frameWidth = width;
    m_frameHeight = height;
    m_frameStride = stride;
    m_bufferSize = qMax(2, bufferSize);
    m_frameDataSize = static_cast<size_t>(height * stride);
    m_frameIndex = 0;

    // 创建工作缓冲区 (类似waylandintegration_p.h中的m_ffmFrame)
    m_workingBuffer = new unsigned char[m_frameDataSize];
    if (!m_workingBuffer) {
        qWarning() << "Failed to allocate working buffer";
        return false;
    }

    // 创建空闲缓冲区池
    for (int i = 0; i < m_bufferSize; ++i) {
        unsigned char *buffer = new unsigned char[m_frameDataSize];
        if (buffer) {
            m_freeBuffers.append(buffer);
        } else {
            qWarning() << "Failed to allocate buffer" << i;
            cleanupBuffers();
            return false;
        }
    }

    m_initialized = true;
    qDebug() << "ExtCaptureFrameBuffer initialized:"
             << "size:" << width << "x" << height
             << "stride:" << stride
             << "buffers:" << m_bufferSize
             << "frame_size:" << m_frameDataSize;

    return true;
}

void ExtCaptureFrameBuffer::appendBuffer(const unsigned char *data, size_t size, 
                                        int width, int height, int stride, int64_t timestamp)
{
    if (!m_initialized || !data || size == 0) {
        qWarning() << "Invalid append buffer parameters";
        return;
    }

    if (!m_getFrameEnabled) {
        return;
    }

    // 检查帧尺寸是否匹配
    if (width != m_frameWidth || height != m_frameHeight || stride != m_frameStride) {
        qWarning() << "Frame size mismatch:"
                   << "expected:" << m_frameWidth << "x" << m_frameHeight << "stride:" << m_frameStride
                   << "received:" << width << "x" << height << "stride:" << stride;
        return;
    }

    size_t expectedSize = static_cast<size_t>(height * stride);
    if (size != expectedSize) {
        qWarning() << "Frame data size mismatch:"
                   << "expected:" << expectedSize
                   << "received:" << size;
        return;
    }

    QMutexLocker locker(&m_mutex);

    // 如果队列已满，移除最老的帧 (循环缓冲区)
    if (m_frameQueue.size() >= m_bufferSize) {
        ExtFrameData oldFrame = m_frameQueue.dequeue();
        if (oldFrame.data) {
            // 回收到空闲列表
            m_freeBuffers.append(oldFrame.data);
        }
        qDebug() << "Buffer full, recycled frame" << oldFrame.index;
    }

    // 获取空闲缓冲区
    unsigned char *frameBuffer = nullptr;
    if (!m_freeBuffers.isEmpty()) {
        frameBuffer = m_freeBuffers.takeFirst();
    } else {
        qWarning() << "No free buffers available";
        return;
    }

    // 复制帧数据
    std::memcpy(frameBuffer, data, size);

    // 创建帧数据结构
    ExtFrameData frameData(timestamp, m_frameIndex++, width, height, stride, frameBuffer);
    m_frameQueue.enqueue(frameData);

    emit frameAvailable();
    emit bufferStatusChanged(m_frameQueue.size(), m_frameQueue.size() >= m_bufferSize);

    qDebug() << "Frame" << frameData.index << "appended, queue size:" << m_frameQueue.size();
}

bool ExtCaptureFrameBuffer::getFrame(ExtFrameData &frame)
{
    QMutexLocker locker(&m_mutex);

    if (m_frameQueue.isEmpty() || !m_workingBuffer) {
        // 清空输出帧
        frame.width = 0;
        frame.height = 0;
        frame.data = nullptr;
        return false;
    }

    // 获取队首帧
    ExtFrameData queueFrame = m_frameQueue.dequeue();
    
    // 复制到工作缓冲区 (类似waylandintegration_p.h中的逻辑)
    std::memcpy(m_workingBuffer, queueFrame.data, m_frameDataSize);
    
    // 设置输出帧
    frame.timestamp = queueFrame.timestamp;
    frame.index = queueFrame.index;
    frame.width = queueFrame.width;
    frame.height = queueFrame.height;
    frame.stride = queueFrame.stride;
    frame.data = m_workingBuffer;

    // 回收原始缓冲区
    m_freeBuffers.append(queueFrame.data);

    emit bufferStatusChanged(m_frameQueue.size(), false);

    qDebug() << "Frame" << frame.index << "retrieved, remaining:" << m_frameQueue.size();
    return true;
}

bool ExtCaptureFrameBuffer::hasFrames() const
{
    QMutexLocker locker(&m_mutex);
    return !m_frameQueue.isEmpty();
}

int ExtCaptureFrameBuffer::frameCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_frameQueue.size();
}

void ExtCaptureFrameBuffer::clear()
{
    QMutexLocker locker(&m_mutex);
    
    // 回收所有队列中的缓冲区
    while (!m_frameQueue.isEmpty()) {
        ExtFrameData frame = m_frameQueue.dequeue();
        if (frame.data) {
            m_freeBuffers.append(frame.data);
        }
    }

    emit bufferStatusChanged(0, false);
    qDebug() << "Frame buffer cleared";
}

void ExtCaptureFrameBuffer::setGetFrame(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_getFrameEnabled = enabled;
    qDebug() << "GetFrame enabled:" << enabled;
}

bool ExtCaptureFrameBuffer::isGetFrameEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_getFrameEnabled;
}

void ExtCaptureFrameBuffer::recycleMemory()
{
    // 这个方法可以用于主动回收内存，如果需要的话
    QMutexLocker locker(&m_mutex);
    
    // 确保所有缓冲区都在空闲列表中
    qDebug() << "Memory recycling - free buffers:" << m_freeBuffers.size()
             << "queued frames:" << m_frameQueue.size();
}

void ExtCaptureFrameBuffer::cleanupBuffers()
{
    if (m_workingBuffer) {
        delete[] m_workingBuffer;
        m_workingBuffer = nullptr;
    }

    // 清理队列中的缓冲区
    while (!m_frameQueue.isEmpty()) {
        ExtFrameData frame = m_frameQueue.dequeue();
        if (frame.data) {
            delete[] frame.data;
        }
    }

    // 清理空闲缓冲区
    for (unsigned char *buffer : m_freeBuffers) {
        if (buffer) {
            delete[] buffer;
        }
    }
    m_freeBuffers.clear();

    m_initialized = false;
    qDebug() << "ExtCaptureFrameBuffer cleaned up";
}

