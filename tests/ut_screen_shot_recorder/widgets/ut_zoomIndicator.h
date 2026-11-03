// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "stub.h"
#include "addr_pri.h"

#include <QtTest>
#include <QColor>
#include <QDebug>
#include <QPainter>
#include <gtest/gtest.h>
#include "../../src/widgets/zoomIndicator.h"


using namespace testing;
// paintGL moved to the dedicated ZoomIndicatorGL widget; ZoomIndicator (a DLabel)
// now overrides paintEvent. Test paintEvent instead.
ACCESS_PRIVATE_FUN(ZoomIndicator, void(QPaintEvent *), paintEvent);

class ZoomIndicatorTest: public testing::Test, public QObject
{

public:
    ZoomIndicator *zoomindicator;
    virtual void SetUp() override
    {
        std::cout << "start ZoomIndicatorTest" << std::endl;
        zoomindicator = new ZoomIndicator(nullptr, false, true);

    }

    virtual void TearDown() override
    {
        delete zoomindicator;
        std::cout << "end ZoomIndicatorTest" << std::endl;
    }
};

//TEST_F(ZoomIndicatorTest, ButtonClicked)
//{
//    zoomindicator->showMagnifier(QPoint(100, 100));
//}

TEST_F(ZoomIndicatorTest, paintEvent)
{
    QPaintEvent *paintEvent = new QPaintEvent(QRect());
    call_private_fun::ZoomIndicatorpaintEvent(*zoomindicator, paintEvent);

    delete paintEvent;
}

TEST_F(ZoomIndicatorTest, externalSourceImageProvidesCursorColor)
{
    QImage source(4, 4, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(QStringLiteral("#12abef")));

    zoomindicator->setSourceImage(source);
    zoomindicator->setCursorPos(QPoint(2, 2));

    EXPECT_EQ(zoomindicator->currentColorHex(), QStringLiteral("#12abef"));
}

TEST_F(ZoomIndicatorTest, externalSourceImageOutsideBoundsHasNoColor)
{
    QImage source(4, 4, QImage::Format_ARGB32_Premultiplied);
    source.fill(Qt::red);

    zoomindicator->setSourceImage(source);
    zoomindicator->setCursorPos(QPoint(4, 4));

    EXPECT_TRUE(zoomindicator->currentColorHex().isEmpty());
}

TEST_F(ZoomIndicatorTest, externalSourceImageRendersPreview)
{
    QImage source(32, 32, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(QStringLiteral("#12abef")));
    zoomindicator->setSourceImage(source);
    zoomindicator->setCursorPos(QPoint(16, 16));

    QImage rendered(ZoomIndicator::PANEL_WIDTH,
                    ZoomIndicator::PANEL_HEIGHT,
                    QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::transparent);
    QPainter painter(&rendered);
    zoomindicator->render(&painter);

    EXPECT_EQ(rendered.pixelColor(20, 20), QColor(QStringLiteral("#12abef")));
}
