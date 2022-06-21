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

#include "camerawidget.h"

#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QPainter>
#include <QHBoxLayout>
#include <QDir>
#include <QBitmap>
#include <QtConcurrent>


CameraWidget::CameraWidget(DWidget *parent) : DWidget(parent)
{
}

CameraWidget::~CameraWidget()
{
//    camera->stop();
//    timer_image_capture->stop();
//    delete camera;
//    delete viewfinder;
//    if(nullptr != imageCapture){
//        delete imageCapture;
//        imageCapture = nullptr;
//    }
//    delete timer_image_capture;
//    delete m_cameraUI;
}


void CameraWidget::setRecordRect(int x, int y, int width, int height)
{
    this->recordX = x;
    this->recordY = y;
    this->recordWidth = width;
    this->recordHeight = height;
}

void CameraWidget::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();
    move(pos.x(), pos.y());
}

int CameraWidget::getRecordX()
{
    return recordX;
}
int CameraWidget::getRecordY()
{
    return recordY;
}
int CameraWidget::getRecordWidth()
{
    return recordWidth;
}
int CameraWidget::getRecordHeight()
{
    return recordHeight;
}
void CameraWidget::initCamera()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);
    m_cameraUI = new DLabel(this);
    m_wildScreen = false;

    m_capturePath = QDir::homePath() + QDir::separator() + "." + qApp->applicationName();
    QDir t_appDir;
    t_appDir.mkpath(m_capturePath);

    QHBoxLayout *t_hlayout = new QHBoxLayout(this);
    t_hlayout->addWidget(m_cameraUI);
    this->setLayout(t_hlayout);

    while (QCameraInfo::defaultCamera().isNull()) {
        QEventLoop loop;
        QTimer::singleShot(200, &loop, SLOT(quit()));
        loop.exec();
    }
    camera = new QCamera(QCameraInfo::defaultCamera(), this);
    m_deviceName = QCameraInfo::defaultCamera().deviceName();
    qDebug() << "当前摄像头的设备名称: " << m_deviceName;
    m_deviceFile = new QFile(this);
    m_deviceFile->setFileName(m_deviceName);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(cameraInitError(QCamera::Error)));
    imageCapture = new QCameraImageCapture(camera, this);

    timer_image_capture = new QTimer(this);
    connect(timer_image_capture, &QTimer::timeout, this, &CameraWidget::captureImage);
}

bool CameraWidget::cameraStart()
{
    if (m_isInitCamera) {
        timer_image_capture->start(50);
        return true;
    } else {
        camera->start();
    }

    if (imageCapture->isCaptureDestinationSupported(QCameraImageCapture::CaptureToFile)) {
        imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
        qInfo() << "摄像头支持的缓存图片格式： " << imageCapture->supportedBufferFormats();
        imageCapture->setBufferFormat(QVideoFrame::PixelFormat::Format_Jpeg);
        qInfo()  << "图像支持的可以用来编码的分辨率列表： " << imageCapture->supportedResolutions(imageCapture->encodingSettings());
        QList<QSize> t_capSizeLst = imageCapture->supportedResolutions(imageCapture->encodingSettings());

        QSize t_resolutionSize;
        if (t_capSizeLst.isEmpty()) {
            qDebug() << "no resolution";
            return false;
        }

        if (t_capSizeLst.contains(QSize(640, 360))) {
            t_resolutionSize = QSize(640, 360);
            m_wildScreen = false;
        }

        else if (t_capSizeLst.contains(QSize(640, 480))) {
            t_resolutionSize = QSize(640, 480);
            m_wildScreen = true;
        }

        else {
            t_resolutionSize = t_capSizeLst.last();
            m_wildScreen = false;
        }

        //图片编码器设置
        QImageEncoderSettings iamge_setting;
        iamge_setting.setResolution(t_resolutionSize.width(), t_resolutionSize.height());
        qInfo() << "返回编码图像的分辨率: " <<  iamge_setting.resolution();
        imageCapture->setEncodingSettings(iamge_setting);
        connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &CameraWidget::processCapturedImage);
        connect(imageCapture, &QCameraImageCapture::imageSaved, this, &CameraWidget::deleteCapturedImage);
        timer_image_capture->start(200);
        m_isInitCamera = true;
        return true;
    }


    return false;
}

void CameraWidget::cameraStop()
{
    timer_image_capture->stop();
    //camera->stop();
    //camera->unload();
    //m_cameraUI->clear();
}

void CameraWidget::cameraResume()
{
    qDebug() << "QCameraInfo::availableCameras" << QCameraInfo::defaultCamera();
    camera = new QCamera(QCameraInfo::defaultCamera(), this);
    m_deviceName = QCameraInfo::defaultCamera().deviceName();
    m_deviceFile->setFileName(m_deviceName);
//    camera->load();
    camera->setCaptureMode(QCamera::CaptureStillImage);
    imageCapture = new QCameraImageCapture(camera, this);

    m_isInitCamera = false;
}
/*
 * never used
bool CameraWidget::getScreenResolution()
{
    return m_wildScreen;
}
*/
void CameraWidget::captureImage()
{
    imageCapture->capture(m_capturePath);
}

void CameraWidget::processCapturedImage(int request_id, const QImage &img)
{
    Q_UNUSED(request_id);
    QImage t_image = img.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
//    QPixmap pixmap = round(QPixmap::fromImage(t_image), 35);
    QPixmap pixmap = QPixmap::fromImage(t_image);
    m_cameraUI->setPixmap(pixmap);
}
/*
 * never used
QPixmap CameraWidget::round(const QPixmap &img_in, int radius)
{
    if (img_in.isNull()) {
        return QPixmap();
    }
    QSize size(img_in.size());
    QBitmap mask(size);
    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(mask.rect(), Qt::transparent);
    painter.setBrush(Qt::transparent);
    painter.drawRoundedRect(mask.rect(), radius, radius);
    QPixmap image = img_in;// .scaled(size);
    image.setMask(mask);
    return image;
}
*/
void CameraWidget::deleteCapturedImage(int id, const QString &fileName)
{
    Q_UNUSED(id);
    QFile::remove(fileName);
}

bool CameraWidget::setCameraStop(bool status)
{
    Q_UNUSED(status);

    camera->unload();
//    delete camera;

    if (camera) {
        delete camera;
    }

    if (imageCapture) {
        delete imageCapture;
        imageCapture = nullptr;
    }
    return true;

}

bool CameraWidget::getcameraStatus()
{
    if (camera->state() != QCamera::ActiveState || !m_deviceFile->exists()) {
        return false;
    }

    else {
        return true;
    }
}
/*
 * never used
void CameraWidget::cameraStatus()
{
    qDebug() << "camera->status()" << camera->status();
}
*/
void CameraWidget::cameraInitError(QCamera::Error error)
{
    qDebug() << "camera->error()" << error;
//    camera = new QCamera(QCameraInfo::defaultCamera(), this);
}

void CameraWidget::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qDebug() << "CameraWidget enterEvent";
}

void CameraWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
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
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);
//    p.fillRect(this->rect(), Qt::transparent);
//    p.setBrush(Qt::transparent);
//    p.drawRoundedRect(bmp.rect(), 30, 45, Qt::RelativeSize);
    p.drawRoundedRect(12, 12, width() - 25, height() - 25, 8, 8);
    setMask(bmp);



//    DWidget::paintEvent(e);
}
void CameraWidget::mousePressEvent(QMouseEvent *event)
{
    //当鼠标左键点击时.
    if (event->button() == Qt::LeftButton) {
        qDebug() << "CameraWidget mousePressEvent";
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
    if ((event->buttons() & Qt::LeftButton) && m_Immovable == false) {
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
        if (curPos.x() < recordX) //left
            curPos.setX(recordX);

        if (curPos.y() < recordY) //top
            curPos.setY(recordY);

        if ((curPos.x() + size.width()) > (recordX + parentSize.width()))  //right
            curPos.setX(recordX + parentSize.width() - size.width());

        if ((curPos.y() + size.height()) > (recordY + parentSize.height()))  //bottom
            curPos.setY(recordY + parentSize.height() - size.height());

//        QPoint resultPos = QPoint(curPos.x() - size.width() / 2,
//                                  curPos.y() - size.height() / 2);
        this->move(curPos);
    }
    DWidget::mouseMoveEvent(event);
}

void CameraWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        //改变移动状态.
        m_move = false;
        int centerX = this->x() + this->width() / 2;
        int centerY = this->y() + this->height() / 2;
        if (centerX - recordX < recordWidth / 2) {
            if (centerY - recordY < recordHeight / 2) {
                this->move(recordX, recordY);
            } else {
                this->move(recordX, recordY + recordHeight - this->height());
            }
        } else {
            if (centerY - recordY < recordHeight / 2) {
                this->move(recordX + recordWidth - this->width(), recordY);
            } else {
                this->move(recordX + recordWidth - this->width(), recordY + recordHeight - this->height());
            }
        }
    }
}

CameraWidget::Position CameraWidget::postion()
{
    if (this->x() == recordX) {
        if (this->y() == recordY) {
            return leftTop;
        } else {
            return leftBottom;
        }
    } else {
        if (this->y() == recordY) {
            return rightTop;
        } else {
            return rightBottom;
        }
    }
}

// 设置CameraWidget是否可以拖动
void CameraWidget::setCameraWidgetImmovable(bool immovable)
{
    m_Immovable = immovable;
}
