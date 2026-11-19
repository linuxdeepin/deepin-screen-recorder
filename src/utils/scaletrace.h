// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCALETRACE_H
#define SCALETRACE_H

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <cstdio>

namespace ScaleTrace {
inline bool enabled()
{
    static const bool value = qgetenv("DSR_SCALE_TRACE") == "1";
    return value;
}

inline void write(const char *event, const QString &details)
{
    const QByteArray timestamp = QDateTime::currentDateTime().toString(Qt::ISODateWithMs).toUtf8();
    const QByteArray message = details.toUtf8();
    // Field diagnostics must remain visible when the system filters Qt info/debug messages.
    std::fprintf(stderr, "%s [DSR_SCALE_TRACE] pid=%lld thread=%p event=%s %s\n",
                 timestamp.constData(), static_cast<long long>(QCoreApplication::applicationPid()),
                 QThread::currentThreadId(), event, message.constData());
    std::fflush(stderr);
}
}

#define DSR_SCALE_TRACE(event, details) \
    do { \
        if (ScaleTrace::enabled()) { \
            QString scaleTraceDetails; \
            QDebug(&scaleTraceDetails).nospace() << details; \
            ScaleTrace::write(event, scaleTraceDetails); \
        } \
    } while (false)

#endif
