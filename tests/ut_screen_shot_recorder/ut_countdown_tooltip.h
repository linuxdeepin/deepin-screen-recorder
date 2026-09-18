// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QObject>
#include <QScreen>
#include <QDebug>
#include <QPainter>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/countdown_tooltip.h"
#include "../../src/utils/tempfile.h"


using namespace testing;

class CountdownTooltipTest: public testing::Test
{

public:
    Stub stub;
    CountdownTooltip *m_countTip = new CountdownTooltip;
    virtual void SetUp() override
    {
        std::cout << "start CountdownTooltipTest" << std::endl;

    }

    virtual void TearDown() override
    {
        m_countTip->hide();
        delete m_countTip;
        std::cout << "end CountdownTooltipTest" << std::endl;
    }
public slots:
    void startRecord();
};

void CountdownTooltipTest::startRecord()
{
    qDebug() << "startRecord";
}

QPixmap getFullscreenPixmap_stub()
{

    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    return t_primaryScreen->grabWindow(0, 0, 0, 1920, 1080);

}

ACCESS_PRIVATE_FUN(CountdownTooltip, void(QPaintEvent *), paintEvent);
ACCESS_PRIVATE_FIELD(CountdownTooltip, int, showCountdownCounter);
ACCESS_PRIVATE_FUN(CountdownTooltip, QPixmap(), getTooltipBackground);
ACCESS_PRIVATE_FUN(CountdownTooltip, void(QPainter &, QPixmap &), paintRect);
ACCESS_PRIVATE_FIELD(CountdownTooltip, QTimer *, showCountdownTimer);
