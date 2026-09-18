// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrinterface.h"
#include "../../utils/log.h"
#include <QDBusMetaType>

OcrInterface::OcrInterface(const QString &serviceName, const QString &ObjectPath,
                           const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(serviceName, ObjectPath, staticInterfaceName(), connection, parent)
{
    qCDebug(dsrApp) << "Initializing OCR interface with service:" << serviceName << "path:" << ObjectPath;
}

OcrInterface::~OcrInterface()
{
    qCDebug(dsrApp) << "Destroying OCR interface";
}
