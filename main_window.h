#include <QApplication>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QRegion>
#include <QIcon>
#include <QObject>
#include <QPainter>
#include <QWidget>
#include <QDebug>
#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include "screen_windows_info.h"

class MainWindow : public QWidget 
{
    Q_OBJECT
    
    static const int CURSOR_BOUND = 5;
    static const int MIN_SIZE = 48;
    static const int DRAG_POINT_RADIUS = 5;
    
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
    
  public:
    MainWindow(QWidget *parent = 0);
    
  public slots:
    void showWaitSecond();
    void showRecordSecond();
    
  protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    void updateCursor(QEvent *event);
    int getAction(QEvent *event);
    
    void resizeTop(QMouseEvent *event);
    void resizeBottom(QMouseEvent *event);
    void resizeLeft(QMouseEvent *event);
    void resizeRight(QMouseEvent *event);
    void dropMouseEvent();
        
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
    
    int recordButtonStatus;
    
    bool isPressButton;
    bool isReleaseButton;
    
    bool drawDragPoint;
    
    int record_x;
    int record_y;
    int record_width;
    int record_height;
    
    QTimer* showWaitTimer;
    int showWaitCounter;
    
    QTimer* recordTimer;
    int recordCounter;
};
