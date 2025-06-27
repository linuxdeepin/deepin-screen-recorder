// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "writeframethread.h"
#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include <qdebug.h>
#include <qimage.h>
#include "recordadmin.h"
#include "../utils/log.h"

WriteFrameThread::WriteFrameThread(WaylandIntegration::WaylandIntegrationPrivate* context, QObject *parent) :
    QThread(parent),
    m_bWriteFrame(false)
{
    qCDebug(dsrApp) << "WriteFrameThread constructor called";
    m_context = context;
}

//int test = 0;
void WriteFrameThread::run()
{
    qCDebug(dsrApp) << "WriteFrameThread started";
    if(nullptr == m_context) {
        qCWarning(dsrApp) << "WriteFrameThread context is null, thread exiting";
        return;
    }

    m_context->m_recordAdmin->m_cacheMutex.lock();
    WaylandIntegration::WaylandIntegrationPrivate::waylandFrame frame;
    qCDebug(dsrApp) << "Starting frame writing loop";
    while (m_context->isWriteVideo()) {
        if (m_context->getFrame(frame)) {
            qCDebug(dsrApp) << "Received frame, writing video frame";
            m_context->m_recordAdmin->m_pOutputStream->writeVideoFrame(frame);
        } else {
            qCDebug(dsrApp) << "No frame available, continuing loop";
        }
    }
    m_context->m_recordAdmin->m_cacheMutex.unlock();
    qCDebug(dsrApp) << "WriteFrameThread finished";
}

bool WriteFrameThread::bWriteFrame()
{
    QMutexLocker locker(&m_writeFrameMutex);
    qCDebug(dsrApp) << "bWriteFrame called, returning" << m_bWriteFrame;
    return m_bWriteFrame;
}

void WriteFrameThread::setBWriteFrame(bool bWriteFrame)
{
    QMutexLocker locker(&m_writeFrameMutex);
    m_bWriteFrame = bWriteFrame;
    qCDebug(dsrApp) << "setBWriteFrame called with" << bWriteFrame;
}
