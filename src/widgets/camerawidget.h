/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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
#include <QFile>

DWIDGET_USE_NAMESPACE

class CameraWidget : public DWidget
{
    Q_OBJECT
public:
    enum Position {
        leftTop,
        leftBottom,
        rightTop,
        rightBottom,
    };
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
    bool cameraStart();
    void cameraStop();
    void cameraResume();
    //bool getScreenResolution();
    Position postion();
    QPixmap scaledPixmap(const QPixmap &src, int width, int height);
    // 设置CameraWidget是否可以拖动
    void setCameraWidgetImmovable(bool immovable);

signals:

public slots:
    void captureImage();
    void processCapturedImage(int request_id, const QImage &img);
    void deleteCapturedImage(int id, const QString &fileName);
    bool setCameraStop(bool status);
    bool getcameraStatus();
    //void cameraStatus();
    void cameraInitError(QCamera::Error error);
protected:
    void enterEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *e);
private:
    //QPixmap round(const QPixmap &img_in, int radius);
private:
    int recordX = 0;
    int recordY = 0;
    int recordWidth = 0;
    int recordHeight = 0;
    bool m_move = false;
    QPoint m_startPoint;
    QPoint m_windowTopLeftPoint;
    QPoint m_windowTopRightPoint;
    QPoint m_windowBottomLeftPoint;
    QCamera *camera = nullptr;//摄像头
    QCameraImageCapture *imageCapture = nullptr; //截图部件
    QTimer *timer_image_capture = nullptr;
    DLabel *m_cameraUI = nullptr;
    QString m_capturePath;
    QString m_deviceName;
    QFile *m_deviceFile = nullptr;

    bool m_wildScreen = false;
    bool m_Immovable = false; // 窗口不固定
    bool m_isInitCamera = false;
};

#endif // CAMERAWIDGET_H
