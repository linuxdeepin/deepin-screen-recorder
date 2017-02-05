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
    QList<WindowRect> windowRects;
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
        windowRects.append(windowsInfo.getWindowRect(windows[i]));
    }
}

void MainWindow::paintEvent(QPaintEvent *) 
{
    if (record_width > 0 && record_height > 0) {
        QPainter painter(this);
        // Width and height increase 1 to draw right side and bottom side in screen's visible area.
        QRect rect = QRect(record_x, record_y, record_width - 1, record_height - 1);
        
        QPen pen(QColor("#2CA7F8"));
        pen.setWidth(2);
        painter.setPen(pen);        
        painter.drawRect(rect);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    for (int i = 0; i < windowRects.length(); i++) {
        int wx = windowRects[i].x;
        int wy = windowRects[i].y;
        int ww = windowRects[i].width;
        int wh = windowRects[i].height;
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
