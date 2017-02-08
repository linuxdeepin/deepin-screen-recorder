#include <QApplication>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QRegion>
#include <QIcon>
#include <QObject>
#include <QPainter>
#include <QDebug>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include "window_manager.h"
#include "main_window.h"
#include "single_application.h"

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv);

    if (!app.isRunning()) {

        MainWindow window;

        window.setWindowTitle("Deepin recorder");
        window.setWindowIcon(QIcon("image/deepin-recorder.svg"));
        window.setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        window.setAttribute(Qt::WA_NoSystemBackground, true);
        window.setAttribute(Qt::WA_TranslucentBackground, true);
        window.showFullScreen();
        window.show();

        return app.exec();
    }

    return 0;
}
