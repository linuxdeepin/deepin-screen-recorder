// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainToolWidget.h"
#include "accessibility/acTextDefine.h"
#include "../../utils/log.h"

MainToolWidget::MainToolWidget(DWidget *parent) : DWidget(parent)
{
    qCDebug(dsrApp) << "MainToolWidget constructor called.";
    initMainToolWidget();
    qCDebug(dsrApp) << "initMainToolWidget finished.";
}

void MainToolWidget::initMainToolWidget()
{
    qCDebug(dsrApp) << "Initializing MainToolWidget.";
    m_closeButton = new ToolButton(this);
    qCDebug(dsrApp) << "Close button created.";
    m_closeButton->setObjectName(AC_TOOLBARWIDGET_CLOSE_PIN_BUT);
    m_closeButton->setAccessibleName(AC_TOOLBARWIDGET_CLOSE_PIN_BUT);
    m_closeButton->setIcon(QIcon::fromTheme("close"));
    m_closeButton->setFixedSize(QSize(32, 32));
    m_closeButton->setIconSize(QSize(32, 32));
    connect(m_closeButton, SIGNAL(clicked()), this, SIGNAL(signalCloseButtonClicked()));
    qCDebug(dsrApp) << "Close button connected.";

    m_saveButton = new ToolButton(this);
    qCDebug(dsrApp) << "Save button created.";
    m_saveButton->setObjectName(AC_MAINWINDOW_PIN_SAVE_BUT);
    m_saveButton->setAccessibleName(AC_MAINWINDOW_PIN_SAVE_BUT);
    m_saveButton->setIcon(QIcon::fromTheme("confirm"));
    m_saveButton->setFixedSize(QSize(32, 32));
    m_saveButton->setIconSize(QSize(32, 32));
    connect(m_saveButton, SIGNAL(clicked()), this, SIGNAL(signalSaveButtonClicked()));
    qCDebug(dsrApp) << "Save button connected.";

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    qCDebug(dsrApp) << "Horizontal layout created.";
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(m_closeButton, 0, Qt::AlignCenter);
    hLayout->addWidget(m_saveButton, 0, Qt::AlignCenter);
    this->setLayout(hLayout);
    qCDebug(dsrApp) << "Layout set.";
}
