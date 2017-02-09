#include <QApplication>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QProcess>
#include <QRegion>
#include <QIcon>
#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QSystemTrayIcon>
#include <QDebug>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include "window_manager.h"
#include "record_process.h"
#include "settings.h"

class MainWindow : public QWidget
{
    Q_OBJECT

    static const int CURSOR_BOUND = 5;
    static const int RECORD_MIN_SIZE = 48;
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
    
    static const int INIT_TOOLTIP_WIDTH = 184;
    static const int INIT_TOOLTIP_HEIGHT = 68;
    
    static const int COUNTDOWN_TOOLTIP_WIDTH = 200;
    static const int COUNTDOWN_TOOLTIP_HEIGHT = 200;
    static const int COUNTDOWN_NUMBER_OFFSET_Y = 30;
    static const int COUNTDOWN_STRING_OFFSET_Y = 90;
    
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
    void updateMouseEventArea();

private:
    QList<WindowRect> windowRects;
    QList<QString> windowNames;

    QTimer* showCountdownTimer;
    QTimer* flashTryIconTimer;

    RecordProcess recordProcess;
    WindowRect rootWindowRect;

    bool drawDragPoint;

    bool firstDrag;
    bool firstMove;
    bool firstPressButton;
    bool firstReleaseButton;
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
    
    int countdownCounter;
    int flashCounter;
    
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
};
