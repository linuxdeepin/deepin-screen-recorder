// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toptips.h"
#include "../utils/baseutils.h"
#include "../utils/log.h"

#include <DPalette>
#include <QImage>
#include <QSizePolicy>
#include <QDebug>
#include <QApplication>
#include <QScreen>

#include <cmath>

DGUI_USE_NAMESPACE

TopTips::TopTips(DWidget *parent)
    : QLabel(parent)
{
    qCDebug(dsrApp) << "TopTips constructor called.";
    setFixedSize(90, 30);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
//    this->setStyleSheet(" TopTips { background-color: transparent;"
//                        "border-image: url(:/resources/images/action/sizetip.png)  no-repeat;"
//                        "color: white;"
//                        "font-size: 12px;}");
//    this->setPixmap(QPixmap::fromImage(QImage(":/resources/images/action/sizetip.png")));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    DPalette pa = this->palette();
    pa.setColor(DPalette::WindowText, Qt::white);
    this->setPalette(pa);
    this->setAlignment(Qt::AlignLeft | Qt::AlignBottom);


}

void TopTips::setContent(const QSize &size)
{
    qCDebug(dsrApp) << "TopTips::setContent called with size:" << size;
    QString text = QString("%1X%2").arg(size.width()).arg(size.height());
    if(m_showRecorderTips && size.width() * size.height() > 1920 * 1080 && size.width() != m_width && size.height() != m_height) {
        // 1920 / 1080 = w / h
        // w h 等比缩放
        setFixedSize(500, 30);
        int h = static_cast<int>(sqrt(1920.0 * 1080 * size.height() / size.width()));
        int w = static_cast<int>(sqrt(1920.0 * 1080 * size.width() / size.height()));
        QString recorderTips = tr(" Adjust the recording area within %1*%2 to get better video effect");
        qCDebug(dsrApp) << "Setting content with recording area adjustment tip:" << w << "x" << h;
        setText(text + recorderTips.arg(w).arg(h));
    } else {
        qCDebug(dsrApp) << "Setting content size:" << size;
        setText(text);
    }
}

void TopTips::updateTips(QPoint pos, const QSize &size)
{
    qCDebug(dsrApp) << "TopTips::updateTips called with position:" << pos << "and size:" << size;
    if(m_isFullScreenRecord) {
        qCDebug(dsrApp) << "Skip updating tips in full screen record mode";
        return;
    }
    if (!this->isVisible())
        this->show();

    QPoint startPoint = pos;
    qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QSize s = QSize(static_cast<int>(size.width()*pixelRatio),static_cast<int>(size.height()*pixelRatio));
    setContent(s);
    startPoint.setX(pos.x());

    if (pos.y() > this->height()) {
        qCDebug(dsrApp) << "Adjusting tip Y position: above current position.";
        startPoint.setY(pos.y() - this->height() - 3);
    } else {
        qCDebug(dsrApp) << "Adjusting tip Y position: below current position.";
        startPoint.setY(pos.y() + 3);
    }

    qCDebug(dsrApp) << "Updating tips position to:" << startPoint;
    this->move(startPoint);
}

/*
void TopTips::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
    qDebug() << "";
    return;
}
*/

TopTips::~TopTips() {
    qCDebug(dsrApp) << "TopTips destructor called.";
}

void TopTips::setRecorderTipsInfo(const bool showState)
{
    qCDebug(dsrApp) << "TopTips::setRecorderTipsInfo called with showState:" << showState;
    if(QSysInfo::currentCpuArchitecture().startsWith(QString("mips"))){
        qCDebug(dsrApp) << "CPU architecture is MIPS, setting m_showRecorderTips to:" << showState;
        m_showRecorderTips = showState;
    }
}

void TopTips::setFullScreenRecord(const bool flag)
{
    qCDebug(dsrApp) << "TopTips::setFullScreenRecord called with flag:" << flag;
    m_isFullScreenRecord = flag;
}
