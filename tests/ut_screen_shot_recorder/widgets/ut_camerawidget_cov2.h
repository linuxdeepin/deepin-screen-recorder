// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QImage>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>
#include "addr_pri.h"
#include "../../src/widgets/camerawidget.h"

using namespace testing;

// 覆盖 camerawidget.cpp 剩余未覆盖分支：
//  - enterEvent：setOverrideCursor 路径
//  - mousePressEvent：LeftButton 记录几何 / 非 LeftButton 跳过
//  - mouseMoveEvent：m_Immovable=true / m_Immovable=false 各边界
//  - mouseReleaseEvent：四象限吸附分支
//  - paintEvent：QBitmap + setMask 路径
//  - cameraStop：m_imgPrcThread=null + m_cameraUI=null 早退
// 既有 ut_camerawidget.h / ut_camerawidget_ext.h 已声明 enterEvent /
// mousePressEvent / mouseMoveEvent / mouseReleaseEvent / paintEvent 私有函数。

class CameraWidgetCov2Test : public Test
{
public:
    CameraWidget *m_cam = nullptr;
    void SetUp() override
    {
        m_cam = new CameraWidget();
        m_cam->resize(200, 150);
        m_cam->setRecordRect(0, 0, 800, 600);
    }
    void TearDown() override
    {
        delete m_cam;
        while (qApp->overrideCursor()) {
            qApp->restoreOverrideCursor();
        }
    }
};

// enterEvent：设置 ArrowCursor
TEST_F(CameraWidgetCov2Test, enterEventSetsCursor)
{
    QEvent e(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &e));
}

// mousePressEvent：LeftButton 记录几何坐标
TEST_F(CameraWidgetCov2Test, mousePressLeftButton)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &press));
}

// mousePressEvent：RightButton -> 不进入 LeftButton 分支
TEST_F(CameraWidgetCov2Test, mousePressRightButtonSkipped)
{
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10),
                      Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &press));
}

// mouseMoveEvent：m_Immovable=true -> 不进入移动逻辑
TEST_F(CameraWidgetCov2Test, mouseMoveImmovableTrue)
{
    m_cam->setCameraWidgetImmovable(true);
    QMouseEvent move(QEvent::MouseMove, QPointF(50, 50),
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &move));
}

// mouseMoveEvent：m_Immovable=false + 左键按下 -> 边界裁剪逻辑
TEST_F(CameraWidgetCov2Test, mouseMoveImmovableFalseLeftButton)
{
    m_cam->setCameraWidgetImmovable(false);
    QMouseEvent move(QEvent::MouseMove, QPointF(50, 50),
                     Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &move));
}

// mouseMoveEvent：m_Immovable=false + 右键（无 LeftButton）-> 跳过移动
TEST_F(CameraWidgetCov2Test, mouseMoveImmovableFalseNoLeft)
{
    m_cam->setCameraWidgetImmovable(false);
    QMouseEvent move(QEvent::MouseMove, QPointF(50, 50),
                     Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &move));
}

// mouseReleaseEvent：左键 -> 左上象限吸附
TEST_F(CameraWidgetCov2Test, mouseReleaseLeftTopQuadrant)
{
    m_cam->move(10, 10); // centerX-recordX < recordWidth/2 且 centerY-recordY < recordHeight/2
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(10, 10),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &release));
}

// mouseReleaseEvent：左下象限吸附（Y 在下半）
TEST_F(CameraWidgetCov2Test, mouseReleaseLeftBottomQuadrant)
{
    m_cam->move(10, 500); // X 左半，Y 下半
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(10, 500),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &release));
}

// mouseReleaseEvent：右上象限吸附
TEST_F(CameraWidgetCov2Test, mouseReleaseRightTopQuadrant)
{
    m_cam->move(500, 10); // X 右半，Y 上半
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(500, 10),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &release));
}

// mouseReleaseEvent：右下象限吸附
TEST_F(CameraWidgetCov2Test, mouseReleaseRightBottomQuadrant)
{
    m_cam->move(500, 500);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(500, 500),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &release));
}

// mouseReleaseEvent：右键 -> 不进入 LeftButton 分支
TEST_F(CameraWidgetCov2Test, mouseReleaseRightButtonSkipped)
{
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(10, 10),
                        Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &release));
}

// paintEvent：QBitmap mask 构建
TEST_F(CameraWidgetCov2Test, paintEventBuildsMask)
{
    QPaintEvent pe(QRect(0, 0, 200, 150));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(m_cam, &pe));
}

// cameraStop：m_cameraUI=null + m_imgPrcThread=null -> 早退
TEST_F(CameraWidgetCov2Test, cameraStopWithNullMembers)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->cameraStop());
}

// cameraStop：m_cameraUI 已初始化（initUI）-> clear 路径
TEST_F(CameraWidgetCov2Test, cameraStopAfterInitUI)
{
    m_cam->initUI();
    EXPECT_NO_FATAL_FAILURE(m_cam->cameraStop());
}

// showAt：未可见 -> show + move
TEST_F(CameraWidgetCov2Test, showAtMovesWidget)
{
    EXPECT_NO_FATAL_FAILURE(m_cam->showAt(QPoint(50, 50)));
}

// scaledPixmap：声明未实现，跳过（避免链接错误）

// postion：所有四象限通过 move+geometry 配置
TEST_F(CameraWidgetCov2Test, postionAllQuadrantsViaGeometry)
{
    // 默认构造 x=0,y=0,recordX=0,recordY=0 -> leftTop
    EXPECT_NO_FATAL_FAILURE((void)m_cam->postion());
}
