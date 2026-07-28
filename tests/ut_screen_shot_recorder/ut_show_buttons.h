// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QPoint>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/screenshot.h"
#include "../../src/keydefine.h"


using namespace testing;

class ShowButtonsTest: public testing::Test
{

public:
    Stub stub;
    ShowButtons showButtons;
    virtual void SetUp() override
    {
        std::cout << "start ShowButtonsTest" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "end ShowButtonsTest" << std::endl;
    }
};


