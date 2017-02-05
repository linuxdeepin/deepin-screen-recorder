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
#include <QWidget>
#include <QThread>
#include "main_window.h"
#include <X11/extensions/shape.h>
#include <QtX11Extras/QX11Info>

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
    
    drawDragPoint = false;
    
    recordButtonStatus = RECORD_BUTTON_NORMAL;
    
    showWaitCounter = 0;
    recordCounter = 0;
    
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
        
        painter.setClipping(true);
        painter.setClipRegion(QRegion(backgroundRect).subtracted(QRegion(frameRect)));
        painter.drawRect(backgroundRect);
        
        // Draw frame.
        QPen framePen(QColor("#2CA7F8"));
        framePen.setWidth(2);
        painter.setBrush(QBrush());  // clear brush
        painter.setPen(framePen);        
        painter.drawRect(frameRect);
        
        // Draw drag pint.
        if (drawDragPoint) {
            painter.setClipRegion(QRegion(backgroundRect));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setOpacity(1);
            framePen.setWidth(1);
            painter.setBrush(QBrush("#666666"));
            painter.drawEllipse(record_x - DRAG_POINT_RADIUS, record_y - DRAG_POINT_RADIUS, DRAG_POINT_RADIUS * 2, DRAG_POINT_RADIUS * 2);
            painter.drawEllipse(record_x - DRAG_POINT_RADIUS + record_width, record_y - DRAG_POINT_RADIUS, DRAG_POINT_RADIUS * 2, DRAG_POINT_RADIUS * 2);
            painter.drawEllipse(record_x - DRAG_POINT_RADIUS, record_y - DRAG_POINT_RADIUS + record_height, DRAG_POINT_RADIUS * 2, DRAG_POINT_RADIUS * 2);
            painter.drawEllipse(record_x - DRAG_POINT_RADIUS + record_width, record_y - DRAG_POINT_RADIUS + record_height, DRAG_POINT_RADIUS * 2, DRAG_POINT_RADIUS * 2);
            painter.drawEllipse(record_x - DRAG_POINT_RADIUS, record_y - DRAG_POINT_RADIUS + record_height / 2, DRAG_POINT_RADIUS * 2, DRAG_POINT_RADIUS * 2);
            painter.drawEllipse(record_x - DRAG_POINT_RADIUS + record_width, record_y - DRAG_POINT_RADIUS + record_height / 2, DRAG_POINT_RADIUS * 2, DRAG_POINT_RADIUS * 2);
            painter.drawEllipse(record_x - DRAG_POINT_RADIUS + record_width / 2, record_y - DRAG_POINT_RADIUS, DRAG_POINT_RADIUS * 2, DRAG_POINT_RADIUS * 2);
            painter.drawEllipse(record_x - DRAG_POINT_RADIUS + record_width / 2, record_y - DRAG_POINT_RADIUS + record_height, DRAG_POINT_RADIUS * 2, DRAG_POINT_RADIUS * 2);
        }
        
        // Draw record panel.
        if (firstPressButton) {
            if (firstReleaseButton) {
                QString buttonString;
                if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                    painter.setBrush(QBrush("#2CA7F8"));
                    buttonString = "录制";
                } else if (recordButtonStatus == RECORD_BUTTON_WAIT) {
                    painter.setBrush(QBrush("#FF8D00"));
                    buttonString = "准备中";
                } else if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
                    painter.setBrush(QBrush("#FF2100"));
                    buttonString = "停止";
                }
                
                painter.setClipping(false);
                int buttonX, buttonY;
                if (rootWindowRect.height - record_y - record_height > PANEL_HEIGHT) {
                    buttonX = record_x + record_width / 2 - PANEL_WIDTH / 2;
                    buttonY = record_y + record_height;
                } else {
                    buttonX = record_x + record_width / 2 - PANEL_WIDTH / 2;
                    buttonY = record_y + record_height - PANEL_HEIGHT;
                }
                painter.drawRect(QRect(buttonX, buttonY, PANEL_WIDTH, PANEL_HEIGHT));
                
                QFont font = painter.font() ;
                font.setPointSize(14);
                painter.setFont(font);
                painter.setPen(QPen(QColor("#000000")));
                painter.drawText(QPoint(buttonX + 40, buttonY + 24), buttonString);
                
                if (recordCounter > 0) {
                    painter.drawText(QPoint(buttonX + 10, buttonY + 24), QString::number(recordCounter));
                }
                
                // Draw record wait second.
                if (showWaitCounter > 0) {
                    QFont secondFont = painter.font() ;
                    secondFont.setPointSize(30);
                    painter.setFont(secondFont);
                    painter.setPen(QPen(QColor("#ff0000")));
                    painter.drawText(QPoint(record_x + record_width / 2, record_y + record_height / 2), QString::number(showWaitCounter));
                }
            }
        }
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
            
            if (firstReleaseButton) {
                int pressX = mouseEvent->x();
                int pressY = mouseEvent->y();
                
                int buttonX, buttonY;
                
                if (rootWindowRect.height - record_y - record_height > PANEL_HEIGHT) {
                    buttonX = record_x + record_width / 2 - PANEL_WIDTH / 2;
                    buttonY = record_y + record_height;
                } else {
                    buttonX = record_x + record_width / 2 - PANEL_WIDTH / 2;
                    buttonY = record_y + record_height - PANEL_HEIGHT;
                }
                
                if (pressX > buttonX && pressX < buttonX + PANEL_WIDTH && pressY > buttonY && pressY < buttonY + PANEL_HEIGHT) {
                    if (recordButtonStatus == RECORD_BUTTON_NORMAL) {
                        recordButtonStatus = RECORD_BUTTON_WAIT;
                        
                        showWaitCounter = 3;
                        showWaitTimer = new QTimer(this);
                        connect(showWaitTimer, SIGNAL(timeout()), this, SLOT(showWaitSecond()));
                        showWaitTimer->start(500);
                        
                        repaint();
                    } else if (recordButtonStatus == RECORD_BUTTON_RECORDING) {
                        recordTimer->stop();
                        
                        recordProcess.stopRecord();
                        QApplication::quit();
                    }
                }
            }
        }
        
        isPressButton = true;
        isReleaseButton = false;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (!firstReleaseButton) {
            firstReleaseButton = true;
            
            updateCursor(event);
        }
        
        isPressButton = false;
        isReleaseButton = true;
        
        repaint();
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
                
                repaint();
            }
            
            updateCursor(event);
            
            bool drawPoint = getAction(event) != ACTION_MOVE;
            if (drawPoint != drawDragPoint) {
                drawDragPoint = drawPoint;
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
    }
    
    return false;
}

void MainWindow::showWaitSecond() {
    repaint();
    
    if (showWaitCounter <= 0) {
        showWaitTimer->stop();
        
        recordButtonStatus = RECORD_BUTTON_RECORDING;
        repaint();
        
        recordCounter = 0;
        recordTimer = new QTimer(this);
        connect(recordTimer, SIGNAL(timeout()), this, SLOT(showRecordSecond()));
        recordTimer->start(1000);
        
        dropMouseEvent();

        recordProcess.setRecordInfo(record_x, record_y, record_width, record_height);
        recordProcess.start();
    }
    showWaitCounter--;
}

void MainWindow::showRecordSecond() {
    recordCounter++;
    
    repaint();
}

void MainWindow::dropMouseEvent() 
{
    int buttonX, buttonY;
    if (rootWindowRect.height - record_y - record_height > PANEL_HEIGHT) {
        buttonX = record_x + record_width / 2 - PANEL_WIDTH / 2;
        buttonY = record_y + record_height;
    } else {
        buttonX = record_x + record_width / 2 - PANEL_WIDTH / 2;
        buttonY = record_y + record_height - PANEL_HEIGHT;
    }
                
    XRectangle* reponseArea = new XRectangle;
    reponseArea->x = buttonX;
    reponseArea->y = buttonY;
    reponseArea->width = PANEL_WIDTH;
    reponseArea->height = PANEL_HEIGHT;

    XShapeCombineRectangles(QX11Info::display(), winId(), ShapeInput, 0, 0, reponseArea ,1 ,ShapeSet, YXBanded);
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

