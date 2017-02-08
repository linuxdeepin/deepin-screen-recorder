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
#include <QDebug>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include "window_manager.h"
#include "record_process.h"

class MainWindow : public QWidget
{
    Q_OBJECT

    static const int CURSOR_BOUND = 5;
    static const int RECORD_MIN_SIZE = 48;
    static const int DRAG_POINT_RADIUS = 8;

    static const int PANEL_WIDTH = 120;
    static const int PANEL_HEIGHT = 32;

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
    
    static const int TOOLTIP_WIDTH = 184;
    static const int TOOLTIP_HEIGHT = 68;

public:
    MainWindow(QWidget *parent = 0);

public slots:
    void showCountdown();
    void showRecordSecond();

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

    QTimer* recordTimer;
    QTimer* showCountdownTimer;

    RecordProcess recordProcess;
    WindowRect rootWindowRect;

    bool drawDragPoint;

    bool firstMove;
    bool firstPressButton;
    bool firstReleaseButton;
    bool isPressButton;
    bool isReleaseButton;
    
    bool isKeyPress;

    int dragAction;
    int dragRecordHeight;
    int dragRecordWidth;
    int dragRecordX;
    int dragRecordY;
    int dragStartX;
    int dragStartY;

    int recordButtonStatus;
    int recordCounter;
    int recordHeight;
    int recordWidth;
    int recordX;
    int recordY;

    int countdownCounter;
    
    QImage resizeHandleBig;
    QImage resizeHandleSmall;
};
