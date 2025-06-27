// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "savebutton.h"
#include "../utils/baseutils.h"
#include "../utils/log.h"
#include <QHBoxLayout>

const QSize TOOL_SAVE_BTN = QSize(32, 22);
const QSize SAVE_BTN = QSize(21, 22);
const QSize LIST_BTN = QSize(11, 22);

SaveButton::SaveButton(DWidget *parent)
    : DPushButton(parent)
{
    qCDebug(dsrApp) << "SaveButton constructor entered";
    setFixedSize(TOOL_SAVE_BTN);
//    setStyleSheet(getFileContent(":/resources/qss/toolsavebutton.qss"));
    m_saveBtn = new ToolButton(this);
    m_saveBtn->setObjectName("SaveBtn");
    m_saveBtn->setFixedSize(SAVE_BTN);
    m_listBtn = new ToolButton(this);
    m_listBtn->setObjectName("ListBtn");
    m_listBtn->setFixedSize(LIST_BTN);

    QHBoxLayout *saveLayout = new QHBoxLayout();
    saveLayout->setContentsMargins(0, 0, 0, 0);
    saveLayout->setSpacing(0);
    saveLayout->addWidget(m_saveBtn);
    saveLayout->addWidget(m_listBtn);

    setLayout(saveLayout);

    connect(m_saveBtn, &ToolButton::clicked, this,
            &SaveButton::saveAction);
    qCDebug(dsrApp) << "Connected saveBtn clicked to saveAction";
    connect(m_listBtn, &ToolButton::clicked, this, [ = ]() {
        bool isChecked = m_listBtn->isChecked();
        qCDebug(dsrApp) << "List button clicked, checked status:" << isChecked;
        emit  expandSaveOption(isChecked);
    });
}

SaveButton::~SaveButton()
{
    qCDebug(dsrApp) << "SaveButton destructor entered";
}
