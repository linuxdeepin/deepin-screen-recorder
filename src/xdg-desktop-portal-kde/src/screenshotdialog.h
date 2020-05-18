/*
 * Copyright © 2018 Red Hat, Inc
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
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_DIALOG_H

#include <QDialog>

namespace Ui
{
class ScreenshotDialog;
}

class ScreenshotDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ScreenshotDialog(QDialog *parent = nullptr, Qt::WindowFlags flags = {});
    ~ScreenshotDialog();

    QImage image() const;

public Q_SLOTS:
    void takeScreenshot();

Q_SIGNALS:
    void failed();

private:
    Ui::ScreenshotDialog * m_dialog;
    QImage m_image;

    int mask();
};

#endif // XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_DIALOG_H


