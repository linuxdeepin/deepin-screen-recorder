/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2017 Deepin, Inc.
 *               2011 ~ 2017 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QSystemTrayIcon>
#include <QVBoxLayout>
#include "window_manager.h"
#include "record_process.h"
#include "record_button.h"
#include "record_option_panel.h"
#include "countdown_tooltip.h"
#include "start_tooltip.h"
#include "event_monitor.h"
#include "button_feedback.h"

#undef Bool

class MainWindow : public QWidget
{
    Q_OBJECT

    static const int CURSOR_BOUND;
    static const int RECORD_MIN_SIZE;
    static const int DRAG_POINT_RADIUS;
    
    static const int RECORD_BUTTON_NORMAL;
    static const int RECORD_BUTTON_WAIT;
    static const int RECORD_BUTTON_RECORDING;

    static const int ACTION_MOVE;
    static const int ACTION_RESIZE_TOP_LEFT;
    static const int ACTION_RESIZE_TOP_RIGHT;
    static const int ACTION_RESIZE_BOTTOM_LEFT;
    static const int ACTION_RESIZE_BOTTOM_RIGHT;
    static const int ACTION_RESIZE_TOP;
    static const int ACTION_RESIZE_BOTTOM;
    static const int ACTION_RESIZE_LEFT;
    static const int ACTION_RESIZE_RIGHT;
    
    static const int RECORD_OPTIONAL_PADDING;
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow() {
        // All process will quit if MainWindow destroy.
        // So we don't need delete object by hand.
    }
    
    // Split attributes and resource for speed up start.
    void initAttributes();
    void initResource();

public slots:
    void startRecord();
    void flashTrayIcon();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void stopRecord();
    void startCountdown();
    void showPressFeedback(int x, int y);
    void showDragFeedback(int x, int y);
    void showReleaseFeedback(int x, int y);
    void responseEsc();
    
protected:
    bool eventFilter(QObject *object, QEvent *event);
    int getAction(QEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeBottom(QMouseEvent *event);
    void resizeLeft(QMouseEvent *event);
    void resizeRight(QMouseEvent *event);
    void resizeTop(QMouseEvent *event);
    void updateCursor(QEvent *event);
    void setDragCursor();
    void resetCursor();
    void setFontSize(QPainter &painter, int textSize);
    void showRecordButton();
    void hideRecordButton();
    void adjustLayout(QVBoxLayout *layout, int layoutWidth, int layoutHeight);

private:
    QList<WindowRect> windowRects;
    QList<QString> windowNames;

    QTimer* flashTrayIconTimer;

    RecordProcess recordProcess;
    WindowRect rootWindowRect;

    bool drawDragPoint;

    bool isFirstDrag;
    bool isFirstMove;
    bool isFirstPressButton;
    bool isFirstReleaseButton;
    bool isPressButton;
    bool isReleaseButton;
    
    int dragAction;
    int dragRecordHeight;
    int dragRecordWidth;
    int dragRecordX;
    int dragRecordY;
    int dragStartX;
    int dragStartY;

    int recordButtonStatus;
    int recordHeight;
    int recordWidth;
    int recordX;
    int recordY;
    
    int recordButtonState;
    
    int flashTrayIconCounter;
    
    QImage resizeHandleBigImg;
    QImage resizeHandleSmallImg;
    
    QString selectAreaName;
    
    QSystemTrayIcon* trayIcon;
    
    WindowManager* windowManager;
    
    QVBoxLayout* recordButtonLayout;
    QVBoxLayout* countdownLayout;
    RecordButton* recordButton;
    RecordOptionPanel* recordOptionPanel;
    
    StartTooltip* startTooltip;
    CountdownTooltip* countdownTooltip;
    
    ButtonFeedback* buttonFeedback;
    
    EventMonitor eventMonitor;
    
    // Just use for debug.
    // int repaintCounter;
};
