/*
 * Copyright © 2018 Alexander Volkov <a.volkov@rusbitech.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_UTILS_H
#define XDG_DESKTOP_PORTAL_KDE_UTILS_H

class QString;
class QWidget;

class Utils
{
public:
    static void setParentWindow(QWidget *w, const QString &parent_window);
};

#endif // XDG_DESKTOP_PORTAL_KDE_UTILS_H
