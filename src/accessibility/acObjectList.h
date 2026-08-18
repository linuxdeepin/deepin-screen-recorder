// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H
#define SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H

#include <QAccessible>
#include <QString>

class QObject;
class RecordButton;
class RecordOptionPanel;

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object);

#endif // SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H
