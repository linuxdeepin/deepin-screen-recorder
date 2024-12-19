// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "capture.h"

#include <private/qguiapplication_p.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandintegration_p.h>
#include <QDebug>
#include <QPointer>

// 获取 Wayland 集成对象，用于与 Wayland 显示服务器交互
inline QtWaylandClient::QWaylandIntegration *waylandIntegration()
{
    return dynamic_cast<QtWaylandClient::QWaylandIntegration *>(QGuiApplicationPrivate::platformIntegration());
    // 返回当前平台集成对象，如果是 Wayland 集成则返回其指针
}

// 获取 Wayland 显示对象
inline QPointer<QtWaylandClient::QWaylandDisplay> waylandDisplay()
{
    return waylandIntegration()->display();
    // 通过 Wayland 集成对象获取 Wayland 显示对象
}

// 单例模式实现，确保全局只有一个 CaptureManager 实例
TreelandCaptureManager *TreelandCaptureManager::instance()
{
    static TreelandCaptureManager manager;
    return &manager;
    // 返回单例实例的指针
}

TreelandCaptureManager::TreelandCaptureManager()
    : QWaylandClientExtensionTemplate<TreelandCaptureManager>(1)
    , QtWayland::treeland_capture_manager_v1()
{
    qInfo() << "TreelandCaptureManager created.";
    // 输出日志，表示 TreelandCaptureManager 已创建
    connect(this, &TreelandCaptureManager::activeChanged, this, [this] {
        if (!isActive() && m_context) {
            delete m_context;
        }
    });
    // 连接 activeChanged 信号，当活跃状态改变时，如果不再活跃且有上下文，则删除上下文
}

TreelandCaptureManager::~TreelandCaptureManager()
{
    if (m_context)
        delete m_context;
    // 删除上下文对象
    destroy();
    // 销毁 Wayland 扩展
}

// 确保上下文存在，如果不存在则创建新的上下文
TreelandCaptureContext *TreelandCaptureManager::ensureContext()
{
    if (m_context)
        return m_context;
    // 如果上下文已存在，直接返回
    auto context = get_context();
    m_context = new TreelandCaptureContext(context, this);
    // 创建新的上下文对象
    // 监听上下文销毁信号，清理相关资源
    connect(m_context, &TreelandCaptureContext::destroyed, this, [this] {
        m_context = nullptr;
        Q_EMIT contextChanged();
    });
    Q_EMIT contextChanged();
    // 发出上下文改变信号
    return m_context;
}

// 处理捕获源准备就绪的信号
void TreelandCaptureContext::treeland_capture_context_v1_source_ready(int32_t region_x,
                                                                      int32_t region_y,
                                                                      uint32_t region_width,
                                                                      uint32_t region_height,
                                                                      uint32_t source_type)
{
    // 更新捕获区域并发送相关信号
    m_captureRegion = QRect(region_x, region_y, region_width, region_height);
    Q_EMIT captureRegionChanged();
    Q_EMIT sourceReady(QRect(region_x, region_y, region_width, region_height), source_type);
}

void TreelandCaptureContext::treeland_capture_context_v1_source_failed(uint32_t reason)
{
    Q_EMIT sourceFailed(reason);
    // 发出捕获源失败信号
}

void TreelandCaptureManager::cancelCapture() {
    destroy(); // 销毁 Wayland 扩展
}

TreelandCaptureFrame *TreelandCaptureContext::ensureFrame()
{
    if (m_frame) {
        return m_frame;
    }
    // 如果帧已存在，直接返回
    auto object = QtWayland::treeland_capture_context_v1::capture();
    m_frame = new TreelandCaptureFrame(object, this);
    // 创建新的帧对象
    connect(m_frame, &TreelandCaptureFrame::destroyed, this, [this] {
        m_frame = nullptr;
        Q_EMIT frameChanged();
    });
    Q_EMIT frameChanged();
    // 发出帧改变信号
    return m_frame;
}

TreelandCaptureContext::TreelandCaptureContext(::treeland_capture_context_v1 *object,
                                               QObject *parent)
    : QObject(parent)
    , QtWayland::treeland_capture_context_v1(object)
{
    // 构造函数，初始化基类
}

TreelandCaptureContext::~TreelandCaptureContext()
{
    if (m_frame)
        delete m_frame;
    // 删除帧对象
    if (m_session)
        delete m_session;
    // 删除会话对象
    destroy();
    // 销毁 Wayland 上下文
}

void TreelandCaptureContext::selectSource(uint32_t sourceHint,
                                          bool freeze,
                                          bool withCursor,
                                          ::wl_surface *mask)
{
    select_source(sourceHint, freeze, withCursor, mask);
    // 调用底层接口选择捕获源
}

TreelandCaptureSession *TreelandCaptureContext::ensureSession()
{
    if (m_session)
        return m_session;
    // 如果会话已存在，直接返回
    auto object = create_session();
    m_session = new TreelandCaptureSession(object, this);
    // 创建新的会话对象
    connect(m_session, &TreelandCaptureSession::destroyed, this, [this] {
        m_session = nullptr;
        Q_EMIT sessionChanged();
    });
    Q_EMIT sessionChanged();
    // 发出会话改变信号
    return m_session;
}

TreelandCaptureFrame::TreelandCaptureFrame(::treeland_capture_frame_v1 *object, QObject *parent)
    : QObject(parent)
    , QtWayland::treeland_capture_frame_v1(object)
    , m_shmBuffer(nullptr)
    , m_pendingShmBuffer(nullptr)
{
    // 构造函数，初始化基类和成员变量
}

TreelandCaptureFrame::~TreelandCaptureFrame()
{
    delete m_shmBuffer;
    // 删除共享内存缓冲区
    delete m_pendingShmBuffer;
    // 删除待处理的共享内存缓冲区
    destroy();
    // 销毁 Wayland 帧
}

// 处理帧缓冲区数据
void TreelandCaptureFrame::treeland_capture_frame_v1_buffer(uint32_t format,
                                                            uint32_t width,
                                                            uint32_t height,
                                                            uint32_t stride)
{
    // 检查 stride 是否符合要求（每像素 4 字节）
    if (stride != width * 4) {
        qDebug() << "Receive a buffer format which is not compatible with QWaylandShmBuffer."
                 << "format:" << format << "width:" << width << "height:" << height
                 << "stride:" << stride;
        return;
    }
    // 如果已有待处理的缓冲区，则跳过
    if (m_pendingShmBuffer)
        return; // We only need one supported format
    m_pendingShmBuffer = new QtWaylandClient::QWaylandShmBuffer(
        waylandDisplay(),
        QSize(width, height),
        QtWaylandClient::QWaylandShm::formatFrom(static_cast<::wl_shm_format>(format)));
    // 创建新的共享内存缓冲区
    copy(m_pendingShmBuffer->buffer());
    // 将数据复制到缓冲区
}

void TreelandCaptureFrame::treeland_capture_frame_v1_flags(uint32_t flags)
{
    m_flags = flags;
    // 设置帧标志
}

void TreelandCaptureFrame::treeland_capture_frame_v1_ready()
{
    if (m_shmBuffer)
        delete m_shmBuffer;
    // 删除当前的共享内存缓冲区
    m_shmBuffer = m_pendingShmBuffer;
    m_pendingShmBuffer = nullptr;
    // 将待处理的缓冲区设为当前缓冲区
    Q_EMIT ready(*m_shmBuffer->image());
    // 发出帧就绪信号
}

void TreelandCaptureFrame::treeland_capture_frame_v1_failed()
{
    Q_EMIT failed();
    // 发出帧失败信号
}

TreelandCaptureSession::TreelandCaptureSession(::treeland_capture_session_v1 *object,
                                               QObject *parent)
    : QObject(parent)
    , QtWayland::treeland_capture_session_v1(object)
{
    // 构造函数，初始化基类
}

TreelandCaptureSession::~TreelandCaptureSession() { }

void TreelandCaptureSession::start()
{
    QtWayland::treeland_capture_session_v1::start();
    // 调用底层接口开始会话
    m_started = true;
    Q_EMIT started();
    // 设置会话已开始，并发出开始信号
}

void TreelandCaptureSession::treeland_capture_session_v1_frame(int32_t offset_x,
                                                               int32_t offset_y,
                                                               uint32_t width,
                                                               uint32_t height,
                                                               uint32_t buffer_flags,
                                                               uint32_t flags,
                                                               uint32_t format,
                                                               uint32_t mod_high,
                                                               uint32_t mod_low,
                                                               uint32_t num_objects)
{
    Q_EMIT invalid();
    // 发出无效信号
    m_objects.clear();
    m_objects.reserve(num_objects);
    // 清空对象列表并预留空间
    m_offset = { offset_x, offset_y };
    // 设置偏移量
    m_bufferWidth = width;
    m_bufferHeight = height;
    // 设置缓冲区宽度和高度
    m_bufferFlags = buffer_flags;
    m_bufferFormat = format;
    // 设置缓冲区标志和格式
    m_flags = static_cast<QtWayland::treeland_capture_session_v1::flags>(flags);
    // 设置会话标志
    m_modifierUnion.modLow = mod_low;
    m_modifierUnion.modHigh = mod_high;
    // 设置缓冲区修饰符
}

void TreelandCaptureSession::treeland_capture_session_v1_object(uint32_t index,
                                                                int32_t fd,
                                                                uint32_t size,
                                                                uint32_t offset,
                                                                uint32_t stride,
                                                                uint32_t plane_index)
{
    m_objects.push_back({ .index = index,
                          .fd = fd,
                          .size = size,
                          .offset = offset,
                          .stride = stride,
                          .planeIndex = plane_index });
    // 添加对象到对象列表
}

void TreelandCaptureSession::treeland_capture_session_v1_ready(uint32_t tv_sec_hi,
                                                               uint32_t tv_sec_lo,
                                                               uint32_t tv_nsec)
{
    m_tvSecHi = tv_sec_hi;
    m_tvSecLo = tv_sec_lo;
    m_tvUsec = tv_nsec;
    // 设置时间戳
    Q_EMIT ready();
    // 发出会话就绪信号
}

void TreelandCaptureSession::treeland_capture_session_v1_cancel(uint32_t reason) { }

void TreelandCaptureManager::setRecord(bool newRecord)
{
    if (m_record == newRecord)
        return;
    // 如果新值与当前值相同，直接返回
    m_record = newRecord;
    qInfo() << "Set record to" << m_record;
    // 设置记录状态并输出日志
    emit recordChanged();
    // 发出记录状态改变信号
}

bool TreelandCaptureManager::recordStarted() const
{
    if (m_context && m_context->session()) {
        return m_context->session()->started();
    } else {
        return false;
    }
    // 检查会话是否已开始
}
