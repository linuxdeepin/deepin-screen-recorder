// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textbutton.h"
#include "../utils/baseutils.h"
#include "../utils/configsettings.h"
#include "../utils/log.h"

TextButton::TextButton(int num, DWidget *parent)
    : DPushButton(parent)
{
    qCDebug(dsrApp) << "TextButton constructor called with font size:" << num;
    setObjectName("TextButton");
    m_fontsize = num;
    setText(QString("%1").arg(m_fontsize));

    setFixedSize(24, 26);
    setCheckable(true);

    connect(this, &TextButton::clicked, this, [ = ] {
        qCDebug(dsrApp) << "TextButton clicked, checked:" << this->isChecked();
        if (this->isChecked())
        {
            qCDebug(dsrApp) << "Text font size changed to:" << m_fontsize;
            ConfigSettings::instance()->setValue("text", "fontsize", m_fontsize);
        }
    });
}

TextButton::~TextButton() {
    qCDebug(dsrApp) << "TextButton destructor called.";
}
