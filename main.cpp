#include <QApplication>
#include <QWidget>
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
};
    
MainWindow::MainWindow(QWidget *parent) : QWidget(parent) 
{
}

void MainWindow::paintEvent(QPaintEvent *) 
{
    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.setFont(QFont("Arial", 30));
    painter.drawText(rect(), Qt::AlignCenter, "Qt");
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
    
    ScreenWindowsInfo windowsInfo;
    QList<xcb_window_t> windows = windowsInfo.getWindows();
    
    for (int i = 0; i < windows.length(); i++) {
        qDebug() << windowsInfo.getWindowWorkspace(windows[i]) << windowsInfo.getWindowName(windows[i]);
    }
    
    return app.exec();
}
