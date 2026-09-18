// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include "utils.h"
#include "record_option_panel.h"
#include "constant.h"
#include <DHiDPIHelper>
#include "./utils/log.h"

const int RecordOptionPanel::WIDTH = 124;
const int RecordOptionPanel::HEIGHT = 36;
const int RecordOptionPanel::ICON_OFFSET_X = 14;

DWIDGET_USE_NAMESPACE

RecordOptionPanel::RecordOptionPanel(DPushButton *parent) : DPushButton(parent)
{
    qCDebug(dsrApp) << "RecordOptionPanel constructor called.";
    installEventFilter(this);
    qCDebug(dsrApp) << "Event filter installed.";
    setMouseTracking(true);
    qCDebug(dsrApp) << "Mouse tracking enabled.";

    gifNormalImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("gif_normal.svg"));
    gifPressImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("gif_press.svg"));
    gifCheckedImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("gif_checked.svg"));
    qCDebug(dsrApp) << "GIF images loaded.";

    videoNormalImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("video_normal.svg"));
    videoPressImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("video_press.svg"));
    videoCheckedImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("video_checked.svg"));
    qCDebug(dsrApp) << "Video images loaded.";

    settings = new Settings();
    qCDebug(dsrApp) << "Settings object created.";
    saveAsGif = settings->getOption("save_as_gif").toBool();
    qCDebug(dsrApp) << "Initial saveAsGif setting:" << saveAsGif;

    isPressGif = false;
    isPressVideo = false;
    qCDebug(dsrApp) << "Initial press states set to false.";

    setFixedSize(WIDTH, HEIGHT);
    qCDebug(dsrApp) << "Fixed size set for panel:" << size();
}

RecordOptionPanel::~RecordOptionPanel()
{
    qCDebug(dsrApp) << "RecordOptionPanel destructor called.";
    if (settings) {
        delete settings;
        settings = nullptr;
        qCDebug(dsrApp) << "Settings object deleted.";
    }
}

void RecordOptionPanel::paintEvent(QPaintEvent *)
{
    qCDebug(dsrApp) << "paintEvent called.";
    // Init.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    qCDebug(dsrApp) << "Painter initialized with antialiasing.";

    // Draw background.
    Utils::drawTooltipBackground(painter, rect());
    qCDebug(dsrApp) << "Tooltip background drawn.";

    // Draw icon.
    qreal devicePixelRatio = qApp->devicePixelRatio();
    painter.setOpacity(1);
    int gifIconX = static_cast<int>(rect().x() + ICON_OFFSET_X / devicePixelRatio);
    int gifIconY = static_cast<int>(rect().y() + (rect().height() - gifCheckedImg.height() / devicePixelRatio) / 2);
    int gifTextX = static_cast<int>(gifIconX + gifCheckedImg.width() / devicePixelRatio);
    int gifTextWidth = static_cast<int>(rect().width() / 2 - ICON_OFFSET_X / devicePixelRatio - gifCheckedImg.width() / devicePixelRatio);
    QString gifColor;
    if (saveAsGif) {
        painter.drawPixmap(QPoint(gifIconX, gifIconY), gifCheckedImg);
        gifColor = "#217DFF";
        qCDebug(dsrApp) << "Drawing checked GIF icon.";
    } else if (isPressGif) {
        painter.drawPixmap(QPoint(gifIconX, gifIconY), gifPressImg);
        gifColor = "#004BCA";
        qCDebug(dsrApp) << "Drawing pressed GIF icon.";
    } else {
        painter.drawPixmap(QPoint(gifIconX, gifIconY), gifNormalImg);
        gifColor = "#000000";
        qCDebug(dsrApp) << "Drawing normal GIF icon.";
    }
    Utils::drawTooltipText(painter, "GIF", gifColor, 9, QRectF(gifTextX, rect().y(), gifTextWidth, rect().height()));
    qCDebug(dsrApp) << "Drawing GIF text.";

    int videoIconX = static_cast<int>(rect().x() + rect().width() / 2);
    int videoIconY = static_cast<int>(rect().y() + (rect().height() - videoCheckedImg.height() / devicePixelRatio) / 2);
    int videoTextX = static_cast<int>(videoIconX + videoCheckedImg.width() / devicePixelRatio);
    int videoTextWidth = static_cast<int>(rect().width() / 2 - ICON_OFFSET_X / devicePixelRatio - gifCheckedImg.width() / devicePixelRatio);
    QString videoColor;
    if (!saveAsGif) {
        painter.drawPixmap(QPoint(videoIconX, videoIconY), videoCheckedImg);
        videoColor = "#217DFF";
        qCDebug(dsrApp) << "Drawing checked Video icon.";
    } else if (isPressVideo) {
        painter.drawPixmap(QPoint(videoIconX, videoIconY), videoPressImg);
        videoColor = "#004BCA";
        qCDebug(dsrApp) << "Drawing pressed Video icon.";
    } else {
        painter.drawPixmap(QPoint(videoIconX, videoIconY), videoNormalImg);
        videoColor = "#000000";
        qCDebug(dsrApp) << "Drawing normal Video icon.";
    }
    Utils::drawTooltipText(painter, "MP4", videoColor, 9, QRectF(videoTextX, rect().y(), videoTextWidth, rect().height()));
    qCDebug(dsrApp) << "Drawing MP4 text.";
}

bool RecordOptionPanel::eventFilter(QObject *, QEvent *event)
{
    qCDebug(dsrApp) << "eventFilter called for event type:" << event->type();
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        int pressX = mouseEvent->x();
        qCDebug(dsrApp) << "MouseButtonPress event at X:" << pressX;

        if (pressX > rect().x() && pressX < rect().x() + rect().width() / 2) {
            isPressGif = true;
            isPressVideo = false;
            qCDebug(dsrApp) << "GIF button pressed.";
        } else {
            isPressVideo = true;
            isPressGif = false;
            qCDebug(dsrApp) << "Video button pressed.";
        }

        repaint();
        qCDebug(dsrApp) << "Repaint triggered after mouse press.";
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        int releaseX = mouseEvent->x();
        qCDebug(dsrApp) << "MouseButtonRelease event at X:" << releaseX;

        if (releaseX > rect().x() && releaseX < rect().x() + rect().width() / 2) {
            if (isPressGif) {
                saveAsGif = true;
                settings->setOption("save_as_gif", saveAsGif);
                qCDebug(dsrApp) << "GIF selected. saveAsGif set to true.";
            } else if (isPressVideo) {
                isPressVideo = false;
                qCDebug(dsrApp) << "Video button was pressed but GIF area clicked, resetting isPressVideo.";
            }
        } else {
            if (isPressVideo) {
                saveAsGif = false;
                settings->setOption("save_as_gif", saveAsGif);
                qCDebug(dsrApp) << "MP4 selected. saveAsGif set to false.";
            } else if (isPressGif) {
                isPressGif = false;
                qCDebug(dsrApp) << "GIF button was pressed but Video area clicked, resetting isPressGif.";
            }
        }

        repaint();
        qCDebug(dsrApp) << "Repaint triggered after mouse release.";
    }

    return false;
}
/*
 * never used
bool RecordOptionPanel::isSaveAsGif()
{
    return saveAsGif;
}
*/
