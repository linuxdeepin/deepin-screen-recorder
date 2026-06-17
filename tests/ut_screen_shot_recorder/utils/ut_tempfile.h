// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QScreen>
#include <QPixmap>
#include <QApplication>
#include <gtest/gtest.h>
#include "../../src/utils/tempfile.h"


using namespace testing;

class TempFileTest:public testing::Test{

public:
    TempFile* tempFile = nullptr;
    QScreen *m_primaryScreen;
    QPixmap m_pix;
    virtual void SetUp() override{
        tempFile = TempFile::instance();
        m_primaryScreen = QGuiApplication::primaryScreen();
        m_pix = m_primaryScreen->grabWindow(0);
        std::cout << "start TempFileTest" << std::endl;
    }

    virtual void TearDown() override{
        //delete tempFile;
        std::cout << "end TempFileTest" << std::endl;
    }
};


TEST_F(TempFileTest, setFullScreenPixmap)
{
    tempFile->setFullScreenPixmap(m_pix);
    tempFile->getFullscreenPixmap();
    EXPECT_NE(nullptr, tempFile);
}

TEST_F(TempFileTest, setBlurPixmap)
{
    const int radius = 10;
    tempFile->setBlurPixmap(m_pix, radius);
    tempFile->getBlurPixmap(radius);
    EXPECT_NE(nullptr, tempFile);
}

TEST_F(TempFileTest, setMosaicPixmap)
{
    const int radius = 10;
    tempFile->setMosaicPixmap(m_pix, radius);
    tempFile->getMosaicPixmap(radius);
    EXPECT_NE(nullptr, tempFile);
}
