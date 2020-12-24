#ifndef SCREENGRABBER_H
#define SCREENGRABBER_H

#include "../utils/desktopinfo.h"

#include <QObject>

class ScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject *parent = nullptr);
    QPixmap grabEntireDesktop(bool &ok, const QRect &rect, const qreal devicePixelRatio);

private:
    DesktopInfo m_info;
};

#endif // SCREENGRABBER_H
