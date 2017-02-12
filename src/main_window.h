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
#include "window_manager.h"
#include "record_process.h"
#include "settings.h"

class MainWindow : public QWidget
{
    Q_OBJECT

    static const int CURSOR_BOUND = 5;
    static const int RECORD_MIN_SIZE = 200;
    static const int DRAG_POINT_RADIUS = 8;

    static const int RECORD_BUTTON_NORMAL = 0;
    static const int RECORD_BUTTON_WAIT = 1;
    static const int RECORD_BUTTON_RECORDING = 2;

    static const int ACTION_MOVE = 0;
    static const int ACTION_RESIZE_TOP_LEFT = 1;
    static const int ACTION_RESIZE_TOP_RIGHT = 2;
    static const int ACTION_RESIZE_BOTTOM_LEFT = 3;
    static const int ACTION_RESIZE_BOTTOM_RIGHT = 4;
    static const int ACTION_RESIZE_TOP = 5;
    static const int ACTION_RESIZE_BOTTOM = 6;
    static const int ACTION_RESIZE_LEFT = 7;
    static const int ACTION_RESIZE_RIGHT = 8;
    static const int ACTION_STAY = 9;
    
    static const int INIT_TOOLTIP_PADDING_X = 20;
    static const int INIT_TOOLTIP_PADDING_Y = 20;
    
    static const int COUNTDOWN_TOOLTIP_PADDING_X = 20;
    static const int COUNTDOWN_TOOLTIP_PADDING_Y = 20;
    static const int COUNTDOWN_TOOLTIP_NUMBER_PADDING_Y = 30;
    
    static const int RECORD_BUTTON_AREA_WIDTH = 124;
    static const int RECORD_BUTTON_AREA_HEIGHT = 86;
    static const int RECORD_BUTTON_OFFSET_Y = 12;
    
    static const int RECORD_OPTIONS_AREA_HEIGHT = 36;
    static const int RECORD_OPTIONS_AREA_PADDING = 12;
    
    static const int BUTTON_STATE_NORMAL = 0;
    static const int BUTTON_STATE_HOVER = 1;
    static const int BUTTON_STATE_PRESS = 2;
    static const int BUTTON_STATE_CHECKED = 3;
    
    static const int BUTTON_OPTION_HEIGHT = 24;
    static const int BUTTON_OPTION_ICON_OFFSET_X = 14;
    static const int BUTTON_OPTION_STRING_OFFSET_X = 5;

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
    void renderTooltipRect(QPainter &painter, QList<QRectF> &rects, qreal opacity);
    void clearTooltip();
    void setFontSize(QPainter &painter, int textSize);
    QSize getRenderSize(QPainter &painter, QString string);

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
    int recordOptionGifState;
    int recordOptionMp4State;
    
    bool saveAsGif;
    bool moveResizeByKey;
    
    int showCountdownCounter;
    int flashTrayIconCounter;
    
    QImage resizeHandleBigImg;
    QImage resizeHandleSmallImg;
    
    QImage countdown1Img;
    QImage countdown2Img;
    QImage countdown3Img;
    
    QImage recordIconNormalImg;
    QImage recordIconHoverImg;
    QImage recordIconPressImg;
    
    QImage recordGifNormalImg;
    QImage recordGifPressImg;
    QImage recordGifCheckedImg;
    
    QImage recordMp4NormalImg;
    QImage recordMp4PressImg;
    QImage recordMp4CheckedImg;
    
    QString selectAreaName;
    
    Settings settings;
    
    QSystemTrayIcon* trayIcon;
    
    WindowManager* windowManager;
};
