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
    static const int CURSOR_BOUND = 5;
    static const int MIN_SIZE = 48;
    
    static const int ACTION_MOVE = 0;
    static const int ACTION_RESIZE_TOP_LEFT = 1;
    static const int ACTION_RESIZE_TOP_RIGHT = 2;
    static const int ACTION_RESIZE_BOTTOM_LEFT = 3;
    static const int ACTION_RESIZE_BOTTOM_RIGHT = 4;
    static const int ACTION_RESIZE_TOP = 5;
    static const int ACTION_RESIZE_BOTTOM = 6;
    static const int ACTION_RESIZE_LEFT = 7;
    static const int ACTION_RESIZE_RIGHT = 8;
    
  public:
    MainWindow(QWidget *parent = 0);
    
  protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    void updateCursor(QEvent *event);
    int getAction(QEvent *event);
    
    void resizeTop(QMouseEvent *event);
    void resizeBottom(QMouseEvent *event);
    void resizeLeft(QMouseEvent *event);
    void resizeRight(QMouseEvent *event);
    
  private:
    QList<WindowRect> windowRects;
    WindowRect rootWindowRect;
    
    bool firstPressButton;
    bool firstReleaseButton;
    int dragStartX;
    int dragStartY;
    
    int dragAction;
    int dragRecordX;
    int dragRecordY;
    int dragRecordWidth;
    int dragRecordHeight;
    
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
    
    QApplication::setOverrideCursor(Qt::CrossCursor);    
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
            dragAction = getAction(event);
            
            dragRecordX = record_x;
            dragRecordY = record_y;
            dragRecordWidth = record_width;
            dragRecordHeight = record_height;
        }
        
        isPressButton = true;
        isReleaseButton = false;
        
        qDebug() << "press";
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (!firstReleaseButton) {
            firstReleaseButton = true;
            
            updateCursor(event);
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
                if (dragAction == ACTION_MOVE) {
                    record_x = std::max(std::min(dragRecordX + mouseEvent->x() - dragStartX, rootWindowRect.width - record_width), 1);
                    record_y = std::max(std::min(dragRecordY + mouseEvent->y() - dragStartY, rootWindowRect.height - record_height), 1);
                } else if (dragAction == ACTION_RESIZE_TOP_LEFT) {
                    resizeTop(mouseEvent);
                    resizeLeft(mouseEvent);
                } else if (dragAction == ACTION_RESIZE_TOP_RIGHT) {
                    resizeTop(mouseEvent);
                    resizeRight(mouseEvent);
                } else if (dragAction == ACTION_RESIZE_BOTTOM_LEFT) {
                    resizeBottom(mouseEvent);
                    resizeLeft(mouseEvent);
                } else if (dragAction == ACTION_RESIZE_BOTTOM_RIGHT) {
                    resizeBottom(mouseEvent);
                    resizeRight(mouseEvent);
                } else if (dragAction == ACTION_RESIZE_TOP) {
                    resizeTop(mouseEvent);
                } else if (dragAction == ACTION_RESIZE_BOTTOM) {
                    resizeBottom(mouseEvent);
                } else if (dragAction == ACTION_RESIZE_LEFT) {
                    resizeLeft(mouseEvent);
                } else if (dragAction == ACTION_RESIZE_RIGHT) {
                    resizeRight(mouseEvent);
                }
                
                qDebug() << "**********";
                
                repaint();
            }
            
            updateCursor(event);
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

void MainWindow::resizeTop(QMouseEvent *mouseEvent) 
{
    int offsetY = mouseEvent->y() - dragStartY;
    record_y = std::max(std::min(dragRecordY + offsetY, dragRecordY + dragRecordHeight - MIN_SIZE), 1);
    record_height = std::max(std::min(dragRecordHeight - offsetY, rootWindowRect.height), MIN_SIZE);
}

void MainWindow::resizeBottom(QMouseEvent *mouseEvent) 
{
    int offsetY = mouseEvent->y() - dragStartY;
    record_height = std::max(std::min(dragRecordHeight + offsetY, rootWindowRect.height), MIN_SIZE);
}

void MainWindow::resizeLeft(QMouseEvent *mouseEvent) 
{
    int offsetX = mouseEvent->x() - dragStartX;
    record_x = std::max(std::min(dragRecordX + offsetX, dragRecordX + dragRecordWidth - MIN_SIZE), 1);
    record_width = std::max(std::min(dragRecordWidth - offsetX, rootWindowRect.width), MIN_SIZE);
}

void MainWindow::resizeRight(QMouseEvent *mouseEvent) 
{
    int offsetX = mouseEvent->x() - dragStartX;
    record_width = std::max(std::min(dragRecordWidth + offsetX, rootWindowRect.width), MIN_SIZE);
}

int MainWindow::getAction(QEvent *event) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    int cursorX = mouseEvent->x();
    int cursorY = mouseEvent->y();
    
    if (cursorX > record_x - CURSOR_BOUND
        && cursorX < record_x + CURSOR_BOUND
        && cursorY > record_y - CURSOR_BOUND
        && cursorY < record_y + CURSOR_BOUND) {
        // Top-Left corner.
        return ACTION_RESIZE_TOP_LEFT;
    } else if (cursorX > record_x + record_width - CURSOR_BOUND
               && cursorX < record_x + record_width + CURSOR_BOUND
               && cursorY > record_y + record_height - CURSOR_BOUND
               && cursorY < record_y + record_height + CURSOR_BOUND) {
        // Bottom-Right corner.
        return ACTION_RESIZE_BOTTOM_RIGHT;
    } else if (cursorX > record_x + record_width - CURSOR_BOUND
               && cursorX < record_x + record_width + CURSOR_BOUND
               && cursorY > record_y - CURSOR_BOUND
               && cursorY < record_y + CURSOR_BOUND) {
        // Top-Right corner.
        return ACTION_RESIZE_TOP_RIGHT;
    } else if (cursorX > record_x - CURSOR_BOUND
               && cursorX < record_x + CURSOR_BOUND
               && cursorY > record_y + record_height - CURSOR_BOUND
               && cursorY < record_y + record_height + CURSOR_BOUND) {
        // Bottom-Left corner.
        return ACTION_RESIZE_BOTTOM_LEFT;
    } else if (cursorX > record_x - CURSOR_BOUND
               && cursorX < record_x + CURSOR_BOUND) {
        // Left.
        return ACTION_RESIZE_LEFT;
    } else if (cursorX > record_x + record_width - CURSOR_BOUND
               && cursorX < record_x + record_width + CURSOR_BOUND) {
        // Right.
        return ACTION_RESIZE_RIGHT;
    } else if (cursorY > record_y - CURSOR_BOUND
               && cursorY < record_y + CURSOR_BOUND) {
        // Top.
        return ACTION_RESIZE_TOP;
    } else if (cursorY > record_y + record_height - CURSOR_BOUND
               && cursorY < record_y + record_height + CURSOR_BOUND) {
        // Bottom.
        return ACTION_RESIZE_BOTTOM;
    } else {
        return ACTION_MOVE;
    }
}

void MainWindow::updateCursor(QEvent *event) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    int cursorX = mouseEvent->x();
    int cursorY = mouseEvent->y();
    
    if (cursorX > record_x - CURSOR_BOUND
        && cursorX < record_x + CURSOR_BOUND
        && cursorY > record_y - CURSOR_BOUND
        && cursorY < record_y + CURSOR_BOUND) {
        // Top-Left corner.
        QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
    } else if (cursorX > record_x + record_width - CURSOR_BOUND
               && cursorX < record_x + record_width + CURSOR_BOUND
               && cursorY > record_y + record_height - CURSOR_BOUND
               && cursorY < record_y + record_height + CURSOR_BOUND) {
        // Bottom-Right corner.
        QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
    } else if (cursorX > record_x + record_width - CURSOR_BOUND
               && cursorX < record_x + record_width + CURSOR_BOUND
               && cursorY > record_y - CURSOR_BOUND
               && cursorY < record_y + CURSOR_BOUND) {
        // Top-Right corner.
        QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
    } else if (cursorX > record_x - CURSOR_BOUND
               && cursorX < record_x + CURSOR_BOUND
               && cursorY > record_y + record_height - CURSOR_BOUND
               && cursorY < record_y + record_height + CURSOR_BOUND) {
        // Bottom-Left corner.
        QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
    } else if (cursorX > record_x - CURSOR_BOUND
               && cursorX < record_x + CURSOR_BOUND) {
        // Left.
        QApplication::setOverrideCursor(Qt::SizeHorCursor);
    } else if (cursorX > record_x + record_width - CURSOR_BOUND
               && cursorX < record_x + record_width + CURSOR_BOUND) {
        // Right.
        QApplication::setOverrideCursor(Qt::SizeHorCursor);
    } else if (cursorY > record_y - CURSOR_BOUND
               && cursorY < record_y + CURSOR_BOUND) {
        // Top.
        QApplication::setOverrideCursor(Qt::SizeVerCursor);
    } else if (cursorY > record_y + record_height - CURSOR_BOUND
               && cursorY < record_y + record_height + CURSOR_BOUND) {
        // Bottom.
        QApplication::setOverrideCursor(Qt::SizeVerCursor);
    } else {
        if (isPressButton) {
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);    
        } else {
            QApplication::setOverrideCursor(Qt::OpenHandCursor);    
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
