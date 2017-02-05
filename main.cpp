#include <QApplication>
#include <QWidget>
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

class MainWindow : public QWidget 
{
  public:
    MainWindow(QWidget *parent = 0);
    
  protected:
    void paintEvent(QPaintEvent *event);
    // void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    
  private:
    QList<WindowRect> windowRects;
    WindowRect rootWindowRect;
    
    bool firstPressButton;
    bool firstReleaseButton;
    int dragStartX;
    int dragStartY;
    
    int dragRecordX;
    int dragRecordY;
    
    bool isPressButton;
    bool isReleaseButton;
    
    int record_x;
    int record_y;
    int record_width;
    int record_height;
};
    
MainWindow::MainWindow(QWidget *parent) : QWidget(parent) 
{
    // Init attributes.
    setMouseTracking(true);   // make MouseMove can response
    installEventFilter(this);  // add event filter
    
    firstPressButton = false;
    firstReleaseButton = false;
    dragStartX = 0;
    dragStartY = 0;
    
    isPressButton = false;
    isReleaseButton = false;
    
    record_x = 0;
    record_y = 0;
    record_width = 0;
    record_height = 0;
    
    
    // Get all windows geometry.
    ScreenWindowsInfo windowsInfo;
    QList<xcb_window_t> windows = windowsInfo.getWindows();
    rootWindowRect = windowsInfo.getRootWindowRect();
    
    for (int i = 0; i < windows.length(); i++) {
        windowRects.append(windowsInfo.getWindowRect(windows[i]));
    }
}

void MainWindow::paintEvent(QPaintEvent *) 
{
    if (record_width > 0 && record_height > 0) {
        QPainter painter(this);
        
        QRect backgroundRect = QRect(rootWindowRect.x, rootWindowRect.y, rootWindowRect.width, rootWindowRect.height);
        QRect frameRect = QRect(record_x, record_y, record_width - 1, record_height - 1);
        
        // Draw background.
        painter.setBrush(QBrush("#000000"));
        painter.setOpacity(0.8);
        
        painter.setClipRegion(QRegion(backgroundRect).subtracted(QRegion(frameRect)));
        painter.drawRect(backgroundRect);
        
        // Draw frame.
        QPen framePen(QColor("#2CA7F8"));
        framePen.setWidth(2);
        painter.setBrush(QBrush());  // clear brush
        painter.setPen(framePen);        
        painter.drawRect(frameRect);
    }
}

bool MainWindow::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        dragStartX = mouseEvent->x();
        dragStartY = mouseEvent->y();
        if (!firstPressButton) {
            firstPressButton = true;
        } else {
            dragRecordX = record_x;
            dragRecordY = record_y;
        }
        
        isPressButton = true;
        isReleaseButton = false;
        
        qDebug() << "press";
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (!firstReleaseButton) {
            firstReleaseButton = true;
        }
        
        isPressButton = false;
        isReleaseButton = true;
        
        qDebug() << "release";
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (firstPressButton) {
            if (!firstReleaseButton) {
                if (isPressButton && !isReleaseButton) {
                    record_x = std::min(dragStartX, mouseEvent->x());
                    record_y = std::min(dragStartY, mouseEvent->y());
                    record_width = std::abs(dragStartX - mouseEvent->x());
                    record_height = std::abs(dragStartY - mouseEvent->y());
        
                    repaint();
                }
            } else if (isPressButton) {
                record_x = std::max(std::min(dragRecordX + mouseEvent->x() - dragStartX, rootWindowRect.width - record_width), 1);
                record_y = std::max(std::min(dragRecordY + mouseEvent->y() - dragStartY, rootWindowRect.height - record_height), 1);
                
                qDebug() << "**********";
                
                repaint();
            }
        } else {
            for (int i = 0; i < windowRects.length(); i++) {
                int wx = windowRects[i].x;
                int wy = windowRects[i].y;
                int ww = windowRects[i].width;
                int wh = windowRects[i].height;
                int ex = mouseEvent->x();
                int ey = mouseEvent->y();
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
        
        qDebug() << "move";
    }
    
    return false;
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
