// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QRect>
#include <QTimer>
#include <unistd.h>
#include "../src/process_tree.h"


using namespace testing;

class ProcessTreeTest:public testing::Test{

    //typedef std::map<int, proc_t> StoredProcType;
public:
    ProcessTree *processTree;
    virtual void SetUp() override{
        std::cout << "start RecordProcessTest" << std::endl;
        processTree = new ProcessTree;
    }

    virtual void TearDown() override{
        delete processTree;
        std::cout << "end RecordProcessTest" << std::endl;
    }
};

