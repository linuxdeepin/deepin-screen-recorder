// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QPainter>
#include <QEvent>
#include <QApplication>
#include "record_button.h"
#include "utils.h"
#include "constant.h"
#include <DHiDPIHelper>
#include <DIconButton>
#include "./utils/log.h"

DWIDGET_USE_NAMESPACE

const int RecordButton::WIDTH = 128;
const int RecordButton::HEIGHT = 86;
const int RecordButton::TEXT_PADDING = 0;

RecordButton::RecordButton(DPushButton *parent) : DPushButton(parent)
{
    qCDebug(dsrApp) << "RecordButton constructor called.";
    installEventFilter(this);  // add event filter
    qCDebug(dsrApp) << "Event filter installed.";
    setMouseTracking(true);
    qCDebug(dsrApp) << "Mouse tracking enabled.";

    isFocus = false;
    isPress = false;
    qCDebug(dsrApp) << "Initial focus and press states set to false.";

    normalImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("record_icon_normal.svg"));
    hoverImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("record_icon_hover.svg"));
    pressImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("record_icon_press.svg"));
    qCDebug(dsrApp) << "Button images loaded.";
}

void RecordButton::setText(QString string)
{
    qCDebug(dsrApp) << "Setting button text to:" << string;
    text = string;
    textSize = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, string);
    qCDebug(dsrApp) << "Text size calculated:" << textSize;

    setFixedSize(std::max(textSize.width() + Constant::RECTANGLE_PADDING * 2, WIDTH), std::max(textSize.height() + Constant::RECTANGLE_PADDING * 2, HEIGHT));
    qCDebug(dsrApp) << "Fixed size set for button:" << size();
}

void RecordButton::paintEvent(QPaintEvent *)
{
    qCDebug(dsrApp) << "paintEvent called.";
    // Init.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QString status = "NORMAL";
    if (isFocus) {
        if (isPress) {
            status = "PRESS";
            qCDebug(dsrApp) << "Button status: PRESS.";
        } else {
            status = "HOVER";
            qCDebug(dsrApp) << "Button status: HOVER.";
        }
    } else {
        qCDebug(dsrApp) << "Button status: NORMAL.";
    }

    // Draw background.
    qreal backgroundOpacity = 1.0;
    if (status == "NORMAL") {
        backgroundOpacity = 0.6;
        qCDebug(dsrApp) << "Background opacity for NORMAL status:" << backgroundOpacity;
    } else if (status == "PRESS") {
        backgroundOpacity = 0.2;
        qCDebug(dsrApp) << "Background opacity for PRESS status:" << backgroundOpacity;
    } else if (status == "HOVER") {
        backgroundOpacity = 0.7;
        qCDebug(dsrApp) << "Background opacity for HOVER status:" << backgroundOpacity;
    }

    Utils::drawTooltipBackground(painter, rect(), backgroundOpacity);
    qCDebug(dsrApp) << "Tooltip background drawn with opacity:" << backgroundOpacity;

    // Draw icon.
    qreal devicePixelRatio = qApp->devicePixelRatio();
    painter.setOpacity(1);
    int iconX = static_cast<int>(rect().x() + (rect().width() - normalImg.width() / devicePixelRatio) / 2);
    int iconY = static_cast<int>(rect().y() + (rect().height() - normalImg.height() / devicePixelRatio - textSize.height() - TEXT_PADDING) / 2);
    if (status == "NORMAL") {
        painter.drawPixmap(QPoint(iconX, iconY), normalImg);
        qCDebug(dsrApp) << "Drawing normal icon at:" << QPoint(iconX, iconY);
    } else if (status == "PRESS") {
        painter.drawPixmap(QPoint(iconX, iconY), pressImg);
        qCDebug(dsrApp) << "Drawing press icon at:" << QPoint(iconX, iconY);
    } else if (status == "HOVER") {
        painter.drawPixmap(QPoint(iconX, iconY), hoverImg);
        qCDebug(dsrApp) << "Drawing hover icon at:" << QPoint(iconX, iconY);
    }

    // Draw text.
    int textX = rect().x();
    int textY = static_cast<int>(iconY + normalImg.height() / devicePixelRatio + TEXT_PADDING);
    Utils::drawTooltipText(painter, text, "#e34342", Constant::RECTANGLE_FONT_SIZE, QRect(textX, textY, rect().width(), textSize.height()));
    qCDebug(dsrApp) << "Drawing text:" << text << "at:" << QRect(textX, textY, rect().width(), textSize.height());
}

bool RecordButton::eventFilter(QObject *, QEvent *event)
{
    qCDebug(dsrApp) << "eventFilter called for event type:" << event->type();
    if (event->type() == QEvent::MouseButtonPress) {
        isPress = true;
        repaint();
        qCDebug(dsrApp) << "MouseButtonPress event: isPress set to true, repaint triggered.";
    } else if (event->type() == QEvent::MouseButtonRelease) {
        isPress = false;
        repaint();
        qCDebug(dsrApp) << "MouseButtonRelease event: isPress set to false, repaint triggered.";
    } else if (event->type() == QEvent::Enter) {
        isFocus = true;
        repaint();
        qCDebug(dsrApp) << "Enter event: isFocus set to true, repaint triggered.";
    } else if (event->type() == QEvent::Leave) {
        isFocus = false;
        repaint();
        qCDebug(dsrApp) << "Leave event: isFocus set to false, repaint triggered.";
    }

    return false;
}
