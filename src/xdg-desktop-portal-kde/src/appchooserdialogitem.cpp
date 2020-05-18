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

#include "appchooserdialogitem.h"

#include <QMouseEvent>
#include <QFontMetrics>
#include <QVBoxLayout>

#include <QDebug>

AppChooserDialogItem::AppChooserDialogItem(const QString &applicationName, const QString &icon, const QString &applicationExec, QWidget *parent)
    : QToolButton(parent)
    , m_applicationName(applicationExec)
{
    setAutoRaise(true);
    setAutoExclusive(true);
    setStyleSheet(QStringLiteral("text-align: center"));
    setIcon(QIcon::fromTheme(icon));
    setIconSize(QSize(64, 64));
    setCheckable(true);
    setFixedHeight(100);
    setFixedWidth(150);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QFontMetrics metrics(font());
    QString elidedText = metrics.elidedText(applicationName, Qt::ElideRight, 128);
    setText(elidedText);

    connect(this, &QToolButton::toggled, this, [this] (bool toggled) {
        if (!toggled) {
            setDown(false);
        }
    });
}

AppChooserDialogItem::~AppChooserDialogItem()
{
}

QString AppChooserDialogItem::applicationName() const
{
    return m_applicationName;
}

void AppChooserDialogItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT clicked(m_applicationName);
    }
}
