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

#include "screencastwidget.h"
#include "waylandintegration.h"

#include <KLocalizedString>

ScreenCastWidget::ScreenCastWidget(QWidget *parent)
    : QListWidget(parent)
{
    QMapIterator<quint32, WaylandIntegration::WaylandOutput> it(WaylandIntegration::screens());
    while (it.hasNext()) {
        it.next();
        QListWidgetItem *widgetItem = new QListWidgetItem(this);
        widgetItem->setData(Qt::UserRole, it.key());
        if (it.value().outputType() == WaylandIntegration::WaylandOutput::Laptop) {
            widgetItem->setIcon(QIcon::fromTheme(QStringLiteral("computer-laptop")));
            widgetItem->setText(i18n("Laptop screen\nModel: %1", it.value().model()));
        } else if (it.value().outputType() == WaylandIntegration::WaylandOutput::Monitor) {
            widgetItem->setIcon(QIcon::fromTheme(QStringLiteral("video-display")));
            widgetItem->setText(i18n("Manufacturer: %1\nModel: %2", it.value().manufacturer(), it.value().model()));
        } else {
            widgetItem->setIcon(QIcon::fromTheme(QStringLiteral("video-television")));
            widgetItem->setText(i18n("Manufacturer: %1\nModel: %2", it.value().manufacturer(), it.value().model()));
        }
    }

    setItemSelected(itemAt(0, 0), true);
}

ScreenCastWidget::~ScreenCastWidget()
{
}

QList<quint32> ScreenCastWidget::selectedScreens() const
{
    QList<quint32> selectedScreens;

    for (QListWidgetItem *item : selectedItems()) {
        selectedScreens << item->data(Qt::UserRole).toUInt();
    }

    return selectedScreens;
}
