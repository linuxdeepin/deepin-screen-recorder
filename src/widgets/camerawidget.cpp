#include "camerawidget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QPainter>
#include <QHBoxLayout>
#include <QDir>
#include <QBitmap>

CameraWidget::CameraWidget(DWidget *parent) : DWidget(parent)
{
}

CameraWidget::~CameraWidget()
{
//    camera->stop();
//    timer_image_capture->stop();
//    delete camera;
//    delete viewfinder;
//    delete imageCapture;
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
    setWindowFlags(Qt::WindowStaysOnTopHint);
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

    camera = new QCamera(QCameraInfo::defaultCamera(), this);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(cameraInitError(QCamera::Error)));
    imageCapture = new QCameraImageCapture(camera);

    timer_image_capture = new QTimer(this);
    connect(timer_image_capture, &QTimer::timeout, this, &CameraWidget::captureImage);
}

bool CameraWidget::cameraStart()
{
    camera->start();

    if (imageCapture->isCaptureDestinationSupported(QCameraImageCapture::CaptureToFile)) {
        imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);
        qDebug() << imageCapture->supportedBufferFormats();
        imageCapture->setBufferFormat(QVideoFrame::PixelFormat::Format_Jpeg);
        qDebug() << imageCapture->supportedResolutions(imageCapture->encodingSettings());

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

        QImageEncoderSettings iamge_setting;
        iamge_setting.setResolution(t_resolutionSize.width(), t_resolutionSize.height());

        qDebug() << iamge_setting.resolution();
        imageCapture->setEncodingSettings(iamge_setting);
        connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &CameraWidget::processCapturedImage);
        connect(imageCapture, &QCameraImageCapture::imageSaved, this, &CameraWidget::deleteCapturedImage);
        timer_image_capture->start(50);
        return true;
    }

    else {
        return false;
    }
}

void CameraWidget::cameraStop()
{
    timer_image_capture->stop();
    camera->stop();
    m_cameraUI->clear();
}

void CameraWidget::cameraResume()
{
    qDebug() << "QCameraInfo::availableCameras" << QCameraInfo::defaultCamera();
    camera = new QCamera(QCameraInfo::defaultCamera(), this);
//    camera->load();
    camera->setCaptureMode(QCamera::CaptureStillImage);
    imageCapture = new QCameraImageCapture(camera);
}

bool CameraWidget::getScreenResolution()
{
    return m_wildScreen;
}

void CameraWidget::captureImage()
{
    imageCapture->capture(m_capturePath);
}

void CameraWidget::processCapturedImage(int request_id, const QImage &img)
{
    QImage t_image = img.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
//    QPixmap pixmap = round(QPixmap::fromImage(t_image), 35);
    QPixmap pixmap = QPixmap::fromImage(t_image);
    m_cameraUI->setPixmap(pixmap);
}

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

void CameraWidget::deleteCapturedImage(int id, const QString &fileName)
{
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
    }
    return true;

}

bool CameraWidget::getcameraStatus()
{
    qDebug() << "camera->state()" << camera->state();
    if ( camera->state() != QCamera::ActiveState) {
        return false;
    }

    else {
        return true;
    }
}

void CameraWidget::cameraStatus()
{
    qDebug() << "camera->status()" << camera->status();
}

void CameraWidget::cameraInitError(QCamera::Error error)
{
    qDebug() << "camera->error()" << error;
//    camera = new QCamera(QCameraInfo::defaultCamera(), this);
}

void CameraWidget::enterEvent(QEvent *e)
{
    qApp->setOverrideCursor(Qt::ArrowCursor);
    qDebug() << "CameraWidget enterEvent";
}

void CameraWidget::paintEvent(QPaintEvent *e)
{
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
    if (event->buttons() & Qt::LeftButton) {
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

        if ( (curPos.x() + size.width()) > (recordX + parentSize.width())) //right
            curPos.setX(recordX + parentSize.width() - size.width());

        if ( (curPos.y() + size.height()) > (recordY + parentSize.height())) //bottom
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
