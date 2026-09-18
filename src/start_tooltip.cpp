// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DWidget>
#include <QPainter>
#include <QScreen>
#include <QApplication>
#include <QStyle>
#include <QEvent>
#include "start_tooltip.h"
#include "utils.h"
#include "constant.h"
#include <DHiDPIHelper>

#include "utils/log.h"

DWIDGET_USE_NAMESPACE

StartTooltip::StartTooltip(DWidget *parent) : DWidget(parent)
{
    qCDebug(dsrApp) << "StartTooltip constructor called.";
    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::BypassWindowManagerHint);
    qCDebug(dsrApp) << "Window flags set.";
    setAttribute(Qt::WA_TranslucentBackground, true);
    qCDebug(dsrApp) << "Translucent background attribute set.";

    iconImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("deepin-screen-recorder.svg"));
    qCDebug(dsrApp) << "Icon image loaded.";

    installEventFilter(this);
    qCDebug(dsrApp) << "Event filter installed.";

    text = tr("Click or drag to\nselect the area to record");
    QSize size = Utils::getRenderSize(Constant::RECTANGLE_FONT_SIZE, text);
    qCDebug(dsrApp) << "Tooltip text set and render size calculated.";

    qreal devicePixelRatio = qApp->devicePixelRatio();
    setFixedSize(size.width() + Constant::RECTANGLE_PADDING * 2,
                 size.height() + iconImg.height() / devicePixelRatio + Constant::RECTANGLE_PADDING * 3);
    qCDebug(dsrApp) << "Fixed size set for tooltip. Size:" << size.width() + Constant::RECTANGLE_PADDING * 2 << "x" << size.height() + iconImg.height() / devicePixelRatio + Constant::RECTANGLE_PADDING * 3 << ".";

    Utils::passInputEvent(this->winId());
    qCDebug(dsrApp) << "Input events passed for window ID:" << this->winId() << ".";
}

void StartTooltip::setWindowManager(DWindowManager *wm)
{
    qCDebug(dsrApp) << "setWindowManager() called.";
    windowManager = wm;
    qCDebug(dsrApp) << "Window manager set.";

    const qreal ratio = devicePixelRatioF();
    WindowRect rootWindowRect = windowManager->getRootWindowRect();
    qCDebug(dsrApp) << "Device pixel ratio:" << ratio << ", Root window rect:" << rootWindowRect.x << "," << rootWindowRect.y << "," << rootWindowRect.width << "," << rootWindowRect.height << ".";
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            QRect(rootWindowRect.x * ratio, rootWindowRect.y * ratio, rootWindowRect.width, rootWindowRect.height)
        )
    );
    qCDebug(dsrApp) << "Tooltip geometry set.";
}

void StartTooltip::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    qCDebug(dsrApp) << "paintEvent() called.";
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    qCDebug(dsrApp) << "Painter render hint set to Antialiasing.";

    Utils::drawTooltipBackground(painter, rect());
    qCDebug(dsrApp) << "Tooltip background drawn.";

    const qreal devicePixelRatio = devicePixelRatioF();
    painter.setOpacity(1);
    painter.drawPixmap(QPoint((rect().width() - iconImg.width() / devicePixelRatio) / 2, Constant::RECTANGLE_PADDING / devicePixelRatio), iconImg);
    qCDebug(dsrApp) << "Icon image drawn.";

    Utils::drawTooltipText(painter, text, "#000000", Constant::RECTANGLE_FONT_SIZE,
                           QRectF(rect().x() * devicePixelRatio,
                                  rect().y() * devicePixelRatio + Constant::RECTANGLE_PADDING / devicePixelRatio + iconImg.height() / devicePixelRatio,
                                  rect().width(),
                                  rect().height() - Constant::RECTANGLE_PADDING / devicePixelRatio - iconImg.height() / devicePixelRatio
                                 ));
    qCDebug(dsrApp) << "Tooltip text drawn.";
}

bool StartTooltip::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    qCDebug(dsrApp) << "eventFilter() called for event type:" << event->type() << ".";
    if (event->type() == QEvent::ShowToParent) {
        Utils::blurRect(windowManager, this->winId(), rect());
        qCDebug(dsrApp) << "Blur effect applied for QEvent::ShowToParent.";
    } else if (event->type() == QEvent::HideToParent) {
        Utils::clearBlur(windowManager, this->winId());
        qCDebug(dsrApp) << "Blur effect cleared for QEvent::HideToParent.";
    }

    qCDebug(dsrApp) << "Event filter returning false.";
    return false;
}
