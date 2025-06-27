// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "camerawidget.h"
#include "v4l2_core.h"
#include "../utils/log.h"

#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QPainter>
#include <QHBoxLayout>
#include <QDir>
#include <QBitmap>
#include <QtConcurrent>

#include "../camera/LPF_V4L2.h"
#include "../camera/majorimageprocessingthread.h"

CameraWidget::CameraWidget(DWidget *parent) : DWidget(parent)
{
    qCDebug(dsrApp) << "CameraWidget constructor entered";
}

CameraWidget::~CameraWidget()
{
    qCDebug(dsrApp) << "CameraWidget destructor entered";
    if (m_imgPrcThread) {
        qCDebug(dsrApp) << "Stopping image processing thread";
        m_imgPrcThread->stop();
        m_imgPrcThread->deleteLater();
        m_imgPrcThread = nullptr;
    }
    qCDebug(dsrApp) << "~CameraWidget() finished";
}


void CameraWidget::setRecordRect(int x, int y, int width, int height)
{
    qCDebug(dsrApp) << "setRecordRect called with x:" << x << ", y:" << y << ", width:" << width << ", height:" << height;
    this->recordX = x;
    this->recordY = y;
    this->recordWidth = width;
    this->recordHeight = height;
}

void CameraWidget::showAt(QPoint pos)
{
    qCDebug(dsrApp) << "showAt called with position:" << pos;
    if (!isVisible())
        this->show();
    move(pos.x(), pos.y());
}

int CameraWidget::getRecordX()
{
    qCDebug(dsrApp) << "getRecordX called, returning:" << recordX;
    return recordX;
}
int CameraWidget::getRecordY()
{
    qCDebug(dsrApp) << "getRecordY called, returning:" << recordY;
    return recordY;
}
int CameraWidget::getRecordWidth()
{
    qCDebug(dsrApp) << "getRecordWidth called, returning:" << recordWidth;
    return recordWidth;
}
int CameraWidget::getRecordHeight()
{
    qCDebug(dsrApp) << "getRecordHeight called, returning:" << recordHeight;
    return recordHeight;
}

void CameraWidget::initUI()
{
    qCDebug(dsrApp) << "Initializing camera UI...";
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);
    m_cameraUI = new DLabel(this);

    QHBoxLayout *t_hlayout = new QHBoxLayout(this);
    t_hlayout->addWidget(m_cameraUI);
    this->setLayout(t_hlayout);

//    qDebug() << "第一次打开摄像头界面通过QCamera获取摄像头名称:" << QCameraInfo::defaultCamera().isNull();
//    //第一次打开摄像头界面通过QCamera获取摄像头名称
//    while (QCameraInfo::defaultCamera().isNull()) {
//        qDebug() << "正在通过QCamera获取摄像头名称:" << QCameraInfo::defaultCamera().isNull();
//        QEventLoop loop;
//        QTimer::singleShot(200, &loop, SLOT(quit()));
//        loop.exec();
//    }
//    m_deviceName = QCameraInfo::defaultCamera().deviceName();
    qCDebug(dsrApp) << "Camera UI initialization completed";

}

void CameraWidget::cameraStart()
{
    qCDebug(dsrApp) << "cameraStart called";
    if (!m_isInitCamera) {
        qCDebug(dsrApp) << "First time initializing camera...";
        if (m_imgPrcThread == nullptr) {
            qCDebug(dsrApp) << "Creating MajorImageProcessingThread";
            m_imgPrcThread = new MajorImageProcessingThread;
            m_imgPrcThread->setParent(this);
            m_imgPrcThread->setObjectName("MajorThread");
            connect(m_imgPrcThread, SIGNAL(SendMajorImageProcessing(QPixmap)),
                    this, SLOT(onReceiveMajorImage(QPixmap)));
        }
        m_isInitCamera = true;
        startCameraV4l2(m_deviceName.toLatin1());
        qCDebug(dsrApp) << "First time camera initialization completed!";
    } else {
        qCDebug(dsrApp) << "Re-initializing camera...";
        restartDevices();
        qCDebug(dsrApp) << "Re-initialization of camera completed!";
    }
}
int CameraWidget::startCameraV4l2(const char *device)
{
    qCDebug(dsrApp) << "Initializing camera (" << device << ")...";
    int ret = camInit(device);
    v4l2_dev_t *vd =  get_v4l2_device_handler();

    if (vd == nullptr) {
        qCWarning(dsrApp) << "camInit failed! not found camera device!";
        return -1;
    }
    qCDebug(dsrApp) << "videoDevice->videodevice: " << vd->videodevice;
    if (ret == E_OK) {
        //m_imgPrcThread->setCameraDevice(vd);
        qCDebug(dsrApp) << "Starting image processing thread";
        m_imgPrcThread->start();
    } else {
        qCWarning(dsrApp) << "camInit failed";
        camUnInit();
    }
    qCDebug(dsrApp) << "Camera initialized (" << device << ")";
    return ret;
}

void CameraWidget::restartDevices()
{
    qCDebug(dsrApp) << "restartDevices called";
    //获取当前的摄像头设备
    v4l2_dev_t *devicehandler =  get_v4l2_device_handler();

    //停止采集摄像头画面的线程
    if (m_imgPrcThread != nullptr && m_imgPrcThread->isRunning()) {
        qCDebug(dsrApp) << "Stopping image processing thread for restart";
        m_imgPrcThread->stop();
    }

    while (m_imgPrcThread->isRunning());
    QString str;

    if (devicehandler != nullptr) {
        qCDebug(dsrApp) << "Closing V4L2 device handler";
        str = QString(devicehandler->videodevice);
        close_v4l2_device_handler();
    }

    v4l2_device_list_t *devlist = get_device_list();
    qCDebug(dsrApp) << "Number of devices found:" << devlist->num_devices;
    if (devlist->num_devices == 2) {
        qCDebug(dsrApp) << "Handling two devices";
        for (int i = 0 ; i < devlist->num_devices; i++) {
            QString str1 = QString(devlist->list_devices[i].device);
            if (str != str1) {
                qCDebug(dsrApp) << "Attempting to start camera for device:" << str1;
                if (E_OK == startCameraV4l2(devlist->list_devices[i].device)) {
                    qCDebug(dsrApp) << "Camera started successfully for device:" << str1;
                    break;
                }
            }
        }
    } else {
        qCDebug(dsrApp) << "Handling multiple devices";
        for (int i = 0 ; i < devlist->num_devices; i++) {
            QString str1 = QString(devlist->list_devices[i].device);
            if (str == str1) {
                qCDebug(dsrApp) << "Current device found in list:" << str1;
                if (i == devlist->num_devices - 1) {
                    qCDebug(dsrApp) << "Current device is last, starting first device";
                    startCameraV4l2(devlist->list_devices[0].device);
                } else {
                    qCDebug(dsrApp) << "Starting next device in list";
                    startCameraV4l2(devlist->list_devices[i + 1].device);
                }
                break;
            }
            if (str.isEmpty()) {
                qCDebug(dsrApp) << "Current device string is empty, starting first device";
                startCameraV4l2(devlist->list_devices[0].device);
                break;
            }
        }
    }
}

void CameraWidget::cameraStop()
{
    qCDebug(dsrApp) << "cameraStop called";
    m_cameraUI->clear();

    if (m_imgPrcThread != nullptr) {
        qCDebug(dsrApp) << "Stopping image processing thread";
        m_imgPrcThread->stop();
    }
    camUnInit();
}

void CameraWidget::onReceiveMajorImage(QImage image)
{
    qCDebug(dsrApp) << "onReceiveMajorImage (QImage) called";
    QImage t_image = image.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    QPixmap pixmap = QPixmap::fromImage(image);
    if (m_cameraUI) {
        qCDebug(dsrApp) << "Setting pixmap to camera UI";
        m_cameraUI->setPixmap(pixmap);
    }
}

void CameraWidget::onReceiveMajorImage(QPixmap pixmap)
{
    qCDebug(dsrApp) << "onReceiveMajorImage (QPixmap) called";
    pixmap =  pixmap.scaled(this->width(), this->height());
    if (m_cameraUI) {
        qCDebug(dsrApp) << "Setting pixmap to camera UI";
        m_cameraUI->setPixmap(pixmap);
    }
}

void CameraWidget::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qCDebug(dsrApp) << "CameraWidget enterEvent";
}

void CameraWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    qCDebug(dsrApp) << "CameraWidget paintEvent";
//    QPainter painter(this);
//    painter.setBrush(QColor(0, 0, 0, 255));
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.drawRect(rect());

    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);

    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawRoundedRect(12, 12, width() - 25, height() - 25, 8, 8);
    setMask(bmp);

//    DWidget::paintEvent(e);
}
void CameraWidget::mousePressEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "CameraWidget mousePressEvent";
    //当鼠标左键点击时.
    if (event->button() == Qt::LeftButton) {
        qCDebug(dsrApp) << "Left mouse button pressed, initiating move";
        m_move = true;
        //记录鼠标的世界坐标.
        m_startPoint = event->globalPos();
        //记录窗体的世界坐标.
        m_windowTopLeftPoint = this->frameGeometry().topLeft();
        m_windowTopRightPoint = this->frameGeometry().topRight();
        m_windowBottomLeftPoint = this->frameGeometry().bottomLeft();
    }
}

void CameraWidget::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "CameraWidget mouseMoveEvent";
    if ((event->buttons() & Qt::LeftButton) && m_Immovable == false) {
        qCDebug(dsrApp) << "Mouse moving with left button pressed and widget is movable";
//        //移动中的鼠标位置相对于初始位置的相对位置.
//        QPoint relativePos = event->globalPos() - m_startPoint;
//        QRect recordRect = QRect(recordX, recordY, recordWidth, recordHeight);
//        QPoint removePos = m_windowTopLeftPoint + relativePos;
//        QPoint removeTopRightPos = m_windowTopRightPoint + relativePos;
//        QPoint removeBottomLeftPos = m_windowBottomLeftPoint + relativePos;
//        //然后移动窗体即可.
//        if (recordRect.contains(removePos) &&
//                recordRect.contains(removeTopRightPos) &&
//                recordRect.contains(removeBottomLeftPos) ) {
//            this->move(removePos);
//        }

//        else {

//        }
        QSize size = this->size();
        QSize parentSize(recordWidth, recordHeight);
        QPoint curPos = QPoint(event->globalX() - size.width() / 2,
                               event->globalY() - size.height() / 2);
        if (curPos.x() < recordX) { //left
            qCDebug(dsrApp) << "Adjusting position: left bound reached";
            curPos.setX(recordX);
        }

        if (curPos.y() < recordY) { //top
            qCDebug(dsrApp) << "Adjusting position: top bound reached";
            curPos.setY(recordY);
        }

        if ((curPos.x() + size.width()) > (recordX + parentSize.width()))  { //right
            qCDebug(dsrApp) << "Adjusting position: right bound reached";
            curPos.setX(recordX + parentSize.width() - size.width());
        }

        if ((curPos.y() + size.height()) > (recordY + parentSize.height()))  { //bottom
            qCDebug(dsrApp) << "Adjusting position: bottom bound reached";
            curPos.setY(recordY + parentSize.height() - size.height());
        }

//        QPoint resultPos = QPoint(curPos.x() - size.width() / 2,
//                                  curPos.y() - size.height() / 2);
        this->move(curPos);
    }
    DWidget::mouseMoveEvent(event);
}

void CameraWidget::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "CameraWidget mouseReleaseEvent";
    if (event->button() == Qt::LeftButton) {
        qCDebug(dsrApp) << "Left mouse button released";
        //改变移动状态.
        m_move = false;
        int centerX = this->x() + this->width() / 2;
        int centerY = this->y() + this->height() / 2;
        if (centerX - recordX < recordWidth / 2) {
            qCDebug(dsrApp) << "Adjusting position: centerX is in left half";
            if (centerY - recordY < recordHeight / 2) {
                qCDebug(dsrApp) << "Adjusting position: centerY is in top half, moving to top-left";
                this->move(recordX, recordY);
            } else {
                qCDebug(dsrApp) << "Adjusting position: centerY is in bottom half, moving to bottom-left";
                this->move(recordX, recordY + recordHeight - this->height());
            }
        } else {
            qCDebug(dsrApp) << "Adjusting position: centerX is in right half";
            if (centerY - recordY < recordHeight / 2) {
                qCDebug(dsrApp) << "Adjusting position: centerY is in top half, moving to top-right";
                this->move(recordX + recordWidth - this->width(), recordY);
            } else {
                qCDebug(dsrApp) << "Adjusting position: centerY is in bottom half, moving to bottom-right";
                this->move(recordX + recordWidth - this->width(), recordY + recordHeight - this->height());
            }
        }
    }
}

CameraWidget::Position CameraWidget::postion()
{
    qCDebug(dsrApp) << "postion called";
    if (this->x() == recordX) {
        qCDebug(dsrApp) << "Widget X matches recordX";
        if (this->y() == recordY) {
            qCDebug(dsrApp) << "Widget Y matches recordY, returning leftTop";
            return leftTop;
        } else {
            qCDebug(dsrApp) << "Widget Y does not match recordY, returning leftBottom";
            return leftBottom;
        }
    } else {
        qCDebug(dsrApp) << "Widget X does not match recordX";
        if (this->y() == recordY) {
            qCDebug(dsrApp) << "Widget Y matches recordY, returning rightTop";
            return rightTop;
        } else {
            qCDebug(dsrApp) << "Widget Y does not match recordY, returning rightBottom";
            return rightBottom;
        }
    }
}

// 设置CameraWidget是否可以拖动
void CameraWidget::setCameraWidgetImmovable(bool immovable)
{
    qCDebug(dsrApp) << "setCameraWidgetImmovable called with:" << immovable;
    m_Immovable = immovable;
}

int CameraWidget::getCameraStatus()
{
    qCDebug(dsrApp) << "getCameraStatus called";
    if (m_imgPrcThread) {
        int status = m_imgPrcThread->getStatus();
        qCDebug(dsrApp) << "Returning camera status:" << status;
        return status;
    }
    qCDebug(dsrApp) << "Image processing thread is null, returning 0";
    return 0;
}

void CameraWidget::setDevcieName(const QString &devcieName)
{
    qCDebug(dsrApp) << "setDevcieName called with:" << devcieName;
    m_deviceName = devcieName;
}
