// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontsizewidget.h"

#include <QHBoxLayout>
#include <QDebug>

#include "../utils/baseutils.h"
#include "../utils/configsettings.h"
#include "../utils/log.h"

const QSize BUTTON_SIZE = QSize(20, 16);
const QSize LINE_EDIT_SIZE = QSize(43, 16);

Separator::Separator(DWidget *parent)
    : DLabel(parent)
{
    qCDebug(dsrApp) << "Separator constructor entered";
    setFixedSize(1, 16);
}

Separator::~Separator()
{
    qCDebug(dsrApp) << "Separator destructor entered";
}

FontSizeWidget::FontSizeWidget(DWidget *parent)
    : DLabel(parent)
{
    qCDebug(dsrApp) << "FontSizeWidget constructor entered";
    initWidget();
}

void FontSizeWidget::initWidget()
{
    qCDebug(dsrApp) << "Initializing FontSizeWidget UI";
    setObjectName("FontSizeWidget");
//    setStyleSheet(getFileContent(":/resources/qss/fontsizewidget.qss"));
    setFixedSize(86, 18);
    m_fontSizeEdit = new DLineEdit(this);
    m_fontSizeEdit->setObjectName("FontSizeEdit");
    m_fontSizeEdit->setFixedSize(LINE_EDIT_SIZE);

    m_fontSize = ConfigSettings::instance()->value("text", "fontsize").toInt();
    qCDebug(dsrApp) << "Retrieved initial font size:" << m_fontSize;
    m_fontSizeEdit->setText(QString("%1").arg(m_fontSize));
    m_addSizeBtn = new DPushButton(this);
    m_addSizeBtn->setObjectName("AddSizeBtn");
    m_addSizeBtn->setFixedSize(BUTTON_SIZE);
    m_reduceSizeBtn = new DPushButton(this);
    m_reduceSizeBtn->setObjectName("ReduceSizeBtn");
    m_reduceSizeBtn->setFixedSize(BUTTON_SIZE);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addSpacing(4);
    layout->addWidget(m_fontSizeEdit);
    layout->addSpacing(0);
    layout->addWidget( new Separator(this));
    layout->addWidget(m_addSizeBtn);
    layout->addWidget( new Separator(this));
    layout->addSpacing(0);
    layout->addWidget(m_reduceSizeBtn);
    layout->addStretch();
    setLayout(layout);

    connect(m_addSizeBtn, &DPushButton::clicked, [ = ] {
        qCDebug(dsrApp) << "Font size increase button clicked";
        adjustFontSize(true);
    });
    connect(m_reduceSizeBtn, &DPushButton::clicked, [ = ] {
        qCDebug(dsrApp) << "Font size decrease button clicked";
        adjustFontSize(false);
    });
}

void FontSizeWidget::setFontSize(int fontSize)
{
    qCDebug(dsrApp) << "setFontSize called with:" << fontSize;
    m_fontSize = fontSize;
}

void FontSizeWidget::adjustFontSize(bool add)
{
    qCDebug(dsrApp) << "adjustFontSize called with add:" << add;
    if (add) {
        m_fontSize = m_fontSize + 1;
        m_fontSize = std::min(m_fontSize, 72);
        qCDebug(dsrApp) << "Increased font size to:" << m_fontSize;
    } else {
        m_fontSize = m_fontSize - 1;
        m_fontSize = std::max(9, m_fontSize);
        qCDebug(dsrApp) << "Decreased font size to:" << m_fontSize;
    }

    qCDebug(dsrApp) << "Adjusted font size to:" << m_fontSize;
    m_fontSizeEdit->setText(QString("%1").arg(m_fontSize));
    emit fontSizeChanged(m_fontSize);

    connect(this, &FontSizeWidget::fontSizeChanged, this, [ = ](int fontSize) {
        qCDebug(dsrApp) << "Saving font size to config:" << fontSize;
        ConfigSettings::instance()->setValue("text", "fontsize", fontSize);
    });
}

FontSizeWidget::~FontSizeWidget()
{
    qCDebug(dsrApp) << "FontSizeWidget destructor entered";
}
