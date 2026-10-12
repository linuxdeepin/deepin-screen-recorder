// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcut.h"

#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>

#include "utils.h"
#include "utils/log.h"

DCORE_USE_NAMESPACE

Shortcut::Shortcut(QObject *parent)
    : QObject(parent)
{
    qCDebug(dsrApp) << "Initializing Shortcut object.";
    ShortcutGroup screenshotGroup;
    ShortcutGroup exitGroup;
    ShortcutGroup toolsGroup;
    ShortcutGroup recordGroup;
    ShortcutGroup sizeGroup;
    ShortcutGroup setGroup;

    screenshotGroup.groupName = tr("Start/Screenshot");
    exitGroup.groupName = tr("Exit/Save");
    toolsGroup.groupName = tr("Tools");
    recordGroup.groupName = tr("Start/Recording");
    sizeGroup.groupName = tr("Size Adjustment");
    setGroup.groupName = tr("Settings");

    screenshotGroup.groupItems << ShortcutItem(tr("Quick start"), getSysShortcuts("screenshot"))
                               << ShortcutItem(tr("Window screenshot"), getSysShortcuts("screenshot-window"))
                               << ShortcutItem(tr("Delay screenshot"), getSysShortcuts("screenshot-delayed"))
                               << ShortcutItem(tr("Full screenshot"), getSysShortcuts("screenshot-fullscreen"))
                               << ShortcutItem(tr("Quick start scrollshot"), getSysShortcuts("screenshot-scroll"));

    exitGroup.groupItems << ShortcutItem(tr("Exit"), "Esc")
                         << ShortcutItem(tr("Save"), "Ctrl+S")
                         << ShortcutItem(tr("Copy to clipboard"), "Enter");

    // 工具类顺序与截图工具栏保持一致
    toolsGroup.groupItems << ShortcutItem(tr("Geometric Tools"), "R")
                          << ShortcutItem(tr("Line"), "L")
                          << ShortcutItem(tr("Arrow"), "X")
                          << ShortcutItem(tr("Pencil"), "P")
                          << ShortcutItem(tr("Blur"), "B")
                          << ShortcutItem(tr("Text"), "T")
                          << ShortcutItem(tr("Sequence number"), "Alt+S");
#ifdef OCR_SCROLL_FLAGE_ON
    toolsGroup.groupItems << ShortcutItem(tr("Scrollshot"), "Alt+I")
                          << ShortcutItem(tr("Extract text"), "Alt+O");
#endif
    toolsGroup.groupItems << ShortcutItem(tr("Table recognition"), "Alt+T")
                          << ShortcutItem(tr("Pin screenshots"), "Alt+P")
                          << ShortcutItem(tr("AI Screenshot"), "Alt+A")
                          << ShortcutItem(tr("Undo"), "Ctrl+Z")
                          << ShortcutItem(tr("Options"), "F3")
                          << ShortcutItem(tr("Delete"), "Delete");

    recordGroup.groupItems << ShortcutItem(tr("Start recording"), getSysShortcuts("deepin-screen-recorder"))
                           << ShortcutItem(tr("Sound"), "S") << ShortcutItem(tr("Keystroke"), "K")
                           << ShortcutItem(tr("Webcam"), "C") << ShortcutItem(tr("Mouse"), "M")
                           << ShortcutItem(tr("Options"), "F3") << ShortcutItem(" ", " ");

    sizeGroup.groupItems << ShortcutItem(tr("Increase height up"), "Ctrl+↑")
                         << ShortcutItem(tr("Increase height down"), "Ctrl+↓")
                         << ShortcutItem(tr("Increase width left"), "Ctrl+←")
                         << ShortcutItem(tr("Increase width right"), "Ctrl+→")
                         << ShortcutItem(tr("Decrease height up"), "Ctrl+Shift+↑")
                         << ShortcutItem(tr("Decrease height down"), "Ctrl+Shift+↓")
                         << ShortcutItem(tr("Decrease width left"), "Ctrl+Shift+←")
                         << ShortcutItem(tr("Decrease width right"), "Ctrl+Shift+→");

    setGroup.groupItems << ShortcutItem(tr("Help"), "F1") << ShortcutItem(tr("Display shortcuts"), "Ctrl+Shift+?");

    m_shortcutGroups << screenshotGroup << recordGroup << toolsGroup << exitGroup << sizeGroup << setGroup;
    qCDebug(dsrApp) << "Shortcut groups populated.";

    // convert to json object
    QJsonArray jsonGroups;
    for (auto scg : m_shortcutGroups) {
        QJsonObject jsonGroup;
        jsonGroup.insert("groupName", scg.groupName);
        QJsonArray jsonGroupItems;
        for (auto sci : scg.groupItems) {
            QJsonObject jsonItem;
            jsonItem.insert("name", sci.name);
            jsonItem.insert("value", sci.value);
            jsonGroupItems.append(jsonItem);
        }
        jsonGroup.insert("groupItems", jsonGroupItems);
        jsonGroups.append(jsonGroup);
    }
    m_shortcutObj.insert("shortcut", jsonGroups);
    qCDebug(dsrApp) << "Shortcut object converted to JSON.";
}
QString Shortcut::toStr()
{
    qCDebug(dsrApp) << "Converting shortcut object to string.";
    QJsonDocument doc(m_shortcutObj);
    return doc.toJson().data();
}
QString Shortcut::getSysShortcuts(const QString type)
{
    qCDebug(dsrApp) << "Getting system shortcuts for type:" << type;
    QDBusInterface shortcuts(KEYBINDING_NAME, KEYBINDING_PATH, KEYBINDING_INTERFACE);

    if (!shortcuts.isValid()) {
        qCWarning(dsrApp) << "DBus interface for shortcuts is not valid, returning default value.";
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
            for (QJsonValue accel : Accels) {
                AccelsString += accel.toString();
            }
            AccelsString.remove('<');
            AccelsString.replace('>', '+');
            AccelsString.replace("Control", "Ctrl");
            AccelsString.replace("Print", "PrintScreen");
            qCDebug(dsrApp) << "Found shortcut for type " << type << ":" << AccelsString;
            if (!AccelsString.isEmpty()) {
                return AccelsString;
            }
            // 系统里登记了该快捷键但没配置按键时回落到默认值，避免面板显示空白
            qCWarning(dsrApp) << "Empty shortcut for type " << type << ", returning default value.";
            return getDefaultValue(type);
        }
    }
    qCDebug(dsrApp) << "Shortcut not found for type " << type << ", returning default value.";
    return getDefaultValue(type);
}

QString Shortcut::getDefaultValue(const QString type)
{
    qCDebug(dsrApp) << "Getting default shortcut value for type:" << type;
    QString retShortcut;
    if (type == "screenshot") {
        retShortcut = "Ctrl+Alt+A";
        qCDebug(dsrApp) << "Default shortcut for screenshot:" << retShortcut;
    } else if (type == "deepin-screen-recorder") {
        retShortcut = "Ctrl+Alt+R";
        qCDebug(dsrApp) << "Default shortcut for deepin-screen-recorder:" << retShortcut;
    } else if (type == "screenshot-window") {
        retShortcut = "Alt+PrintScreen";
        qCDebug(dsrApp) << "Default shortcut for screenshot-window:" << retShortcut;
    } else if (type == "screenshot-delayed") {
        retShortcut = "Ctrl+PrintScreen";
        qCDebug(dsrApp) << "Default shortcut for screenshot-delayed:" << retShortcut;
    } else if (type == "screenshot-fullscreen") {
        retShortcut = "PrintScreen";
        qCDebug(dsrApp) << "Default shortcut for screenshot-fullscreen:" << retShortcut;
    } else if (type == "screenshot-scroll") {
        retShortcut = "Ctrl+Alt+I";
        qCDebug(dsrApp) << "Default shortcut for screenshot-scroll:" << retShortcut;
    } else {
        qCWarning(dsrApp) << "Unknown shortcut type:" << type << ", returning empty string.";
    }
    return retShortcut;
}
