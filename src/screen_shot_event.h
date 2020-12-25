#ifndef SCREENSHOTEVENT_H
#define SCREENSHOTEVENT_H

#include <QThread>
#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <X11/extensions/Xfixes.h>

class ScreenShotEvent : public QThread
{
    Q_OBJECT

public:
    explicit ScreenShotEvent(QObject *parent = nullptr);

    static void callback(XPointer trash, XRecordInterceptData *data);
    void handleRecordEvent(XRecordInterceptData *);

signals:
    void activateWindow();
    void shotKeyPressEvent(const unsigned char &keyCode);

protected:
    void run();
};

#endif // SCREENSHOTEVENT_H
