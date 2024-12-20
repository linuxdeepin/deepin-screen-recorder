// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-treeland-capture-unstable-v1.h"

#include <private/qwaylandclientextension_p.h>
#include <private/qwaylandshmbackingstore_p.h>

class TreelandCaptureFrame
    : public QObject
    , public QtWayland::treeland_capture_frame_v1
{
    Q_OBJECT
public:
    TreelandCaptureFrame(struct ::treeland_capture_frame_v1 *object, QObject *parent = nullptr);
    ~TreelandCaptureFrame() override;

    inline uint flags() const
    {
        return m_flags;
    }

Q_SIGNALS:
    void ready(QImage image);
    void failed();

protected:
    void treeland_capture_frame_v1_buffer(uint32_t format,
                                          uint32_t width,
                                          uint32_t height,
                                          uint32_t stride) override;
    void treeland_capture_frame_v1_flags(uint32_t flags) override;
    void treeland_capture_frame_v1_ready() override;
    void treeland_capture_frame_v1_failed() override;

private:
    QtWaylandClient::QWaylandShmBuffer *m_shmBuffer{ nullptr };
    QtWaylandClient::QWaylandShmBuffer *m_pendingShmBuffer{ nullptr };
    uint m_flags;
};

struct FrameObject
{
    uint32_t index;
    int32_t fd;
    uint32_t size;
    uint32_t offset;
    uint32_t stride;
    uint32_t planeIndex;
};

union ModifierUnion
{
    struct
    {
        uint32_t modLow;
        uint32_t modHigh;
    };

    uint64_t modifier;
};

class TreelandCaptureSession
    : public QObject
    , public QtWayland::treeland_capture_session_v1
{
    Q_OBJECT
    Q_PROPERTY(bool started READ started NOTIFY startedChanged FINAL)

public:
    explicit TreelandCaptureSession(::treeland_capture_session_v1 *object,
                                    QObject *parent = nullptr);
    ~TreelandCaptureSession() override;

    inline uint bufferWidth() const
    {
        return m_bufferWidth;
    }

    inline uint bufferHeight() const
    {
        return m_bufferHeight;
    }

    inline uint bufferFormat() const
    {
        return m_bufferFormat;
    }

    inline uint bufferFlags() const
    {
        return m_bufferFlags;
    }

    inline const QList<FrameObject> &objects() const
    {
        return m_objects;
    }

    inline ModifierUnion modifierUnion() const
    {
        return m_modifierUnion;
    }

    inline bool started() const
    {
        return m_started;
    }

    void start();
Q_SIGNALS:
    void invalid();
    void ready();
    void startedChanged();

protected:
    void treeland_capture_session_v1_frame(int32_t offset_x,
                                           int32_t offset_y,
                                           uint32_t width,
                                           uint32_t height,
                                           uint32_t buffer_flags,
                                           uint32_t flags,
                                           uint32_t format,
                                           uint32_t mod_high,
                                           uint32_t mod_low,
                                           uint32_t num_objects) override;
    void treeland_capture_session_v1_object(uint32_t index,
                                            int32_t fd,
                                            uint32_t size,
                                            uint32_t offset,
                                            uint32_t stride,
                                            uint32_t plane_index) override;
    void treeland_capture_session_v1_ready(uint32_t tv_sec_hi,
                                           uint32_t tv_sec_lo,
                                           uint32_t tv_nsec) override;
    void treeland_capture_session_v1_cancel(uint32_t reason) override;

private:
    QPoint m_offset;
    uint m_bufferWidth;
    uint m_bufferHeight;
    uint m_bufferFlags;
    uint m_bufferFormat;
    ModifierUnion m_modifierUnion;
    QList<FrameObject> m_objects;
    QtWayland::treeland_capture_session_v1::flags m_flags;
    bool m_started{ false };
    uint32_t m_tvSecHi;
    uint32_t m_tvSecLo;
    uint32_t m_tvUsec;
};

class TreelandCaptureContext
    : public QObject
    , public QtWayland::treeland_capture_context_v1
{
    Q_OBJECT
    Q_PROPERTY(TreelandCaptureFrame* frame READ frame NOTIFY frameChanged FINAL)
    Q_PROPERTY(TreelandCaptureSession* session READ session NOTIFY sessionChanged FINAL)
    Q_PROPERTY(QRectF captureRegion READ captureRegion NOTIFY captureRegionChanged FINAL)

public:

    using QtWayland::treeland_capture_context_v1::source_type;
    explicit TreelandCaptureContext(struct ::treeland_capture_context_v1 *object,
                                    QObject *parent = nullptr);
    ~TreelandCaptureContext() override;

    inline QRectF captureRegion() const
    {
        return m_captureRegion;
    }

    inline QtWayland::treeland_capture_context_v1::source_type sourceType() const
    {
        return m_sourceType;
    }

    inline TreelandCaptureFrame *frame() const
    {
        return m_frame;
    }

    inline TreelandCaptureSession *session() const
    {
        return m_session;
    }

    void selectSource(uint32_t sourceHint, bool freeze, bool withCursor, ::wl_surface *mask);
    TreelandCaptureFrame *ensureFrame();
    TreelandCaptureSession *ensureSession();

Q_SIGNALS:
    void sourceReady(QRect region, uint32_t sourceType);
    void sourceFailed(uint32_t reason);
    void frameChanged();
    void sessionChanged();
    void captureRegionChanged();

protected:
    void treeland_capture_context_v1_source_ready(int32_t region_x,
                                                  int32_t region_y,
                                                  uint32_t region_width,
                                                  uint32_t region_height,
                                                  uint32_t source_type) override;
    void treeland_capture_context_v1_source_failed(uint32_t reason) override;

private:
    QRect m_captureRegion;
    QtWayland::treeland_capture_context_v1::source_type m_sourceType;
    TreelandCaptureFrame *m_frame{ nullptr };
    TreelandCaptureSession *m_session{ nullptr };
};

class TreelandCaptureManager
    : public QWaylandClientExtensionTemplate<TreelandCaptureManager>
    , public QtWayland::treeland_capture_manager_v1
{
    Q_OBJECT
    Q_PROPERTY(TreelandCaptureContext* context READ context NOTIFY contextChanged FINAL)
    Q_PROPERTY(bool record READ record WRITE setRecord NOTIFY recordChanged FINAL)
    Q_PROPERTY(bool recordStarted READ recordStarted NOTIFY recordStartedChanged FINAL)

public:

    void cancelCapture(); //11

    static TreelandCaptureManager *instance();

    ~TreelandCaptureManager() override;

    inline TreelandCaptureContext *context() const
    {
        return m_context;
    }

    TreelandCaptureContext *ensureContext();

    inline bool record() const
    {
        return m_record;
    }

    void setRecord(bool newRecord);
    bool recordStarted() const;

Q_SIGNALS:
    void contextChanged();
    void recordChanged();
    void finishSelect();
    void recordStartedChanged();

private:
    TreelandCaptureManager();

    TreelandCaptureContext *m_context{ nullptr };
    bool m_record{ false };
};
