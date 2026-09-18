// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "acObjectList.h"
#include "accessiblefunctions.h"
#include "../record_button.h"
#include "../record_option_panel.h"

SET_FORM_ACCESSIBLE(QWidget, m_w->objectName())
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
