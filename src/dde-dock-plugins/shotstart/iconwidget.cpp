// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconwidget.h"
#include "dde-dock/constants.h"

#include <DGuiApplicationHelper>
#include <DStyle>

#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QPixmap>
#include <QThread>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QPainterPath>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

IconWidget::IconWidget(QWidget *parent):
    QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(PLUGIN_BACKGROUND_MIN_SIZE, PLUGIN_BACKGROUND_MIN_SIZE);

    QString iconName("screenshot");
    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/res/%1.svg").arg(iconName)));

    m_timer = new QTimer(this);
    m_showTimeStr = tr("Screen Capture");
}

bool IconWidget::enabled()
{
    return isEnabled();
}

const QString IconWidget::itemContextMenu()
{
    QList<QVariant> items;
    items.reserve(2);
    QMap<QString, QVariant> shot;
    shot["itemId"] = "shot";
    shot["itemText"] = tr("Screenshot") + getSysShortcuts("screenshot");
    shot["isActive"] = true;
    items.push_back(shot);


    QMap<QString, QVariant> recorder;
    recorder["itemId"] = "recorder";
    recorder["itemText"] = tr("Recording") + getSysShortcuts("deepin-screen-recorder");
    recorder["isActive"] = true;
    items.push_back(recorder);


    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void IconWidget::invokedMenuItem(const QString &menuId)
{

    if (menuId == "shot") {
        QDBusInterface shotDBusInterface("com.deepin.Screenshot",
                                         "/com/deepin/Screenshot",
                                         "com.deepin.Screenshot",
                                         QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("StartScreenshot");
    } else if (menuId == "recorder") {
        QDBusInterface shotDBusInterface("com.deepin.ScreenRecorder",
                                         "/com/deepin/ScreenRecorder",
                                         "com.deepin.ScreenRecorder",
                                         QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("stopRecord");
    }
}

QString IconWidget::getSysShortcuts(const QString type)
{
    QDBusInterface shortcuts("org.deepin.dde.Keybinding1", "/org/deepin/dde/Keybinding1", "org.deepin.dde.Keybinding1");
    if (!shortcuts.isValid()) {
        return getDefaultValue(type);
    }

    QDBusReply<QString> shortLists = shortcuts.call(QStringLiteral("ListAllShortcuts"));
    QJsonDocument doc = QJsonDocument::fromJson(shortLists.value().toUtf8());
    QJsonArray shorts = doc.array();
    QMap<QString, QString> shortcutsMap;

    for (QJsonValue shortcut : shorts) {
        const QString Id = shortcut["Id"].toString();
        if (Id == type) {
            QJsonArray Accels = shortcut["Accels"].toArray();
            QString AccelsString;
            for (QJsonValue accel : Accels)  {
                AccelsString += accel.toString();
            }
            AccelsString.remove('<');
            AccelsString.replace('>', '+');
            AccelsString.replace("Control", "Ctrl");
            AccelsString = "(" + AccelsString + ")";
            return AccelsString;
        }
    }
    return getDefaultValue(type);
}

QString IconWidget::getDefaultValue(const QString type)
{
    QString retShortcut;
    if (type == "screenshot") {
        retShortcut = "Ctrl+Alt+A";
    } else if (type == "deepin-screen-recorder") {
        retShortcut = "Ctrl+Alt+R";
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << "Shortcut Error !!!!!!!!!" << type;
    }
    return retShortcut;
}

QPixmap IconWidget::iconPixMap(QIcon icon, QSize size)
{
    QPixmap pixmap;
    const auto ratio = devicePixelRatioF();
    qDebug() << "获取缩放比例：" << ratio;
    QSize pixmapSize = QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps)? size:(size*ratio);
    qDebug() << "获取图标大小：" << pixmapSize;
    if(!icon.isNull()){
        pixmap = icon.pixmap(pixmapSize);
        pixmap.setDevicePixelRatio(ratio);
        if(!pixmap.isNull()){
            pixmap = pixmap.scaled(size * ratio);
        }else{
            qWarning() << "pixmap is null!";
        }
    }else{
        qWarning() << "icon is null!";
    }
    return  pixmap;
}
void IconWidget::start()
{
    m_showTimeStr = QString("00:00:00");
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_baseTime = QTime::currentTime();
    m_timer->start(400);
}

void IconWidget::stop()
{
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_showTimeStr = tr("Screen Capture");
}

QString IconWidget::getTimeStr()
{
    return m_showTimeStr;
}
void IconWidget::onTimeout()
{
    QTime showTime(0, 0, 0);
    int time = m_baseTime.secsTo(QTime::currentTime());
    showTime = showTime.addSecs(time);
    m_showTimeStr = showTime.toString("hh:mm:ss");
    qInfo() << __FUNCTION__ << __LINE__ << ">>>>>> m_showTimeStr: " << m_showTimeStr;
    update();
}
void IconWidget::paintEvent(QPaintEvent *e)
{
    qInfo() << ">>>>>>>>>>>>>>>>>>>>>>> " << __FUNCTION__ << __LINE__;
    QPainter painter(this);

    QPixmap pixmap;
    QString iconName = "screenshot";
    if(m_showTimeStr != tr("Screen Capture")){
        iconName = "screen-recording";
    }
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        iconName = "screenshot_dark";
    }
    int iconSize = PLUGIN_ICON_MAX_SIZE + 4;

    painter.setOpacity(1);

    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/res/%1.svg").arg(iconName)));
    pixmap = iconPixMap(m_icon, QSize(iconSize, iconSize));
    const auto ratio = devicePixelRatioF();
    const QRectF &rf = QRectF(rect().x(),rect().y(),rect().width(),rect().height()*0.75);
    QPointF point = rf.center()-pixmap.rect().center();
    //绘制图标
    painter.drawPixmap(point.x(),point.y(), pixmap);

    const QRectF &trf = QRectF(rect().x(),rect().y()+rect().height()*0.65,rect().width(),rect().height()*0.25);
    QFont font = painter.font() ;
    font.setPointSize(8);
    painter.setFont(font);
    painter.setOpacity(0.7);
    painter.setPen(QPen(QGuiApplication::palette().color(QPalette::BrightText)));
    //绘制文字
    painter.drawText(trf, Qt::AlignBottom | Qt::AlignCenter, m_showTimeStr);

    QWidget::paintEvent(e);
}

const QPixmap IconWidget::loadSvg(const QString &fileName, const QSize &size) const
{
    const auto ratio = devicePixelRatioF();

    QPixmap pixmap;
    pixmap = QIcon::fromTheme(fileName, m_icon).pixmap(size * ratio);
    pixmap.setDevicePixelRatio(ratio);

    return pixmap;
}

IconWidget::~IconWidget()
{

}
