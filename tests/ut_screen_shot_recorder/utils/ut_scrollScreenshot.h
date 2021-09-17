/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangwenchao <zhangwenchao@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/utils/scrollScreenshot.h"

using namespace std;
using namespace testing;


ACCESS_PRIVATE_FIELD(ScrollScreenshot, PixMergeThread *, m_PixMerageThread);
ACCESS_PRIVATE_FIELD(ScrollScreenshot, ScrollScreenshot::ScrollStatus, m_curStatus);

class ScrollScreenshotTest: public testing::Test
{
public:
    Stub stub;
    ScrollScreenshot *m_ScrollScreenshot = nullptr;
    virtual void SetUp() override
    {
        if (m_ScrollScreenshot == nullptr)
            m_ScrollScreenshot = new ScrollScreenshot();
        std::cout << "start ScrollScreenshotTest" << std::endl;
    }

    virtual void TearDown() override
    {
        if (nullptr != m_ScrollScreenshot) {
            delete m_ScrollScreenshot;
            m_ScrollScreenshot = nullptr;
        }
        std::cout << "end ScrollScreenshotTest" << std::endl;
    }
};

//添加图片
TEST_F(ScrollScreenshotTest, startAddPixmap)
{
    QPixmap img(":/testImg/addImg1.png");
    m_ScrollScreenshot->setScrollModel(false);
    m_ScrollScreenshot->addPixmap(img);
    QTest::qWait(500);
    m_ScrollScreenshot->setScrollModel(true);
    m_ScrollScreenshot->addPixmap(img);
    QTest::qWait(50);
    PixMergeThread *m_PixMerageThread = access_private_field::ScrollScreenshotm_PixMerageThread(*m_ScrollScreenshot);
    m_PixMerageThread->stopTask();
    m_PixMerageThread->wait();
}

TEST_F(ScrollScreenshotTest, changeState)
{
    ScrollScreenshot::ScrollStatus &m_curStatus = access_private_field::ScrollScreenshotm_curStatus(*m_ScrollScreenshot);
    m_curStatus = ScrollScreenshot::ScrollStatus::Merging;
    m_ScrollScreenshot->changeState(true);
    QTest::qWait(50);
    m_curStatus = ScrollScreenshot::ScrollStatus::Stop;
    m_ScrollScreenshot->changeState(false);
}

TEST_F(ScrollScreenshotTest, savePixmap)
{
    ScrollScreenshot::ScrollStatus &m_curStatus = access_private_field::ScrollScreenshotm_curStatus(*m_ScrollScreenshot);
    m_curStatus = ScrollScreenshot::ScrollStatus::Merging;
    m_ScrollScreenshot->savePixmap();
}

TEST_F(ScrollScreenshotTest, merageImgState)
{
    PixMergeThread::MergeErrorValue state = PixMergeThread::MergeErrorValue::Failed;
    PixMergeThread *m_PixMerageThread = access_private_field::ScrollScreenshotm_PixMerageThread(*m_ScrollScreenshot);
    emit m_PixMerageThread->merageError(state);
    QTest::qWait(50);

    state = PixMergeThread::MergeErrorValue::MaxHeight;
    emit m_PixMerageThread->merageError(state);
}

TEST_F(ScrollScreenshotTest, merageInvalidArea)
{
    QRect rect(1, 1, 1, 1);
    PixMergeThread::MergeErrorValue state = PixMergeThread::MergeErrorValue::InvalidArea;
    PixMergeThread *m_PixMerageThread = access_private_field::ScrollScreenshotm_PixMerageThread(*m_ScrollScreenshot);
    emit m_PixMerageThread->invalidAreaError(state, rect);
    QTest::qWait(50);

    state = PixMergeThread::MergeErrorValue::MaxHeight;
    emit m_PixMerageThread->invalidAreaError(state, rect);
}

TEST_F(ScrollScreenshotTest, ScrollScreenshotOthers)
{
    m_ScrollScreenshot->clearPixmap();
    m_ScrollScreenshot->getInvalidArea();
    m_ScrollScreenshot->setTimeAndCalculateTimeDiff(77942255);
}
