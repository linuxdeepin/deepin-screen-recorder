// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconwidget.h"
#include "dde-dock/constants.h"
#include "../../utils/log.h"
#include "../../dbus_name.h"

#include <DGuiApplicationHelper>
#include <DStyle>
#include <DSysInfo>

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
DCORE_USE_NAMESPACE

IconWidget::IconWidget(DWidget *parent)
    : DWidget(parent)
    , m_dockInter(new iconwidget_interface(DOCK_INTERFACE,
                                          DOCK_PATH,
                                          QDBusConnection::sessionBus(),
                                          this))
    , m_blgPixmap(nullptr)
    , centralLayout(nullptr)
    , m_iconLabel(new QLabel(this))
{
    qCDebug(dsrApp) << "IconWidget constructor called.";
    setContentsMargins(0, 0, 0, 0);
    
    // 创建布局
    auto *layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_iconLabel);

    // FIXME: temporarily disable m_iconLabel
    m_iconLabel->setVisible(false);
    qCDebug(dsrApp) << "m_iconLabel visibility set to false.";

    // 初始化
    m_systemVersion = DSysInfo::minorVersion().toInt();
    setMouseTracking(true);
    setMinimumSize(PLUGIN_BACKGROUND_MIN_SIZE, PLUGIN_BACKGROUND_MIN_SIZE);
    qCDebug(dsrApp) << "System version:" << m_systemVersion << ", minimum size set.";

    // 设置图标
    QString iconName = m_systemVersion >= 1070 ? "screenshot" : "screen-capture";
    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/res/%1.svg").arg(iconName)));
    qCDebug(dsrApp) << "Icon name set to:" << iconName;
    
    // 连接信号
    connect(m_dockInter, SIGNAL(propertyChanged(QString,QVariant)),
            this, SLOT(onPropertyChanged(QString,QVariant)));
            
    // 获取初始位置
    m_position = m_dockInter->position();
    updateIcon();
    qCDebug(dsrApp) << "Initial position:" << m_position << ", icon updated.";
    qCDebug(dsrApp) << "IconWidget constructor finished.";
}

bool IconWidget::enabled()
{
    qCDebug(dsrApp) << "enabled method called.";
    return isEnabled();
}

const QString IconWidget::itemContextMenu()
{
    qCDebug(dsrApp) << "itemContextMenu method called.";
    QList<QVariant> items;
    items.reserve(2);
    QMap<QString, QVariant> shot;
    shot["itemId"] = "shot";
    shot["itemText"] = tr("Screenshot") + getSysShortcuts("screenshot");
    shot["isActive"] = true;
    items.push_back(shot);
    qCDebug(dsrApp) << "Added screenshot menu item.";

    QMap<QString, QVariant> recorder;
    recorder["itemId"] = "recorder";
    recorder["itemText"] = tr("Recording") + getSysShortcuts("deepin-screen-recorder");
    recorder["isActive"] = true;
    items.push_back(recorder);
    qCDebug(dsrApp) << "Added recording menu item.";

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    QString contextMenu = QJsonDocument::fromVariant(menu).toJson();
    qCDebug(dsrApp) << "itemContextMenu method finished, returning:" << contextMenu;
    return contextMenu;
}

void IconWidget::invokedMenuItem(const QString &menuId)
{
    qCDebug(dsrApp) << "invokedMenuItem method called with menuId:" << menuId;
    qCInfo(dsrApp) << "Menu item invoked:" << menuId;
    if (menuId == "shot") {
        qCDebug(dsrApp) << "Starting screenshot via DBus";
        QDBusInterface shotDBusInterface(
            "com.deepin.Screenshot", "/com/deepin/Screenshot", "com.deepin.Screenshot", QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("StartScreenshot");
        qCDebug(dsrApp) << "StartScreenshot async call initiated.";
    } else if (menuId == "recorder") {
        qCDebug(dsrApp) << "Stopping recording via DBus";
        QDBusInterface shotDBusInterface("com.deepin.ScreenRecorder",
                                         "/com/deepin/ScreenRecorder",
                                         "com.deepin.ScreenRecorder",
                                         QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("stopRecord");
        qCDebug(dsrApp) << "stopRecord async call initiated.";
    } else {
        qCDebug(dsrApp) << "Unknown menuId:" << menuId;
    }
    qCDebug(dsrApp) << "invokedMenuItem method finished.";
}

QString IconWidget::getSysShortcuts(const QString type)
{
    qCDebug(dsrApp) << "getSysShortcuts method called with type:" << type;
    qCDebug(dsrApp) << "Getting system shortcuts for type:" << type;
    QDBusInterface shortcuts(KEYBINDING_NAME, KEYBINDING_PATH, KEYBINDING_INTERFACE);

    if (!shortcuts.isValid()) {
        qCWarning(dsrApp) << "Failed to create shortcuts DBus interface, using default values";
        QString defaultValue = getDefaultValue(type);
        qCDebug(dsrApp) << "Shortcuts DBus interface is invalid, returning default value:" << defaultValue;
        return defaultValue;
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
            for (QJsonValue accel : Accels) {
                AccelsString += accel.toString();
            }
            AccelsString.remove('<');
            AccelsString.replace('>', '+');
            AccelsString.replace("Control", "Ctrl");
            AccelsString = "(" + AccelsString + ")";
            qCDebug(dsrApp) << "Found shortcut for" << type << ":" << AccelsString;
            qCDebug(dsrApp) << "getSysShortcuts method finished, returning found shortcut.";
            return AccelsString;
        }
    }
    qCDebug(dsrApp) << "No shortcut found for" << type << ", using default value";
    QString defaultValue = getDefaultValue(type);
    qCDebug(dsrApp) << "getSysShortcuts method finished, returning default value:" << defaultValue;
    return defaultValue;
}

QString IconWidget::getDefaultValue(const QString type)
{
    qCDebug(dsrApp) << "getDefaultValue method called with type:" << type;
    QString retShortcut;
    if (type == "screenshot") {
        retShortcut = "Ctrl+Alt+A";
        qCDebug(dsrApp) << "Type is screenshot, setting shortcut to Ctrl+Alt+A.";
    } else if (type == "deepin-screen-recorder") {
        retShortcut = "Ctrl+Alt+R";
        qCDebug(dsrApp) << "Type is deepin-screen-recorder, setting shortcut to Ctrl+Alt+R.";
    } else {
        qCDebug(dsrApp) << "Unknown type for shortcut, logging error.";
    }
    qCDebug(dsrApp) << "Using default shortcut for" << type << ":" << retShortcut;
    qCDebug(dsrApp) << "getDefaultValue method finished, returning:" << retShortcut;
    return retShortcut;
}

void IconWidget::paintEvent(QPaintEvent *e)
{
    qCDebug(dsrApp) << "paintEvent method called.";
    QPainter painter(this);

    QPixmap pixmap;
    QString iconName = "screen-capture";
    if(m_systemVersion >= 1070 || DSysInfo::deepinType() == DSysInfo::DeepinDesktop){
        iconName = "screenshot";
        qCDebug(dsrApp) << "System version is >= 1070 or DeepinDesktop, setting iconName to screenshot.";
    }
    int iconSize = PLUGIN_ICON_MAX_SIZE;

    if (rect().height() > PLUGIN_BACKGROUND_MIN_SIZE) {
        qCDebug(dsrApp) << "Height is greater than PLUGIN_BACKGROUND_MIN_SIZE, drawing background.";
        QColor color;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = Qt::black;
            painter.setOpacity(0.5);
            qCDebug(dsrApp) << "Theme is LightType, setting color to black and opacity to 0.5.";
            if (m_hover) {
                painter.setOpacity(0.6);
                qCDebug(dsrApp) << "Hover is true, setting opacity to 0.6.";
            }

            if (m_pressed) {
                painter.setOpacity(0.3);
                qCDebug(dsrApp) << "Pressed is true, setting opacity to 0.3.";
            }
        } else {
            color = Qt::white;
            painter.setOpacity(0.1);
            qCDebug(dsrApp) << "Theme is not LightType, setting color to white and opacity to 0.1.";
            if (m_hover) {
                painter.setOpacity(0.2);
                qCDebug(dsrApp) << "Hover is true, setting opacity to 0.2.";
            }

            if (m_pressed) {
                painter.setOpacity(0.05);
                qCDebug(dsrApp) << "Pressed is true, setting opacity to 0.05.";
            }
        }

        painter.setRenderHint(QPainter::Antialiasing, true);

        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);

        QPainterPath path;

        int minSize = std::min(width(), height());
        QRect rc(0, 0, minSize, minSize);
        rc.moveTo(rect().center() - rc.center());

        path.addRoundedRect(rc, radius, radius);
        painter.fillPath(path, color);
        qCDebug(dsrApp) << "Background drawn with rounded rectangle.";
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        // 最小尺寸时，不画背景，采用深色图标
        iconName.append(PLUGIN_MIN_ICON_NAME);
        qCDebug(dsrApp) << "Height is not greater than PLUGIN_BACKGROUND_MIN_SIZE and theme is LightType, appending min icon name.";
    } else {
        qCDebug(dsrApp) << "Height is not greater than PLUGIN_BACKGROUND_MIN_SIZE and theme is not LightType, no background drawn, no min icon.";
    }

    painter.setOpacity(1);

    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/res/%1.svg").arg(iconName)));
    m_icon.paint(&painter, rect());
    qCDebug(dsrApp) << "Main icon painted with opacity 1.";

    QWidget::paintEvent(e);
    qCDebug(dsrApp) << "paintEvent method finished.";
}

void IconWidget::mousePressEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mousePressEvent method called.";
    m_pressed = true;
    update();
    qCDebug(dsrApp) << "m_pressed set to true and widget updated.";

    QWidget::mousePressEvent(event);
    qCDebug(dsrApp) << "mousePressEvent method finished.";
}

void IconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mouseReleaseEvent method called.";
    m_pressed = false;
    m_hover = false;
    update();
    qCDebug(dsrApp) << "m_pressed and m_hover set to false, widget updated.";

    QWidget::mouseReleaseEvent(event);
    qCDebug(dsrApp) << "mouseReleaseEvent method finished.";
}

void IconWidget::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mouseMoveEvent method called.";
    m_hover = true;
    QWidget::mouseMoveEvent(event);
    qCDebug(dsrApp) << "m_hover set to true.";
    qCDebug(dsrApp) << "mouseMoveEvent method finished.";
}

void IconWidget::leaveEvent(QEvent *event)
{
    qCDebug(dsrApp) << "leaveEvent method called.";
    m_hover = false;
    m_pressed = false;
    update();
    qCDebug(dsrApp) << "m_hover and m_pressed set to false, widget updated.";

    QWidget::leaveEvent(event);
    qCDebug(dsrApp) << "leaveEvent method finished.";
}

const QPixmap IconWidget::loadSvg(const QString &fileName, const QSize &size) const
{
    qCDebug(dsrApp) << "loadSvg method called with fileName:" << fileName << "size:" << size;
    // High-DPI pixmap always support on Qt6
    QPixmap pixmap = QIcon::fromTheme(fileName, m_icon).pixmap(size, isEnabled() ? QIcon::Normal : QIcon::Disabled);
    qCDebug(dsrApp) << "Pixmap loaded, returning.";
    return pixmap;
}

void IconWidget::onPropertyChanged(const QString &property, const QVariant &value)
{
    qCDebug(dsrApp) << "onPropertyChanged method called with property:" << property << "value:" << value;
    if (property == "Position") {
        onPositionChanged(value.toInt());
        qCDebug(dsrApp) << "Property is Position, calling onPositionChanged.";
    } else {
        qCDebug(dsrApp) << "Property is not Position, no action.";
    }
    qCDebug(dsrApp) << "onPropertyChanged method finished.";
}

void IconWidget::onPositionChanged(int value)
{
    qCDebug(dsrApp) << "onPositionChanged method called with value:" << value;
    m_position = value;
    updateIcon();
    qCDebug(dsrApp) << "Position updated to:" << m_position << ", icon updated.";
    qCDebug(dsrApp) << "onPositionChanged method finished.";
}

void IconWidget::updateIcon()
{
    qCDebug(dsrApp) << "updateIcon method called.";
    QSize iconSize(PLUGIN_ICON_MAX_SIZE, PLUGIN_ICON_MAX_SIZE);
    QPixmap pixmap = loadSvg(m_systemVersion >= 1070 ? "screenshot" : "screen-capture", iconSize);
    m_iconLabel->setPixmap(pixmap);
    qCDebug(dsrApp) << "Icon updated.";
    qCDebug(dsrApp) << "updateIcon method finished.";
}

IconWidget::~IconWidget()
{
    qCDebug(dsrApp) << "IconWidget destructor called.";
    if (m_dockInter) {
        m_dockInter->deleteLater();
        m_dockInter = nullptr;
        qCDebug(dsrApp) << "m_dockInter deleted.";
    } else {
        qCDebug(dsrApp) << "m_dockInter is null, no deletion needed.";
    }
    qCDebug(dsrApp) << "IconWidget destructor finished.";
}
