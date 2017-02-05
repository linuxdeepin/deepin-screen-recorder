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
#include "screen_windows_info.h"
#include "main_window.h"
    
int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);
    
    MainWindow window;
    
    window.setWindowTitle("Deepin recorder");
    window.setWindowIcon(QIcon("logo.png"));
    window.setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    window.setAttribute(Qt::WA_NoSystemBackground, true);
    window.setAttribute(Qt::WA_TranslucentBackground, true);
    window.showFullScreen();
    window.show();
    
    return app.exec();
}
