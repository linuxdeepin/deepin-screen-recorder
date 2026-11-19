// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <functional>

#include <QImage>
#include <QList>
#include <QObject>
#include <QPixmap>
#include <QTimer>
#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class CameraController;

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

    using CameraControllerFactory = std::function<CameraController *(QObject *)>;

    explicit CameraWidget(DWidget *parent = nullptr,
                          CameraControllerFactory factory = {});
    ~CameraWidget() override;

    /**
     * @brief 设置录屏区域范围
     * @param x: 录屏区域起点x坐标
     * @param y: 录屏区域起点y坐标
     * @param width: 录屏区域宽度
     * @param height: 录屏区域高度
     */
    void setRecordRect(int x, int y, int width, int height);

    /**
     * @brief 在指定位置显示摄像头界面
     * @param pos: 指定位置
     */
    void showAt(QPoint pos);
    void setParentGlobalOffset(const QPoint &offset);
    QPoint parentGlobalOffset() const;
    int getRecordX();
    int getRecordY();
    int getRecordWidth();
    int getRecordHeight();

    /**
     * @brief 初始化UI界面
     */
    void initUI();

    /**
     * @brief 启动摄像头预览
     */
    void cameraStart();

    /**
     * @brief 停止摄像头预览
     */
    void cameraStop();

    Position postion();
    QPixmap scaledPixmap(const QPixmap &src, int width, int height);
    // 设置CameraWidget是否可以拖动
    void setCameraWidgetImmovable(bool immovable);

    /**
     * @brief getCameraStatus
     */
    int getCameraStatus();

    void setDevcieName(const QString &devcieName);

signals:
    /**
     * @brief 摄像头启动或运行失败
     */
    void cameraError(const QString &message);

protected:
    void enterEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *e);

    /**
     * @brief 切换到下一个可用摄像头设备
     */
    void restartDevices();

public slots:
    void onReceiveMajorImage(QImage image);
    void onReceiveMajorImage(QPixmap pixmap);

private:
    void startCameraById(const QString &preferredId);
    void handleControllerError(const QString &message);
    void clearPreview();

    int recordX = 0;
    int recordY = 0;
    int recordWidth = 0;
    int recordHeight = 0;
    bool m_move = false;
    QPoint m_startPoint;
    QPoint m_windowTopLeftPoint;
    QPoint m_windowTopRightPoint;
    QPoint m_windowBottomLeftPoint;
    QPoint m_parentGlobalOffset;
    DLabel *m_cameraUI = nullptr;
    QString m_deviceName;
    bool m_Immovable = false; // 窗口固定
    bool m_startRequested = false;
    CameraController *m_controller = nullptr;
};

#endif // CAMERAWIDGET_H
