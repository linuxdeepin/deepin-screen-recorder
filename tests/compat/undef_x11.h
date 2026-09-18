// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// X11/Xlib.h and X11/X.h leak a large set of lowercase macros (True, False,
// None, Bool, Status, KeyPress, KeyRelease, ButtonPress, FocusIn, ...) that
// collide with Qt6 enum enumerators such as QEvent::KeyPress,
// QCborValue::True, QNetworkCookie::SameSite::None, etc. When a translation
// unit includes both an X11-bearing header (transitively, via src/ headers
// such as event_monitor.h) and Qt headers, the macros corrupt the Qt enums.
//
// Include this header *after* the X11-bearing src/ header and *before* pulling
// in <QTest>/<QNetworkCookie>/etc. to scrub the offending macros.

#ifndef COMPAT_UNDEF_X11_H
#define COMPAT_UNDEF_X11_H

#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif
#ifdef None
#undef None
#endif
#ifdef Bool
#undef Bool
#endif
#ifdef Status
#undef Status
#endif

// X11 event type names that collide with QEvent::Type enumerators.
#ifdef KeyPress
#undef KeyPress
#endif
#ifdef KeyRelease
#undef KeyRelease
#endif
#ifdef ButtonPress
#undef ButtonPress
#endif
#ifdef ButtonRelease
#undef ButtonRelease
#endif
#ifdef MotionNotify
#undef MotionNotify
#endif
#ifdef EnterNotify
#undef EnterNotify
#endif
#ifdef LeaveNotify
#undef LeaveNotify
#endif
#ifdef FocusIn
#undef FocusIn
#endif
#ifdef FocusOut
#undef FocusOut
#endif
#ifdef KeymapNotify
#undef KeymapNotify
#endif
#ifdef Expose
#undef Expose
#endif
#ifdef GraphicsExpose
#undef GraphicsExpose
#endif
#ifdef NoExpose
#undef NoExpose
#endif
#ifdef VisibilityNotify
#undef VisibilityNotify
#endif
#ifdef CreateNotify
#undef CreateNotify
#endif
#ifdef DestroyNotify
#undef DestroyNotify
#endif
#ifdef UnmapNotify
#undef UnmapNotify
#endif
#ifdef MapNotify
#undef MapNotify
#endif
#ifdef MapRequest
#undef MapRequest
#endif
#ifdef ReparentNotify
#undef ReparentNotify
#endif
#ifdef ConfigureNotify
#undef ConfigureNotify
#endif
#ifdef ConfigureRequest
#undef ConfigureRequest
#endif
#ifdef GravityNotify
#undef GravityNotify
#endif
#ifdef ResizeRequest
#undef ResizeRequest
#endif
#ifdef CirculateNotify
#undef CirculateNotify
#endif
#ifdef CirculateRequest
#undef CirculateRequest
#endif
#ifdef PropertyNotify
#undef PropertyNotify
#endif
#ifdef SelectionClear
#undef SelectionClear
#endif
#ifdef SelectionRequest
#undef SelectionRequest
#endif
#ifdef SelectionNotify
#undef SelectionNotify
#endif
#ifdef ColormapNotify
#undef ColormapNotify
#endif
#ifdef ClientMessage
#undef ClientMessage
#endif
#ifdef MappingNotify
#undef MappingNotify
#endif
#ifdef GenericEvent
#undef GenericEvent
#endif

#endif // COMPAT_UNDEF_X11_H
