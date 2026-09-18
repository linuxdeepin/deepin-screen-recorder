// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// KF6 compatibility forward header.
// Source code includes <KF5/KWayland/Client/connection_thread.h>, but KF6 ships
// the header at <KWayland/Client/connection_thread.h>. This shim forwards to
// the real KF6 header so existing includes resolve without touching src/.

#ifndef KF5_COMPAT_CONNECTION_THREAD_H
#define KF5_COMPAT_CONNECTION_THREAD_H
#include <KWayland/Client/connection_thread.h>
#endif
