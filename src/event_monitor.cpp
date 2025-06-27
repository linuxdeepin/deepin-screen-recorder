// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "event_monitor.h"
#include "keydefine.h"
#include "utils/log.h"
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include "utils.h"
#include <QDateTime>
#include "dbus_name.h"

EventMonitor::EventMonitor(QObject *parent) : QThread(parent)
{
    qCDebug(dsrApp) << "EventMonitor constructor called.";
    isPress = false;
    if (Utils::isWaylandMode) {
        qCDebug(dsrApp) << "Wayland mode detected, initializing Wayland event monitor.";
        initWaylandEventMonitor();
    }
    qCDebug(dsrApp) << "EventMonitor constructor finished.";
}

EventMonitor::~EventMonitor()
{
    qCDebug(dsrApp) << "EventMonitor destructor called.";
    releaseRes();
    qCDebug(dsrApp) << "EventMonitor destructor finished.";
}

void EventMonitor::releaseRes()
{
    qCDebug(dsrApp) << "releaseRes method called.";
    if (m_display_datalink && m_display) {
        qInfo() << __FUNCTION__ << __LINE__ << "执行 XRecordDisableContext ...";
        qCDebug(dsrApp) << "XRecordDisableContext executed.";
        XRecordDisableContext(m_display, m_context);

        XFlush(m_display);
        qCDebug(dsrApp) << "X display flushed.";
    }
    qCDebug(dsrApp) << "releaseRes method finished.";
}

void EventMonitor::run()
{
    qCDebug(dsrApp) << "run method called.";
    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr) {
        fprintf(stderr, "unable to open display\n");
        qCDebug(dsrApp) << "Failed to open X display.";
        return;
    }
    qCDebug(dsrApp) << "X display opened.";

    // Receive from ALL clients, including future clients.
    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *range = XRecordAllocRange();
    if (range == nullptr) {
        fprintf(stderr, "unable to allocate XRecordRange\n");
        qCDebug(dsrApp) << "Failed to allocate XRecordRange.";
        return;
    }
    qCDebug(dsrApp) << "XRecordRange allocated.";

    // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events.
    memset(range, 0, sizeof(XRecordRange));
    range->device_events.first = KeyPress;
    range->device_events.last  = MotionNotify;
    qCDebug(dsrApp) << "XRecordRange device events set.";

    // And create the XRECORD context.
    m_context = XRecordCreateContext(m_display, 0, &clients, 1, &range, 1);
    if (m_context == 0) {
        fprintf(stderr, "XRecordCreateContext failed\n");
        qCDebug(dsrApp) << "Failed to create XRECORD context.";
        return;
    }
    XFree(range);
    qCDebug(dsrApp) << "XRECORD context created and range freed.";

    XSync(m_display, True);
    qCDebug(dsrApp) << "X display synchronized.";

    m_display_datalink = XOpenDisplay(nullptr);
    if (m_display_datalink == nullptr) {
        fprintf(stderr, "unable to open second display\n");
        qCDebug(dsrApp) << "Failed to open second X display for datalink.";
        return;
    }
    qCDebug(dsrApp) << "Second X display for datalink opened.";

    if (!XRecordEnableContext(m_display_datalink, m_context,  callback, reinterpret_cast<XPointer>(this))) {
        fprintf(stderr, "XRecordEnableContext() failed\n");
        qCDebug(dsrApp) << "Failed to enable XRecord context.";
        return;
    }
    qInfo() << "XRecordEnableContext() finished.";
    qCDebug(dsrApp) << "XRecord context enabled.";

    qInfo() << __FUNCTION__ << __LINE__ << "执行 XRecordFreeContext ...";
    XRecordFreeContext(m_display, m_context);
    qInfo() << __FUNCTION__ << __LINE__ << "执行 XSync ...";
    XSync(m_display, true);
    XSync(m_display_datalink, true);
    qInfo() << __FUNCTION__ << __LINE__ << "执行 XCloseDisplay m_display_datalink...";
    XCloseDisplay(m_display_datalink);
    qInfo() << __FUNCTION__ << __LINE__ << "执行 XCloseDisplay m_display...";
    XCloseDisplay(m_display);
    m_display_datalink = nullptr;
    m_display = nullptr;
    qCDebug(dsrApp) << "Displays closed and pointers nulled.";
    qCDebug(dsrApp) << "run method finished.";
}

void EventMonitor::callback(XPointer ptr, XRecordInterceptData *data)
{
    qCDebug(dsrApp) << "callback method called.";
    (reinterpret_cast<EventMonitor *>(ptr))->handleEvent(data);
    qCDebug(dsrApp) << "handleEvent called from callback.";
}

void EventMonitor::handleEvent(XRecordInterceptData *data)
{
    qCDebug(dsrApp) << "handleEvent method called.";
    if (data->category == XRecordFromServer) {
        qCDebug(dsrApp) << "Event received from server.";

        xEvent *event = reinterpret_cast<xEvent *>(data->data);
        switch (event->u.u.type) {
        case ButtonPress:
            qCDebug(dsrApp) << "ButtonPress event detected. Detail:" << event->u.u.detail;
            if (event->u.u.detail != WheelUp &&
                    event->u.u.detail != WheelDown &&
                    event->u.u.detail != WheelLeft &&
                    event->u.u.detail != WheelRight) {
                isPress = true;
                //鼠标按压
                emit mousePress(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
                qCDebug(dsrApp) << "Mouse button pressed at X:" << event->u.keyButtonPointer.rootX << ", Y:" << event->u.keyButtonPointer.rootY;
            } else if (event->u.u.detail == WheelUp || event->u.u.detail == WheelDown) {
                qCDebug(dsrApp) << "Mouse wheel scrolled.";
#if (QT_VERSION_MAJOR == 5)
                //qt6中的写法只有较新版本的qt5才适用，5.11.3不支持
                int time = int (QDateTime::currentDateTime().toTime_t());
                //鼠标滚动
                emit mouseScroll(static_cast<int>(event->u.enterLeave.time), event->u.u.detail, event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
                qCDebug(dsrApp) << "Mouse scroll event emitted for Qt5.";
#elif (QT_VERSION_MAJOR == 6)
                int time = int(QDateTime::currentDateTime().toSecsSinceEpoch());
                //鼠标滚动
                emit mouseScroll(time, event->u.u.detail, event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
                qCDebug(dsrApp) << "Mouse scroll event emitted for Qt6.";
#endif
            }
            break;
        case MotionNotify:
            qCDebug(dsrApp) << "MotionNotify event detected.";
            if (isPress) {
                emit mouseDrag(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
                qCDebug(dsrApp) << "Mouse dragged to X:" << event->u.keyButtonPointer.rootX << ", Y:" << event->u.keyButtonPointer.rootY;
            } else {
                //鼠标移动
                emit mouseMove(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
                qCDebug(dsrApp) << "Mouse moved to X:" << event->u.keyButtonPointer.rootX << ", Y:" << event->u.keyButtonPointer.rootY;
            }
            break;
        case ButtonRelease:
            qCDebug(dsrApp) << "ButtonRelease event detected. Detail:" << event->u.u.detail;
            if (event->u.u.detail != WheelUp &&
                    event->u.u.detail != WheelDown &&
                    event->u.u.detail != WheelLeft &&
                    event->u.u.detail != WheelRight) {
                isPress = false;
                emit activateWindow();
                emit mouseRelease(event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
                qCDebug(dsrApp) << "Mouse button released at X:" << event->u.keyButtonPointer.rootX << ", Y:" << event->u.keyButtonPointer.rootY;
            }
            break;
        case KeyPress:
            qCDebug(dsrApp) << "KeyPress event detected. Key:" << (reinterpret_cast<unsigned char *>(data->data))[1];
            // If key is equal to esc, emit pressEsc signal.
//            if ((reinterpret_cast<unsigned char *>(data->data))[1] == KEY_ESCAPE) {
//                emit pressEsc();
//            } else {
//                emit pressKeyButton((reinterpret_cast<unsigned char *>(data->data))[1]);
//            }

            //键盘按键按下
            emit keyboardPress((reinterpret_cast<unsigned char *>(data->data))[1]);
            qCDebug(dsrApp) << "Keyboard press event emitted.";
            break;
        case KeyRelease:
            qCDebug(dsrApp) << "KeyRelease event detected. Key:" << (reinterpret_cast<unsigned char *>(data->data))[1];
            //键盘按键释放
            emit keyboardRelease((reinterpret_cast<unsigned char *>(data->data))[1]);
            qCDebug(dsrApp) << "Keyboard release event emitted.";
            break;
        default:
            qCDebug(dsrApp) << "Unknown event type:" << event->u.u.type;
            break;
        }
    }

    fflush(stdout);
    XRecordFreeData(data);
    qCDebug(dsrApp) << "XRecord data freed.";
    qCDebug(dsrApp) << "handleEvent method finished.";
}

XFixesCursorImage *EventMonitor::getCursorImage()
{
    qCDebug(dsrApp) << "getCursorImage method called.";
    /*
        if(Utils::isWaylandMode) {
            return nullptr;
        }
    */
    Display *x11Display = XOpenDisplay(nullptr);
    if (!x11Display) {
        fprintf(stderr, "unable to open display\n");
        qCDebug(dsrApp) << "Failed to open X11 display for cursor image.";
        return nullptr;
    }
    XFixesCursorImage *cursorImage = XFixesGetCursorImage(x11Display);
    qCDebug(dsrApp) << "Cursor image retrieved. Returning it.";
    return cursorImage;
}

//wayland截屏下获取光标的图片
QImage EventMonitor::getCursorImageWayland()
{
    qCDebug(dsrApp) << "getCursorImageWayland method called.";
    QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                 QStringLiteral("/KWin"),
                                 QStringLiteral("org.kde.KWin"));
    qCDebug(dsrApp) << "KWin DBus interface created.";

    QPixmap res;
    QDBusReply<QByteArray> reply = kwinInterface.call(QStringLiteral("cursorImage"));
    qDebug() << "reply: " << reply.isValid();
    qDebug() << "reply.value().isNull(): " << reply.value().isNull();
    qDebug() << "reply.value().isNull(): " << reply.value().isEmpty();
    qCDebug(dsrApp) << "Cursor image DBus reply received. isValid:" << reply.isValid() << ", isNull:" << reply.value().isNull() << ", isEmpty:" << reply.value().isEmpty();
    res.loadFromData(reply.value());
    if (!res.isNull()) {
        QFile dbusResult(reply.value());
        dbusResult.remove();
        qCDebug(dsrApp) << "Result pixmap is not null, removing DBus result file.";
    }
    qCDebug(dsrApp) << "Returning cursor image as QImage.";
    return res.toImage();

}

void EventMonitor::initWaylandEventMonitor()
{
    qCDebug(dsrApp) << "initWaylandEventMonitor method called.";
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.interface()->isServiceRegistered(INPUT_DEVICES_NAME)) {
        qDebug() << "DBusError" << INPUT_DEVICES_NAME;
        return;
    }
    qCDebug(dsrApp) << "InputDevices DBus service is registered.";

    // 监控全局鼠标信号。
    sessionBus.connect(INPUT_DEVICES_NAME,
                       INPUT_DEVICES_PATH, INPUT_DEVICES_INTERFACE, "ButtonPress",
                       this, SLOT(ButtonPressEvent(int, int, int, QString)));
    qCDebug(dsrApp) << "Connected ButtonPress signal.";

    sessionBus.connect(INPUT_DEVICES_NAME,
                       INPUT_DEVICES_PATH, INPUT_DEVICES_INTERFACE, "ButtonRelease",
                       this, SLOT(ButtonReleaseEvent(int, int, int, QString)));
    qCDebug(dsrApp) << "Connected ButtonRelease signal.";

    sessionBus.connect(INPUT_DEVICES_NAME,
                       INPUT_DEVICES_PATH, INPUT_DEVICES_INTERFACE, "CursorMove",
                       this, SLOT(CursorMoveEvent(int, int, QString)));
    qCDebug(dsrApp) << "Connected CursorMove signal.";
    qCDebug(dsrApp) << "initWaylandEventMonitor method finished.";
}

void EventMonitor::ButtonPressEvent(int type, int x, int y, QString str)
{
    qCDebug(dsrApp) << "ButtonPressEvent method called with type:" << type << ", x:" << x << ", y:" << y;
    Q_UNUSED(str);
    // Button1为鼠标左键，Button3为鼠标右键
    if (type == Button1 || type == Button3) {
        emit mousePress(x, y);
        qCDebug(dsrApp) << "Mouse button (left or right) pressed, emitting mousePress.";
    }
    //Debug()<<"=====ButtonPressEvent=====";
    qCDebug(dsrApp) << "ButtonPressEvent method finished.";
}

void EventMonitor::ButtonReleaseEvent(int type, int x, int y, QString str)
{
    qCDebug(dsrApp) << "ButtonReleaseEvent method called with type:" << type << ", x:" << x << ", y:" << y;
    Q_UNUSED(str);
    // Button1为鼠标左键，Button3为鼠标右键
    if (type == Button1 || type == Button3) {
        emit mouseRelease(x, y);
        qCDebug(dsrApp) << "Mouse button (left or right) released, emitting mouseRelease.";
    }
    //qDebug()<<"=====ButtonReleaseEvent=====";
    qCDebug(dsrApp) << "ButtonReleaseEvent method finished.";
}
//  Wayland下全局鼠标移动事件通过Dbus信号接收
void EventMonitor::CursorMoveEvent(int x, int y, QString str)
{
    qCDebug(dsrApp) << "CursorMoveEvent method called with x:" << x << ", y:" << y;
    Q_UNUSED(str);
    emit mouseMove(x, y);
    qCDebug(dsrApp) << "Mouse moved, emitting mouseMove.";
    qCDebug(dsrApp) << "CursorMoveEvent method finished.";
}
