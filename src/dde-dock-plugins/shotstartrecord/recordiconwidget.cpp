// SPDX-FileCopyrightText: 2021-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordiconwidget.h"
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

RecordIconWidget::RecordIconWidget(DWidget *parent)
    : DWidget(parent)
    , m_dockInter(new recordiconwidget_interface(DOCK_INTERFACE,
                                                DOCK_PATH,
                                                QDBusConnection::sessionBus(),
                                                this))
    , m_blgPixmap(nullptr)
    , centralLayout(nullptr)
    , m_iconLabel(new QLabel(this))
    , m_position(Dock::Position::Bottom)
{
    qCDebug(dsrApp) << "RecordIconWidget constructor called.";
    setMouseTracking(true);
    setMinimumSize(PLUGIN_BACKGROUND_MIN_SIZE, PLUGIN_BACKGROUND_MIN_SIZE);
    qCDebug(dsrApp) << "Mouse tracking enabled and minimum size set.";

    auto *layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_iconLabel);
    qCDebug(dsrApp) << "Layout initialized.";

    // FIXME: temporarily disable m_iconLabel
    m_iconLabel->setVisible(false);
    qCDebug(dsrApp) << "Icon label visibility set to false.";

    // 连接 DBus 信号
    connect(m_dockInter, SIGNAL(propertyChanged(QString,QVariant)),
            this, SLOT(onPropertyChanged(QString,QVariant)));
    qCDebug(dsrApp) << "Connected propertyChanged signal.";

    m_position = m_dockInter->position();
    qCDebug(dsrApp) << "Initial dock position:" << m_position;
    
    QString iconName("status-screen-record");
    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/res/%1.svg").arg(iconName)));
    qCDebug(dsrApp) << "Default icon set.";
    
    updateIcon();
    qCDebug(dsrApp) << "RecordIconWidget constructor finished.";
}

void RecordIconWidget::updateIcon()
{
    qCDebug(dsrApp) << "updateIcon method called.";
    if (Dock::Position::Top == m_position || Dock::Position::Bottom == m_position) {
        m_pixmap = loadSvg("status-screen-record", QSize(PLUGIN_ICON_MAX_SIZE, PLUGIN_ICON_MAX_SIZE));
        qCDebug(dsrApp) << "Loading icon for Top/Bottom position.";
    } else {
        m_pixmap = loadSvg("status-screen-record", QSize(PLUGIN_ICON_MAX_SIZE, PLUGIN_ICON_MAX_SIZE));
        qCDebug(dsrApp) << "Loading icon for other positions.";
    }
    m_iconLabel->setPixmap(m_pixmap);
    qCDebug(dsrApp) << "Icon label pixmap set.";
    qCDebug(dsrApp) << "updateIcon method finished.";
}

void RecordIconWidget::onPositionChanged(int value)
{
    qCDebug(dsrApp) << "onPositionChanged method called with value:" << value;
    m_position = value;
    updateIcon();
    qCDebug(dsrApp) << "Position updated to:" << m_position << ", icon updated.";
    qCDebug(dsrApp) << "onPositionChanged method finished.";
}

void RecordIconWidget::onPropertyChanged(const QString &property, const QVariant &value)
{
    qCDebug(dsrApp) << "onPropertyChanged method called with property:" << property << "value:" << value;
    if (property == "Position") {
        onPositionChanged(value.toInt());
        qCDebug(dsrApp) << "Property is Position, calling onPositionChanged.";
    } else {
        qCDebug(dsrApp) << "Property is not Position.";
    }
    qCDebug(dsrApp) << "onPropertyChanged method finished.";
}

bool RecordIconWidget::enabled()
{
    qCDebug(dsrApp) << "enabled method called, returning isEnabled():" << isEnabled();
    return isEnabled();
}

const QString RecordIconWidget::itemContextMenu()
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
    qCDebug(dsrApp) << "Menu structure created.";

    QString contextMenu = QJsonDocument::fromVariant(menu).toJson();
    qCDebug(dsrApp) << "Returning context menu JSON:" << contextMenu;
    return contextMenu;
}

void RecordIconWidget::invokedMenuItem(const QString &menuId)
{
    qCDebug(dsrApp) << "invokedMenuItem method called with menuId:" << menuId;
    qCInfo(dsrApp) << "Menu item invoked:" << menuId;
    if (menuId == "shot") {
        qCDebug(dsrApp) << "Starting screenshot via DBus";
        QDBusInterface shotDBusInterface(
            "com.deepin.Screenshot", "/com/deepin/Screenshot", "com.deepin.Screenshot", QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("StartScreenshot");
        qCDebug(dsrApp) << "Screenshot StartScreenshot async call initiated.";
    } else if (menuId == "recorder") {
        qCDebug(dsrApp) << "Stopping recording via DBus";
        QDBusInterface shotDBusInterface("com.deepin.ScreenRecorder",
                                         "/com/deepin/ScreenRecorder",
                                         "com.deepin.ScreenRecorder",
                                         QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("stopRecord");
        qCDebug(dsrApp) << "ScreenRecorder stopRecord async call initiated.";
    } else {
        qCDebug(dsrApp) << "Unknown menuId invoked:" << menuId;
    }
    qCDebug(dsrApp) << "invokedMenuItem method finished.";
}

QString RecordIconWidget::getSysShortcuts(const QString &type)
{
    qCDebug(dsrApp) << "getSysShortcuts method called for type:" << type;
    qCDebug(dsrApp) << "Getting system shortcuts for type:" << type;
    QDBusInterface shortcuts(KEYBINDING_NAME, KEYBINDING_PATH, KEYBINDING_INTERFACE);
    if (!shortcuts.isValid()) {
        qCWarning(dsrApp) << "Failed to create shortcuts DBus interface, using default values";
        qCDebug(dsrApp) << "Shortcuts DBus interface invalid, returning default value.";
        return getDefaultValue(type);
    }

    QDBusReply<QString> shortLists = shortcuts.call(QStringLiteral("ListAllShortcuts"));
    QJsonDocument doc = QJsonDocument::fromJson(shortLists.value().toUtf8());
    QJsonArray shorts = doc.array();
    QMap<QString, QString> shortcutsMap;
    qCDebug(dsrApp) << "Received and parsed shortcut list from DBus.";

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
            qCDebug(dsrApp) << "Returning found shortcut:" << AccelsString;
            return AccelsString;
        }
    }
    qCDebug(dsrApp) << "No shortcut found for" << type << ", using default value";
    qCDebug(dsrApp) << "Returning default value as no shortcut found.";
    return getDefaultValue(type);
}

QString RecordIconWidget::getDefaultValue(const QString &type)
{
    qCDebug(dsrApp) << "getDefaultValue method called for type:" << type;
    qCDebug(dsrApp) << "Getting default shortcut value for type:" << type;
    QString retShortcut;
    if (type == "screenshot") {
        retShortcut = "Ctrl+Alt+A";
        qCDebug(dsrApp) << "Default shortcut for screenshot:" << retShortcut;
    } else if (type == "deepin-screen-recorder") {
        retShortcut = "Ctrl+Alt+R";
        qCDebug(dsrApp) << "Default shortcut for deepin-screen-recorder:" << retShortcut;
    } else {
        qCDebug(dsrApp) << "Unknown type for default shortcut:" << type;
    }
    qCDebug(dsrApp) << "Default shortcut value:" << retShortcut;
    qCDebug(dsrApp) << "getDefaultValue method finished.";
    return retShortcut;
}

void RecordIconWidget::paintEvent(QPaintEvent *e)
{
    qCDebug(dsrApp) << "paintEvent method called.";
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPixmap pixmap;
    QString iconName = "status-screen-record";
    int iconSize = PLUGIN_ICON_MAX_SIZE;
    qCDebug(dsrApp) << "Painter initialized. Icon name:" << iconName << ", Icon size:" << iconSize;

    if (rect().height() > PLUGIN_BACKGROUND_MIN_SIZE) {
        qCDebug(dsrApp) << "Height is greater than min background size.";
        QColor color;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = Qt::black;
            painter.setOpacity(0.5);
            qCDebug(dsrApp) << "Light theme detected, color black, opacity 0.5.";

            if (m_hover) {
                painter.setOpacity(0.6);
                qCDebug(dsrApp) << "Hovered, opacity 0.6.";
            }

            if (m_pressed) {
                painter.setOpacity(0.3);
                qCDebug(dsrApp) << "Pressed, opacity 0.3.";
            }
        } else {
            color = Qt::white;
            painter.setOpacity(0.1);
            qCDebug(dsrApp) << "Dark theme detected, color white, opacity 0.1.";

            if (m_hover) {
                painter.setOpacity(0.2);
                qCDebug(dsrApp) << "Hovered, opacity 0.2.";
            }

            if (m_pressed) {
                painter.setOpacity(0.05);
                qCDebug(dsrApp) << "Pressed, opacity 0.05.";
            }
        }

        painter.setRenderHint(QPainter::Antialiasing, true);

        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
        qCDebug(dsrApp) << "Frame radius:" << radius;

        QPainterPath path;

        int minSize = std::min(width(), height());
        QRect rc(0, 0, minSize, minSize);
        rc.moveTo(rect().center() - rc.center());

        path.addRoundedRect(rc, radius, radius);
        painter.fillPath(path, color);
        qCDebug(dsrApp) << "Rounded rectangle path filled.";
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        // 最小尺寸时，不画背景，采用深色图标
        iconName.append(PLUGIN_MIN_ICON_NAME);
        qCDebug(dsrApp) << "Height is not greater than min background size, and light theme, appending min icon name.";
    } else {
        qCDebug(dsrApp) << "Height is not greater than min background size, and not light theme.";
    }

    painter.setOpacity(1);

    m_icon = QIcon::fromTheme(iconName, QIcon(QString(":/res/%1.svg").arg(iconName)));
    m_icon.paint(&painter, rect());
    qCDebug(dsrApp) << "Icon painted.";

    QWidget::paintEvent(e);
    qCDebug(dsrApp) << "paintEvent method finished.";
}

void RecordIconWidget::mousePressEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mousePressEvent method called.";
    m_pressed = true;
    update();
    qCDebug(dsrApp) << "Mouse pressed, updating widget.";

    QWidget::mousePressEvent(event);
    qCDebug(dsrApp) << "mousePressEvent method finished.";
}

void RecordIconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mouseReleaseEvent method called.";
    m_pressed = false;
    m_hover = false;
    update();
    qCDebug(dsrApp) << "Mouse released, updating widget.";

    QWidget::mouseReleaseEvent(event);
    qCDebug(dsrApp) << "mouseReleaseEvent method finished.";
}

void RecordIconWidget::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mouseMoveEvent method called.";
    m_hover = true;
    QWidget::mouseMoveEvent(event);
    qCDebug(dsrApp) << "Mouse moved, setting hover to true.";
    qCDebug(dsrApp) << "mouseMoveEvent method finished.";
}

void RecordIconWidget::leaveEvent(QEvent *event)
{
    qCDebug(dsrApp) << "leaveEvent method called.";
    m_hover = false;
    m_pressed = false;
    update();
    qCDebug(dsrApp) << "Mouse left widget area, updating widget.";

    QWidget::leaveEvent(event);
    qCDebug(dsrApp) << "leaveEvent method finished.";
}

const QPixmap RecordIconWidget::loadSvg(const QString &fileName, const QSize &size) const
{
    qCDebug(dsrApp) << "loadSvg method called with fileName:" << fileName << ", size:" << size;
    // High-DPI pixmap always support on Qt6
    QPixmap pixmap = QIcon::fromTheme(fileName, m_icon).pixmap(size, isEnabled() ? QIcon::Normal : QIcon::Disabled);
    qCDebug(dsrApp) << "Pixmap loaded from SVG, returning it.";
    return pixmap;
}

RecordIconWidget::~RecordIconWidget()
{
    qCDebug(dsrApp) << "RecordIconWidget destructor called.";
}
