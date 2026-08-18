// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H
#define SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H

#include "accessiblefunctions.h"

// 添加accessible

SET_FORM_ACCESSIBLE(QWidget,m_w->objectName())
SET_BUTTON_ACCESSIBLE(RecordButton, "RecordButton")
SET_BUTTON_ACCESSIBLE(RecordOptionPanel, "RecordOptionPanel")

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
    USE_ACCESSIBLE(classname, QWidget);
    USE_ACCESSIBLE(classname, RecordButton);
    USE_ACCESSIBLE(classname, RecordOptionPanel);

    return interface;
}

#endif // SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H
