#include "screengrabber.h"

#include "../utils/desktopinfo.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QStandardPaths>

ScreenGrabber::ScreenGrabber(QObject *parent) : QObject(parent)
{

}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio)
{
    Q_UNUSED(devicePixelRatio);
    ok = true;
    if (m_info.waylandDectected()) {
        QPixmap res;
        QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                     QStringLiteral("/Screenshot"),
                                     QStringLiteral("org.kde.kwin.Screenshot"));
        QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
        res = QPixmap(reply.value());
        if (!res.isNull()) {
            QFile dbusResult(reply.value());
            dbusResult.remove();
        } else {
            ok = false;
        }
        return res;
    }
/*
    QRect r(rect.topLeft() * devicePixelRatio, rect.size());

    int t_screenNum = QApplication::desktop()->screenCount();

    if (t_screenNum == 1) {
        QList<QScreen *> screenList = qApp->screens();
        for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
            if ((*it)->geometry().contains(r)) {
                return (*it)->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
            }
        }
    } else {
    */
        QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
        // 在多屏模式下, winId 不是0 
        return t_primaryScreen->grabWindow(QApplication::desktop()->winId(), rect.x(), rect.y(), rect.width(), rect.height());
    //}
    //return QPixmap();
}
