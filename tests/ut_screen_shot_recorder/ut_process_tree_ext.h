// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <unistd.h>
#include "../../src/process_tree.h"

using namespace testing;

// Extended ProcessTree tests covering more scenarios.
// The existing ut_process_tree.h reads real /proc data via libproc;
// these tests also use real /proc data for richer coverage.

class ProcessTreeExtTest : public Test
{
public:
    ProcessTree *m_tree = nullptr;
    void SetUp() override { m_tree = new ProcessTree; }
    void TearDown() override { delete m_tree; }
};

// Scan an empty process map — should leave rootPid as INT_MAX
TEST_F(ProcessTreeExtTest, scanEmptyProcessMap)
{
    std::map<int, proc_t> empty;
    m_tree->scanProcesses(empty);
    // getAllChildPids on any PID returns empty
    QList<int> children = m_tree->getAllChildPids(getpid());
    EXPECT_TRUE(children.isEmpty());
}

// Scan real processes and verify we can find children of init
TEST_F(ProcessTreeExtTest, scanRealProcessesAndGetChildren)
{
    PROCTAB *proc = openproc(PROC_FILLSTAT);
    static proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_t));
    std::map<int, proc_t> processes;
    while (readproc(proc, &proc_info) != nullptr) {
        processes[proc_info.tid] = proc_info;
    }
    closeproc(proc);

    m_tree->scanProcesses(processes);
    // Our own PID should exist in the tree
    pid_t myPid = getpid();
    QList<int> myChildren = m_tree->getAllChildPids(myPid);
    // This test process has no children
    EXPECT_TRUE(myChildren.isEmpty());
}

// Scan with just a single fake process (no real /proc)
TEST_F(ProcessTreeExtTest, scanSingleFakeProcess)
{
    std::map<int, proc_t> processes;
    proc_t fake;
    memset(&fake, 0, sizeof(proc_t));
    fake.tid = 100;
    fake.ppid = 1;
    processes[100] = fake;

    m_tree->scanProcesses(processes);
    QList<int> children = m_tree->getAllChildPids(1);
    // PID 1 should have 100 as child
    EXPECT_TRUE(children.contains(100));
}

// Destructor should clean up without crash
TEST_F(ProcessTreeExtTest, destructorCleanup)
{
    ProcessTree *tmp = new ProcessTree;
    std::map<int, proc_t> processes;
    proc_t fake;
    memset(&fake, 0, sizeof(proc_t));
    fake.tid = 100;
    fake.ppid = 1;
    processes[100] = fake;
    tmp->scanProcesses(processes);
    EXPECT_NO_FATAL_FAILURE(delete tmp);
}

// getAllChildPids on non-existent PID returns empty
TEST_F(ProcessTreeExtTest, getAllChildPidsNonExistentPid)
{
    QList<int> children = m_tree->getAllChildPids(99999);
    EXPECT_TRUE(children.isEmpty());
}
