// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "camerawidget.h"

#include "../camera/cameracontroller.h"
#include "../camera/qcameracontroller.h"
#include "../utils/log.h"

#include <QApplication>
#include <QBitmap>
#include <QDebug>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

namespace {

QString selectDeviceId(const QList<CameraDeviceDescriptor> &devices,
                       const QString &preferredId)
{
    if (devices.isEmpty())
        return {};

    for (const CameraDeviceDescriptor &device : devices) {
        if (!preferredId.isEmpty() && device.id == preferredId)
            return device.id;
    }

    for (const CameraDeviceDescriptor &device : devices) {
        if (device.isDefault)
            return device.id;
    }

    return devices.first().id;
}

} // namespace

CameraWidget::CameraWidget(DWidget *parent, CameraControllerFactory factory)
    : DWidget(parent)
{
    qCDebug(dsrApp) << "CameraWidget constructor entered";

    m_controller = factory ? factory(this) : new QtCameraController(this);
    if (m_controller && !m_controller->parent())
        m_controller->setParent(this);

    if (m_controller) {
        connect(m_controller, &CameraController::frameReady, this,
                [this](const QImage &image) { onReceiveMajorImage(image); });
        connect(m_controller, &CameraController::errorOccurred,
                this, &CameraWidget::handleControllerError);
    }
}

CameraWidget::~CameraWidget()
{
    qCDebug(dsrApp) << "CameraWidget destructor entered";
    cameraStop();
    qCDebug(dsrApp) << "~CameraWidget() finished";
}

void CameraWidget::setRecordRect(int x, int y, int width, int height)
{
    qCDebug(dsrApp) << "setRecordRect called with x:" << x << ", y:" << y
                    << ", width:" << width << ", height:" << height;
    recordX = x;
    recordY = y;
    recordWidth = width;
    recordHeight = height;
}

void CameraWidget::showAt(QPoint pos)
{
    qCDebug(dsrApp) << "showAt called with position:" << pos;
    if (!isVisible())
        show();
    move(pos.x(), pos.y());
}

void CameraWidget::setParentGlobalOffset(const QPoint &offset)
{
    m_parentGlobalOffset = offset;
}

QPoint CameraWidget::parentGlobalOffset() const
{
    return m_parentGlobalOffset;
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

void CameraWidget::initUI()
{
    if (m_cameraUI)
        return;

    qCDebug(dsrApp) << "Initializing camera UI...";
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);

    m_cameraUI = new DLabel(this);
    m_cameraUI->setAlignment(Qt::AlignCenter);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_cameraUI);
    setLayout(layout);

    qCDebug(dsrApp) << "Camera UI initialization completed";
}

void CameraWidget::cameraStart()
{
    qCDebug(dsrApp) << "cameraStart called";

    if (!m_controller) {
        emit cameraError(tr("Camera backend is unavailable"));
        return;
    }

    if (m_startRequested || m_controller->isActive())
        return;

    const QList<CameraDeviceDescriptor> devices = m_controller->availableDevices();
    if (devices.isEmpty()) {
        qCWarning(dsrApp) << "No camera device found";
        emit cameraError(tr("No camera device found"));
        return;
    }

    const QString deviceId = selectDeviceId(devices, m_deviceName);
    m_deviceName = deviceId;
    clearPreview();
    qCDebug(dsrApp) << "Starting camera device:" << deviceId;

    m_startRequested = m_controller->start(deviceId);
    if (!m_startRequested)
        qCWarning(dsrApp) << "Failed to start camera device:" << deviceId;
}

void CameraWidget::startCameraById(const QString &preferredId)
{
    if (!m_controller)
        return;

    const QList<CameraDeviceDescriptor> devices = m_controller->availableDevices();
    if (devices.isEmpty()) {
        emit cameraError(tr("No camera device found"));
        return;
    }

    const QString deviceId = selectDeviceId(devices, preferredId);
    if (deviceId.isEmpty())
        return;

    clearPreview();
    m_deviceName = deviceId;
    m_startRequested = m_controller->start(deviceId);
}

void CameraWidget::restartDevices()
{
    qCDebug(dsrApp) << "restartDevices called";

    if (!m_controller)
        return;

    const QList<CameraDeviceDescriptor> devices = m_controller->availableDevices();
    if (devices.isEmpty()) {
        cameraStop();
        emit cameraError(tr("No camera device found"));
        return;
    }

    int currentIndex = -1;
    for (int i = 0; i < devices.size(); ++i) {
        if (devices.at(i).id == m_deviceName) {
            currentIndex = i;
            break;
        }
    }

    QString nextDeviceId;
    if (currentIndex < 0) {
        nextDeviceId = selectDeviceId(devices, QString());
    } else {
        nextDeviceId = devices.at((currentIndex + 1) % devices.size()).id;
    }

    qCDebug(dsrApp) << "Switching camera device to:" << nextDeviceId;
    startCameraById(nextDeviceId);
}

void CameraWidget::cameraStop()
{
    qCDebug(dsrApp) << "cameraStop called";
    clearPreview();

    m_startRequested = false;
    if (m_controller)
        m_controller->stop();
}

void CameraWidget::clearPreview()
{
    if (m_cameraUI)
        m_cameraUI->clear();
}

void CameraWidget::handleControllerError(const QString &message)
{
    qCWarning(dsrApp) << "Camera controller error:" << message;
    m_startRequested = false;
    clearPreview();
    emit cameraError(message);

    // 让调用栈先返回 Qt Multimedia 内部，再停止并释放摄像头资源。
    QTimer::singleShot(0, this, [this]() {
        m_startRequested = false;
        if (m_controller)
            m_controller->stop();
    });
}

void CameraWidget::onReceiveMajorImage(QImage image)
{
    if (image.isNull() || !m_cameraUI || width() <= 0 || height() <= 0)
        return;

    const QImage scaledImage = image.scaled(size(),
                                            Qt::IgnoreAspectRatio,
                                            Qt::FastTransformation);
    m_cameraUI->setPixmap(QPixmap::fromImage(scaledImage));
}

void CameraWidget::onReceiveMajorImage(QPixmap pixmap)
{
    if (pixmap.isNull() || !m_cameraUI || width() <= 0 || height() <= 0)
        return;

    pixmap = pixmap.scaled(size(),
                           Qt::IgnoreAspectRatio,
                           Qt::FastTransformation);
    m_cameraUI->setPixmap(pixmap);
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

    QBitmap bmp(size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);

    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawRoundedRect(12, 12, width() - 25, height() - 25, 8, 8);
    setMask(bmp);
}

void CameraWidget::mousePressEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "CameraWidget mousePressEvent";
    if (event->button() == Qt::LeftButton) {
        m_move = true;
        m_startPoint = event->globalPos();
        m_windowTopLeftPoint = frameGeometry().topLeft();
        m_windowTopRightPoint = frameGeometry().topRight();
        m_windowBottomLeftPoint = frameGeometry().bottomLeft();
    }
}

void CameraWidget::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "CameraWidget mouseMoveEvent";
    if ((event->buttons() & Qt::LeftButton) && !m_Immovable) {
        const QSize widgetSize = size();
        const QSize parentSize(recordWidth, recordHeight);
        QPoint curPos(event->globalX() - widgetSize.width() / 2,
                      event->globalY() - widgetSize.height() / 2);

        if (curPos.x() < recordX)
            curPos.setX(recordX);

        if (curPos.y() < recordY)
            curPos.setY(recordY);

        if ((curPos.x() + widgetSize.width()) > (recordX + parentSize.width()))
            curPos.setX(recordX + parentSize.width() - widgetSize.width());

        if ((curPos.y() + widgetSize.height()) > (recordY + parentSize.height()))
            curPos.setY(recordY + parentSize.height() - widgetSize.height());

        move(curPos - m_parentGlobalOffset);
    }
    DWidget::mouseMoveEvent(event);
}

void CameraWidget::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "CameraWidget mouseReleaseEvent";
    if (event->button() == Qt::LeftButton) {
        m_move = false;
        const QPoint globalTopLeft = mapToGlobal(QPoint(0, 0));
        const int centerX = globalTopLeft.x() + width() / 2;
        const int centerY = globalTopLeft.y() + height() / 2;
        QPoint targetGlobalPos;

        if (centerX - recordX < recordWidth / 2) {
            if (centerY - recordY < recordHeight / 2)
                targetGlobalPos = QPoint(recordX, recordY);
            else
                targetGlobalPos = QPoint(recordX, recordY + recordHeight - height());
        } else {
            if (centerY - recordY < recordHeight / 2)
                targetGlobalPos = QPoint(recordX + recordWidth - width(), recordY);
            else
                targetGlobalPos = QPoint(recordX + recordWidth - width(),
                                         recordY + recordHeight - height());
        }

        move(targetGlobalPos - m_parentGlobalOffset);
    }
}

CameraWidget::Position CameraWidget::postion()
{
    qCDebug(dsrApp) << "postion called";
    const QPoint globalTopLeft = mapToGlobal(QPoint(0, 0));
    if (globalTopLeft.x() == recordX) {
        if (globalTopLeft.y() == recordY)
            return leftTop;
        return leftBottom;
    }

    if (globalTopLeft.y() == recordY)
        return rightTop;
    return rightBottom;
}

void CameraWidget::setCameraWidgetImmovable(bool immovable)
{
    qCDebug(dsrApp) << "setCameraWidgetImmovable called with:" << immovable;
    m_Immovable = immovable;
}

int CameraWidget::getCameraStatus()
{
    return m_startRequested || (m_controller && m_controller->isActive()) ? 1 : 0;
}

void CameraWidget::setDevcieName(const QString &devcieName)
{
    qCDebug(dsrApp) << "setDevcieName called with:" << devcieName;
    m_deviceName = devcieName;
}
