#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QObject>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QList>
#include <QDebug>
#include <QImage>
#include <QTimer>
#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class CameraWidget : public DWidget
{
    Q_OBJECT
public:
    explicit CameraWidget(DWidget *parent = nullptr);
    ~CameraWidget();
    void setRecordRect(int x, int y, int width, int height);
    void showAt(QPoint pos);
    int getRecordX();
    int getRecordY();
    int getRecordWidth();
    int getRecordHeight();
    void initCamera();
    void cameraStart();
    void cameraStop();
signals:

public slots:
    void captureImage();
    void processCapturedImage(int request_id, const QImage &img);
protected:
    void enterEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *e);
private:
    int recordX;
    int recordY;
    int recordWidth;
    int recordHeight;
    bool m_move;
    QPoint m_startPoint;
    QPoint m_windowTopLeftPoint;
    QPoint m_windowTopRightPoint;
    QPoint m_windowBottomLeftPoint;
    QCamera *camera;//摄像头
    QCameraViewfinder *viewfinder; //摄像头取景器部件
    QCameraImageCapture *imageCapture; //截图部件
    QTimer *timer_image_capture;
    DLabel *m_cameraUI;
};

#endif // CAMERAWIDGET_H
