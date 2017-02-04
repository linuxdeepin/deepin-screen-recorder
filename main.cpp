#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include <QIcon>
#include <QObject>
#include <QPainter>
#include <QDebug>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include "screen_windows_info.h"

class MainWindow : public QWidget 
{
  public:
    MainWindow(QWidget *parent = 0);
    
  protected:
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    
  private:
    QList<xcb_get_geometry_reply_t*> windowGeometries;
    int record_x;
    int record_y;
    int record_width;
    int record_height;
};
    
MainWindow::MainWindow(QWidget *parent) : QWidget(parent) 
{
    // Init attributes.
    setMouseTracking(true);
    record_x = 0;
    record_y = 0;
    record_width = 0;
    record_height = 0;
    
    // Get all windows geometry.
    ScreenWindowsInfo windowsInfo;
    QList<xcb_window_t> windows = windowsInfo.getWindows();
    
    for (int i = 0; i < windows.length(); i++) {
        windowGeometries.append(windowsInfo.getWindowGeometry(windows[i]));
    }
}

void MainWindow::paintEvent(QPaintEvent *) 
{
    if (record_width > 0 && record_height > 0) {
        QPainter painter(this);
        QRect rect = QRect(record_x, record_y, record_width, record_height);
        
        QPen penHText(QColor("#2CA7F8"));
        painter.setPen(penHText);        
        painter.drawRect(rect);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    for (int i = 0; i < windowGeometries.length(); i++) {
        int wx = windowGeometries[i]->x;
        int wy = windowGeometries[i]->y;
        int ww = windowGeometries[i]->width;
        int wh = windowGeometries[i]->height;
        int ex = event->x();
        int ey = event->y();
        if (ex > wx && ex < wx + ww && ey > wy && ey < wy + wh) {
            record_x = wx;
            record_y = wy;
            record_width = ww;
            record_height = wh;
            
            repaint();
            
            break;
        }
    }
}

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);
    
    MainWindow window;
    
    window.setWindowTitle("Deepin recorder");
    window.setWindowIcon(QIcon("logo.png"));
    window.setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    window.setAttribute(Qt::WA_NoSystemBackground, true);
    window.setAttribute(Qt::WA_TranslucentBackground, true);
    window.showFullScreen();
    window.show();
    
    return app.exec();
}
