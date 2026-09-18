// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "process_tree.h"
#include <QDebug>
#include "./utils/log.h"

ProcessTree::ProcessTree()
{
    qCDebug(dsrApp) << "ProcessTree constructor called.";
    processMap = new QMap<int, Process>();
    qCDebug(dsrApp) << "Process map created.";

    rootPid = INT_MAX;
    qCDebug(dsrApp) << "Root PID initialized to INT_MAX.";
}

ProcessTree::~ProcessTree()
{
    qCDebug(dsrApp) << "ProcessTree destructor called.";
    childrenPids.clear();
    qCDebug(dsrApp) << "childrenPids cleared.";

    for (auto v : processMap->values()) {
        v.childProcesses.clear();
    }
    qCDebug(dsrApp) << "Child processes cleared for all processes in map.";

    processMap->clear();
    qCDebug(dsrApp) << "Process map cleared.";
    delete processMap;
    qCDebug(dsrApp) << "Process map deleted.";
}

void ProcessTree::scanProcesses(StoredProcType processes)
{
    qCDebug(dsrApp) << "Scanning processes, number of processes:" << processes.size();
    for (auto &i : processes) {
        int ppid = (&i.second)->ppid;
        int pid = (&i.second)->tid;
        qCDebug(dsrApp) << "Processing PID:" << pid << ", PPID:" << ppid;

        if (!processMap->contains(pid)) {
            Process process;

            process.parentPid = ppid;

            (*processMap)[pid] = process;
            qCDebug(dsrApp) << "Added new process to map:" << pid;
        }

        if (!processMap->contains(ppid)) {
            Process process;

            process.parentPid = -1;
            process.childProcesses.append(pid);

            (*processMap)[ppid] = process;
            qCDebug(dsrApp) << "Added new parent process to map:" << ppid << ", with child:" << pid;
        } else {
            (*processMap)[ppid].childProcesses.append(pid);
            qCDebug(dsrApp) << "Appended child:" << pid << "to existing parent:" << ppid;
        }

        if (pid < rootPid) {
            rootPid = pid;
            qCDebug(dsrApp) << "Updated root PID to:" << rootPid;
        }
    }
    qCDebug(dsrApp) << "Process scanning completed.";
}

QList<int> ProcessTree::getAllChildPids(int pid)
{
    qCDebug(dsrApp) << "Getting all child PIDs for PID:" << pid;
    childrenPids.clear();
    qCDebug(dsrApp) << "childrenPids cleared before retrieval.";

    getChildPids(pid);
    qCDebug(dsrApp) << "getChildPids called for PID:" << pid;

    return childrenPids;
}

void ProcessTree::getChildPids(int pid)
{
    qCDebug(dsrApp) << "Recursively getting child PIDs for PID:" << pid;
    if (processMap->contains(pid)) {
        qCDebug(dsrApp) << "Process map contains PID:" << pid;
        for (int childPid : ((*processMap)[pid].childProcesses)) {
            childrenPids.append(childPid);
            qCDebug(dsrApp) << "Appended child PID:" << childPid;

            getChildPids(childPid);
        }
    } else {
        qCDebug(dsrApp) << "Process map does not contain PID:" << pid;
    }
}
/*
 * never used
void ProcessTree::printNode(int pid)
{
    qDebug() << "* Node " << pid;

    if (processMap->contains(pid)) {
        qDebug() << "### Child " << (*processMap)[pid].childProcesses;

        for (int childPid : ((*processMap)[pid].childProcesses)) {
            printNode(childPid);
        }
    }
}
void ProcessTree::printTree()
{
    printNode(rootPid);
}
*/
