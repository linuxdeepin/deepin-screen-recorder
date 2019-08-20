#ifndef CAMERA_PROCESS_H
#define CAMERA_PROCESS_H

#include <QObject>
#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QList>
#include <QDebug>
#include <QImage>

class CameraProcess : public QObject
{
    Q_OBJECT
public:
    explicit CameraProcess(QObject *parent = nullptr);
    ~CameraProcess();

signals:

public slots:
    void ShowTheCapture();
    void SavePicture();
    void displayImage(int,QImage image);

private:
    QCamera *camera;//摄像头
    QCameraViewfinder * viewfinder;//摄像头取景器部件
    QCameraImageCapture * imageCapture;//截图部件
};

#endif // CAMERA_PROCESS_H
