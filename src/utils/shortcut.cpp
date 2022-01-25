/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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

    screenshotGroup.groupItems << ShortcutItem(tr("Quick start"), "Ctrl+Alt+A")
                               << ShortcutItem(tr("Window screenshot"), "Alt+PrintScreen")
                               << ShortcutItem(tr("Delay screenshot"),  "Ctrl+PrintScreen")
                               << ShortcutItem(tr("Full screenshot"),  "PrintScreen");

    exitGroup.groupItems << ShortcutItem(tr("Exit"), "Esc")
                         << ShortcutItem(tr("Save"), "Ctrl+S");
#ifdef OCR_SCROLL_FLAGE_ON
    toolsGroup.groupItems << ShortcutItem(tr("Scrollshot"), "Alt+I");
#endif
    toolsGroup.groupItems << ShortcutItem(tr("Pin screenshots"), "Alt+P")
                          << ShortcutItem(tr("Rectangle"), "R")
                          << ShortcutItem(tr("Ellipse"), "O")
                          << ShortcutItem(tr("Line"), "L")
                          << ShortcutItem(tr("Pencil"), "P")
                          << ShortcutItem(tr("Text"), "T");
#ifdef OCR_SCROLL_FLAGE_ON
    toolsGroup.groupItems << ShortcutItem(tr("Extract text"), "Alt+O");
#endif
    toolsGroup.groupItems << ShortcutItem(tr("Delete"), "Delete")
                          << ShortcutItem(tr("Undo"), "Ctrl+Z")
                          << ShortcutItem(tr("Options"), "F3");

    recordGroup.groupItems << ShortcutItem(tr("Start recording"), "Ctr+Alt+R")
                           << ShortcutItem(tr("Sound"), "S")
                           << ShortcutItem(tr("Keystroke"), "K")
                           << ShortcutItem(tr("Webcam"), "W")
                           << ShortcutItem(tr("Mouse"), "M")
                           << ShortcutItem(tr("Options"), "F3")
                           << ShortcutItem(" ", " ");

    sizeGroup.groupItems << ShortcutItem(tr("Increase height up"), "Ctrl+Up")
                         << ShortcutItem(tr("Increase height down"), "Ctrl+Down")
                         << ShortcutItem(tr("Increase width left"), "Ctrl+Left")
                         << ShortcutItem(tr("Increase width right"), "Ctrl+Right")
                         << ShortcutItem(tr("Decrease height up"), "Ctrl+Shift+Up")
                         << ShortcutItem(tr("Decrease height down"), "Ctrl+Shift+Down")
                         << ShortcutItem(tr("Decrease width left"), "Ctrl+Shift+Left")
                         << ShortcutItem(tr("Decrease width right"), "Ctrl+Shift+Right");

    setGroup.groupItems << ShortcutItem(tr("Help"), "F1")
                        << ShortcutItem(tr("Display shortcuts"), "Ctrl+Shift+?");

    m_shortcutGroups << screenshotGroup <<  recordGroup << toolsGroup <<  exitGroup << sizeGroup << setGroup;

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
