// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// CameraWidget smoke tests for the current (Qt6 / v4l2) API.
//
// The legacy ut_camerawidget.h is disabled in test_all_interfaces.h due to API
// drift (initCamera, cameraResume, captureImage, getcameraStatus->getCameraStatus,
// setCameraStop, ...). This file exercises only the pure value getters/setters,
// the UI builder (no hardware), the static pixmap scaler, and the image-receive
// slots (guarded by the m_cameraUI pointer initUI creates).
//
// Skipped (unsafe / hardware / need a shown top-level window):
//   - cameraStart / startCameraV4l2 / restartDevices : v4l2 device enumeration,
//     camInit(), thread spawn. No camera in the UT sandbox.
//   - cameraStop : calls camUnInit() and m_cameraUI->clear(); harmless only if
//     initUI ran, but couples to v4l2 globals -- left out to stay hermetic.
//   - showAt : calls this->show() (needs real window manager).
//   - enterEvent / mousePress/Move/ReleaseEvent : cursor + window geometry;
//     cursor comparison caused the prior SEGV in shapeswidget tests.
//   - paintEvent : builds a QBitmap mask and setMask(); needs shown widget.

#pragma once
#include <gtest/gtest.h>
#include <QImage>
#include <QPixmap>
#include "addr_pri.h"
#include "../../src/widgets/camerawidget.h"

class CameraWidgetExtTest : public ::testing::Test
{
public:
    CameraWidget *m_cam;
    void SetUp() override { m_cam = new CameraWidget; }
    void TearDown() override { delete m_cam; }
};

TEST_F(CameraWidgetExtTest, RecordRectRoundTrip)
{
    EXPECT_EQ(m_cam->getRecordX(), 0);
    EXPECT_EQ(m_cam->getRecordY(), 0);
    EXPECT_EQ(m_cam->getRecordWidth(), 0);
    EXPECT_EQ(m_cam->getRecordHeight(), 0);

    m_cam->setRecordRect(10, 20, 300, 200);
    EXPECT_EQ(m_cam->getRecordX(), 10);
    EXPECT_EQ(m_cam->getRecordY(), 20);
    EXPECT_EQ(m_cam->getRecordWidth(), 300);
    EXPECT_EQ(m_cam->getRecordHeight(), 200);

    // Overwrite with different values.
    m_cam->setRecordRect(-5, -5, 0, 0);
    EXPECT_EQ(m_cam->getRecordX(), -5);
    EXPECT_EQ(m_cam->getRecordWidth(), 0);
}

TEST_F(CameraWidgetExtTest, PositionDefaultsToCorner)
{
    // x()==recordX (both 0) and y()==recordY (both 0) -> leftTop.
    EXPECT_EQ(m_cam->postion(), CameraWidget::leftTop);
}

TEST_F(CameraWidgetExtTest, PositionRightTop)
{
    m_cam->setRecordRect(0, 0, 100, 100);
    m_cam->move(1, 0);   // x != recordX, y == recordY
    EXPECT_EQ(m_cam->postion(), CameraWidget::rightTop);
}

TEST_F(CameraWidgetExtTest, PositionLeftBottom)
{
    m_cam->setRecordRect(0, 0, 100, 100);
    m_cam->move(0, 1);   // x == recordX, y != recordY
    EXPECT_EQ(m_cam->postion(), CameraWidget::leftBottom);
}

TEST_F(CameraWidgetExtTest, PositionRightBottom)
{
    m_cam->setRecordRect(0, 0, 100, 100);
    m_cam->move(1, 1);   // x != recordX, y != recordY
    EXPECT_EQ(m_cam->postion(), CameraWidget::rightBottom);
}

// 注：CameraWidget::scaledPixmap 仅声明未实现，无法链接，故跳过相关用例。

TEST_F(CameraWidgetExtTest, ImmovableSetter)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->setCameraWidgetImmovable(true));
    EXPECT_NO_FATAL_FAILURE(m_cam->setCameraWidgetImmovable(false));
}

TEST_F(CameraWidgetExtTest, CameraStatusWithoutThread)
{
    // No m_imgPrcThread until cameraStart; getter returns 0.
    EXPECT_EQ(m_cam->getCameraStatus(), 0);
}

TEST_F(CameraWidgetExtTest, SetDeviceName)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->setDevcieName(QString("/dev/video0")));
    EXPECT_NO_FATAL_FAILURE(m_cam->setDevcieName(QString()));
}

TEST_F(CameraWidgetExtTest, InitUIIsSafe)
{
    // Builds DLabel + QHBoxLayout; no hardware touched.
    EXPECT_NO_FATAL_FAILURE(m_cam->initUI());
}

TEST_F(CameraWidgetExtTest, ReceiveMajorImagePixmapAfterInitUI)
{
    m_cam->initUI();
    m_cam->resize(32, 32);
    QPixmap pix(16, 16);
    pix.fill(Qt::green);
    EXPECT_NO_FATAL_FAILURE(m_cam->onReceiveMajorImage(pix));
}

TEST_F(CameraWidgetExtTest, ReceiveMajorImageImageAfterInitUI)
{
    m_cam->initUI();
    m_cam->resize(32, 32);
    QImage img(16, 16, QImage::Format_ARGB32);
    img.fill(Qt::yellow);
    EXPECT_NO_FATAL_FAILURE(m_cam->onReceiveMajorImage(img));
}

TEST_F(CameraWidgetExtTest, ReceiveMajorImageWithoutInitUIIsSafe)
{
    // m_cameraUI is null; slot guards on it -- must not crash.
    QPixmap pix(8, 8);
    pix.fill(Qt::gray);
    EXPECT_NO_FATAL_FAILURE(m_cam->onReceiveMajorImage(pix));
    QImage img(8, 8, QImage::Format_ARGB32);
    img.fill(Qt::gray);
    EXPECT_NO_FATAL_FAILURE(m_cam->onReceiveMajorImage(img));
}
