// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>

#include "eventlogutils.h"
#include "log.h"

EventLogUtils *EventLogUtils::m_instance(nullptr);

EventLogUtils &EventLogUtils::get()
{
    qCDebug(dsrApp) << "Getting EventLogUtils instance.";
    if (m_instance == nullptr) {
        qCDebug(dsrApp) << "Creating new EventLogUtils instance.";
        m_instance = new EventLogUtils;
    }
    qCDebug(dsrApp) << "Returning EventLogUtils instance.";
    return *m_instance;
}

EventLogUtils::EventLogUtils()
{
    qCDebug(dsrApp) << "EventLogUtils constructor called.";
    QLibrary library("libdeepin-event-log.so");

    init = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (init == nullptr) {
        qCDebug(dsrApp) << "Initialize function not resolved. Event logging will be disabled.";
        return;
    }
    qCDebug(dsrApp) << "Initialize function resolved. Initializing event log with 'deepin-screen-recorder'.";
    init("deepin-screen-recorder", true);
}

void EventLogUtils::writeLogs(QJsonObject &data)
{
    qCDebug(dsrApp) << "Attempting to write event logs.";
    if (writeEventLog == nullptr) {
        qCDebug(dsrApp) << "writeEventLog function not resolved. Skipping log write.";
        return;
    }

    //std::string str = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString();
    std::string logString = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString();
    writeEventLog(logString);
    qCDebug(dsrApp) << "Event log written: " << QString::fromStdString(logString);
}
