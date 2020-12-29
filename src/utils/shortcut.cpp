/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Maintainer: Peng Hui<penghui@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "shortcut.h"

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;
    ShortcutGroup group3_1;
    ShortcutGroup group4;
    ShortcutGroup group5;

    group1.groupName = tr("Start/Screenshot");
    group2.groupName = tr("Exit/Save");
    group3.groupName = tr("Drawing");
    group3_1.groupName = tr("Record");
    group4.groupName = tr("Size Adjustment");
    group5.groupName = tr("Settings");

    group1.groupItems <<
                      ShortcutItem(tr("Quick start"), "Ctrl+Alt+A") <<
                      ShortcutItem(tr("Window screenshot"), "Alt+PrintScreen") <<
                      ShortcutItem(tr("Delay screenshot"),  "Ctrl+PrintScreen") <<
                      ShortcutItem(tr("Full screenshot"),  "PrintScreen") <<
                      ShortcutItem(tr("Copy to clipboard"), "Ctrl+C");


    group2.groupItems << ShortcutItem(tr("Exit"),   "Esc") <<
                      ShortcutItem(tr("Save"), "Ctrl+S") <<
                      ShortcutItem(" ",   " ") <<
                      ShortcutItem(" ",   " ") <<
                      ShortcutItem(" ",   " ") <<
                      ShortcutItem(" ",   " ");

    group3.groupItems <<
                      ShortcutItem(tr("Rectangle"),  "R") <<
                      ShortcutItem(tr("Ellipse"),        "O") <<
                      ShortcutItem(tr("Line"),        "L") <<
                      ShortcutItem(tr("Pencil"),        "P") <<
                      ShortcutItem(tr("Text"),           "T") <<
                      ShortcutItem(tr("Delete"),    "Delete") <<
                      ShortcutItem(tr("Undo"), "Ctrl+Z") <<
                      ShortcutItem(tr("Options"), "F3");

    group3_1.groupItems << ShortcutItem(tr("Start recording"), "Ctr+Alt+R")
                        << ShortcutItem(tr("Sound"), "S")
                        << ShortcutItem(tr("Keystroke"), "K")
                        << ShortcutItem(tr("Webcam"), "W")
                        << ShortcutItem(tr("Click"), "C") ;

    group4.groupItems <<
                      ShortcutItem(tr("Increase height up"), "Ctrl+Up") <<
                      ShortcutItem(tr("Increase height down"), "Ctrl+Down") <<
                      ShortcutItem(tr("Increase width left"), "Ctrl+Left") <<
                      ShortcutItem(tr("Increase width right"), "Ctrl+Right") <<
                      ShortcutItem(tr("Decrease height up"), "Ctrl+Shift+Up") <<
                      ShortcutItem(tr("Decrease height down"), "Ctrl+Shift+Down") <<
                      ShortcutItem(tr("Decrease width left"), "Ctrl+Shift+Left") <<
                      ShortcutItem(tr("Decrease width right"), "Ctrl+Shift+Right");

    group5.groupItems << ShortcutItem(tr("Help"),   "F1") <<
                      ShortcutItem(tr("Display shortcuts"), "Ctrl+Shift+?");

    m_shortcutGroups << group1 << group2 << group3 << group3_1 << group4 << group5;

    //convert to json object
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
}
QString Shortcut::toStr()
{
    QJsonDocument doc(m_shortcutObj);
    return doc.toJson().data();
}
