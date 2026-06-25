// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QImage>
#include "../../src/widgets/previewwidget.h"

using namespace testing;

class PreviewWidgetTest : public Test
{
public:
    PreviewWidget *m_pw = nullptr;
    void SetUp() override
    {
        m_pw = new PreviewWidget(QRect(0, 0, 1920, 1080));
    }
    void TearDown() override
    {
        delete m_pw;
    }
};

TEST_F(PreviewWidgetTest, constructNotNull)
{
    EXPECT_NE(m_pw, nullptr);
}

TEST_F(PreviewWidgetTest, positionStatusEnumValues)
{
    EXPECT_EQ(PreviewWidget::RIGHT, 0);
    EXPECT_EQ(PreviewWidget::LEFT, 1);
    EXPECT_EQ(PreviewWidget::INSIDE, 2);
}

TEST_F(PreviewWidgetTest, setPreviewWidgetStatusPosRight)
{
    EXPECT_NO_FATAL_FAILURE(m_pw->setPreviewWidgetStatusPos(PreviewWidget::RIGHT));
    EXPECT_EQ(m_pw->getPreviewPostion(), PreviewWidget::RIGHT);
}

TEST_F(PreviewWidgetTest, setPreviewWidgetStatusPosLeft)
{
    EXPECT_NO_FATAL_FAILURE(m_pw->setPreviewWidgetStatusPos(PreviewWidget::LEFT));
    EXPECT_EQ(m_pw->getPreviewPostion(), PreviewWidget::LEFT);
}

TEST_F(PreviewWidgetTest, setPreviewWidgetStatusPosInside)
{
    EXPECT_NO_FATAL_FAILURE(m_pw->setPreviewWidgetStatusPos(PreviewWidget::INSIDE));
    EXPECT_EQ(m_pw->getPreviewPostion(), PreviewWidget::INSIDE);
}

TEST_F(PreviewWidgetTest, updateImageWithNull)
{
    QImage nullImg;
    EXPECT_NO_FATAL_FAILURE(m_pw->updateImage(nullImg));
}

TEST_F(PreviewWidgetTest, updateImageWithValid)
{
    QImage img(100, 100, QImage::Format_ARGB32);
    img.fill(Qt::white);
    EXPECT_NO_FATAL_FAILURE(m_pw->updateImage(img));
}

TEST_F(PreviewWidgetTest, previewGeomtroy)
{
    QRect geom = m_pw->previewGeomtroy();
    EXPECT_GE(geom.width(), 0);
    EXPECT_GE(geom.height(), 0);
}

TEST_F(PreviewWidgetTest, calculatePreviewPosition)
{
    QRect rect = m_pw->calculatePreviewPosition(200, 150);
    EXPECT_GE(rect.width(), 0);
}

TEST_F(PreviewWidgetTest, setScreenInfo)
{
    EXPECT_NO_FATAL_FAILURE(m_pw->setScreenInfo(1920, 1.0));
    EXPECT_NO_FATAL_FAILURE(m_pw->setScreenInfo(2560, 1.25));
}

TEST_F(PreviewWidgetTest, initPreviewWidget)
{
    EXPECT_NO_FATAL_FAILURE(m_pw->initPreviewWidget());
}

TEST_F(PreviewWidgetTest, updatePreviewSize)
{
    EXPECT_NO_FATAL_FAILURE(m_pw->updatePreviewSize(QRect(100, 100, 800, 600)));
}

TEST_F(PreviewWidgetTest, paintEventRunsClean)
{
    m_pw->show();
    EXPECT_NO_FATAL_FAILURE(m_pw->repaint());
    m_pw->hide();
}

TEST_F(PreviewWidgetTest, destructorSafe)
{
    PreviewWidget *tmp = new PreviewWidget(QRect(0, 0, 1280, 720));
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}
