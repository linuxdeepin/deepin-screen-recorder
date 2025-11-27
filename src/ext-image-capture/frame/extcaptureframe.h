// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTCAPTUREFRAME_H
#define EXTCAPTUREFRAME_H

#include <QObject>
#include <QSize>
#include <QRect>
#include <QByteArray>

// 前向声明
struct wl_buffer;
struct CaptureConfig;

/**
 * @brief 帧变换类型
 */
enum class FrameTransform {
    Normal = 0,
    Rotate90 = 1,
    Rotate180 = 2,
    Rotate270 = 3,
    FlippedNormal = 4,
    FlippedRotate90 = 5,
    FlippedRotate180 = 6,
    FlippedRotate270 = 7
};

/**
 * @brief 帧数据结构
 */
struct FrameData {
    void *data = nullptr;          // 原始数据指针
    size_t size = 0;               // 数据大小
    QSize dimensions;              // 帧尺寸
    uint32_t format = 0;           // 像素格式
    uint32_t stride = 0;           // 行字节数
    FrameTransform transform = FrameTransform::Normal;  // 变换
    uint64_t timestamp = 0;        // 时间戳
    QList<QRect> damageRegions;    // 损坏区域
};

/**
 * @brief ext-image-copy-capture帧对象
 * 
 * 管理单帧的捕获、缓冲区和数据访问
 */
class ExtCaptureFrame : public QObject
{
    Q_OBJECT

public:
    enum FrameState {
        Uninitialized,  // 未初始化
        Attached,       // 已附加缓冲区
        Damaged,        // 已设置损坏区域
        Capturing,      // 捕获中
        Ready,          // 数据就绪
        Failed          // 捕获失败
    };

    explicit ExtCaptureFrame(QObject *parent = nullptr);
    ~ExtCaptureFrame();

    /**
     * @brief 初始化帧
     * @param frame 协议帧对象
     * @param config 捕获配置
     * @return 是否成功初始化
     */
    bool initialize(void *frame, const CaptureConfig &config);

    /**
     * @brief 获取帧状态
     */
    FrameState state() const;

    /**
     * @brief 开始捕获
     * @param fullDamage 是否全帧损坏（首次捕获或不跟踪损坏）
     * @return 是否成功开始捕获
     */
    bool capture(bool fullDamage = true);

    /**
     * @brief 获取帧数据
     * @return 帧数据结构，如果帧未就绪返回空数据
     */
    const FrameData& frameData() const;

    /**
     * @brief 映射缓冲区数据
     * @return 映射的数据指针，失败返回nullptr
     */
    void* mapBuffer();

    /**
     * @brief 取消映射缓冲区
     */
    void unmapBuffer();

    /**
     * @brief 复制帧数据
     * @param dest 目标缓冲区
     * @param maxSize 最大复制大小
     * @return 实际复制的字节数
     */
    size_t copyFrameData(void *dest, size_t maxSize) const;

    /**
     * @brief 获取帧大小
     */
    QSize size() const;

    /**
     * @brief 获取像素格式
     */
    uint32_t pixelFormat() const;

    /**
     * @brief 获取时间戳
     */
    uint64_t timestamp() const;

signals:
    /**
     * @brief 帧数据就绪
     */
    void ready();

    /**
     * @brief 帧捕获失败
     */
    void failed(const QString &reason);

protected:
    // 协议事件处理
    void handleTransform(uint32_t transform);
    void handleDamage(int32_t x, int32_t y, int32_t width, int32_t height);
    void handlePresentationTime(uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec);
    void handleReady();
    void handleFailed(uint32_t reason);

private:
    bool createBuffer();
    void setState(FrameState newState);
    QString failureReasonToString(uint32_t reason);

    class Private;
    Private *d;
};

#endif // EXTCAPTUREFRAME_H
