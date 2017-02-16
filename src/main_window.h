/* -*- Mode: Vala; indent-tabs-mode: nil; tab-width: 4 -*-
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

class MainWindow : public QWidget
{
    Q_OBJECT

    static const int CURSOR_BOUND;
    static const int RECORD_MIN_SIZE;
    static const int DRAG_POINT_RADIUS;
    
    static const int RECTANGLE_RAIUDS;

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
    
    static const int INIT_TOOLTIP_PADDING_X;
    static const int INIT_TOOLTIP_PADDING_Y;
    
    static const int COUNTDOWN_TOOLTIP_PADDING_X;
    static const int COUNTDOWN_TOOLTIP_PADDING_Y;
    static const int COUNTDOWN_TOOLTIP_NUMBER_PADDING_Y;
    
    static const int RECORD_AREA_PADDING;
    static const int RECORD_AREA_OFFSET;
    
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
    void showCountdown();
    void flashTrayIcon();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void stopRecord();
    void startCountdown();

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
    void passInputEvent();
    void renderTooltipRect(QPainter &painter, QRectF &rect, qreal opacity);
    void setFontSize(QPainter &painter, int textSize);
    void showRecordButton();
    void hideRecordButton();

private:
    QList<WindowRect> windowRects;
    QList<QString> windowNames;

    QTimer* showCountdownTimer;
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
    
    int showCountdownCounter;
    int flashTrayIconCounter;
    
    QImage resizeHandleBigImg;
    QImage resizeHandleSmallImg;
    
    QImage countdown1Img;
    QImage countdown2Img;
    QImage countdown3Img;
    
    QString selectAreaName;
    
    // Settings settings;
    
    QSystemTrayIcon* trayIcon;
    
    WindowManager* windowManager;
    
    QVBoxLayout* layout;
    RecordButton* recordButton;
    RecordOptionPanel* recordOptionPanel;
    
    // Just use for debug.
    // int repaintCounter;
};
