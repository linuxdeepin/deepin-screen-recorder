/**
 * Copyright (C) 2020 UOS Technology Co., Ltd.
 *
 * to mark the desktop UI
 **/

#ifndef SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H
#define SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H

#include "accessiblefunctions.h"

// 添加accessible

SET_FORM_ACCESSIBLE(QWidget,m_w->objectName())

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
    USE_ACCESSIBLE(classname, QWidget);

    return interface;
}

#endif // SHOT_RECORDER_ACCESSIBLE_OBJECT_LIST_H
