// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "savetips.h"
#include "../utils/baseutils.h"
#include "../utils/log.h"
#include <QDebug>

SaveTips::SaveTips(DWidget *parent)
    : DLabel(parent)
{
//    setStyleSheet(getFileContent(":/resources/qss/savetips.qss"));
    setTipWidth(0);
    setFixedWidth(0);
    m_startAni = new QPropertyAnimation(this, "tipWidth");
    m_stopAni = new QPropertyAnimation(this, "tipWidth");

    connect(m_startAni, &QPropertyAnimation::valueChanged, [ = ](QVariant value) {
        emit tipWidthChanged(std::max(value.toInt(), this->width()));
        setFixedWidth(value.toInt());
    });
    connect(m_stopAni, &QPropertyAnimation::valueChanged, [ = ](QVariant value) {
        emit tipWidthChanged(std::max(value.toInt(), this->width()));
        setFixedWidth(value.toInt());
    });
    connect(m_stopAni, &QPropertyAnimation::finished, this, [ = ] {
        qCDebug(dsrApp) << "Stop animation finished, clearing text";
        this->clear();
        m_text = "";
    });
}

void SaveTips::setSaveText(QString text)
{
    qCDebug(dsrApp) << "Setting save tip text:" << text;
    m_text = text;
    setTipWidth(stringWidth(this->font(), m_text) + 10);
//   setText(text);
}

int SaveTips::tipWidth() const
{
    return  this->width();
}

void SaveTips::setTipWidth(int tipsWidth)
{
    qCDebug(dsrApp) << "Setting tip width to:" << tipsWidth;
    m_tipsWidth = tipsWidth;
}

SaveTips::~SaveTips()
{
}

void SaveTips::startAnimation()
{
    m_stopAni->stop();
    m_startAni->stop();
    m_startAni->setDuration(220);
    m_startAni->setStartValue(this->width());
    m_startAni->setEasingCurve(QEasingCurve::OutSine);
    m_startAni->setEndValue(m_tipsWidth);
    m_startAni->start();
    setText(m_text);
}

void SaveTips::endAnimation()
{
    m_stopAni->setDuration(220);
    m_stopAni->setStartValue(m_tipsWidth);
    m_stopAni->setEndValue(0);
    m_stopAni->setEasingCurve(QEasingCurve::OutSine);

    m_stopAni->start();
}
