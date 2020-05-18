/*
 * Copyright © 2017-2018 Red Hat, Inc
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

#include "appchooserdialog.h"
#include "appchooserdialogitem.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QLoggingCategory>
#include <KLocalizedString>
#include <QSettings>
#include <QStandardPaths>
#include <QScrollArea>
#include <QTimer>

#include <KProcess>

Q_LOGGING_CATEGORY(XdgDesktopPortalKdeAppChooserDialog, "xdp-kde-app-chooser-dialog")

AppChooserDialog::AppChooserDialog(const QStringList &choices, const QString &defaultApp, const QString &fileName, QDialog *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_choices(choices)
    , m_defaultApp(defaultApp)
{
    setMinimumWidth(640);
    setMaximumHeight(480);

    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->setSpacing(20);
    vboxLayout->setMargin(20);

    QLabel *label = new QLabel(this);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label->setScaledContents(true);
    label->setWordWrap(true);
    label->setText(i18n("Select application to open \"%1\". Other applications are available in <a href=#discover><span style=\"text-decoration: underline\">Discover</span></a>.", fileName));
    label->setOpenExternalLinks(false);

    connect(label, &QLabel::linkActivated, this, [] () {
        KProcess::startDetached(QStringLiteral("plasma-discover"));
    });

    vboxLayout->addWidget(label);

    QWidget *appsWidget = new QWidget(this);
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(appsWidget);
    scrollArea->setWidgetResizable(true);

    // FIXME: workaround scrollarea sizing, set minimum height to make sure at least two rows are visible
    if (choices.count() > 3) {
        scrollArea->setMinimumHeight(200);
    }

    m_gridLayout = new QGridLayout;
    appsWidget->setLayout(m_gridLayout);

    QTimer::singleShot(0, this, &AppChooserDialog::addDialogItems);

    vboxLayout->addWidget(scrollArea);

    setLayout(vboxLayout);
    setWindowTitle(i18n("Open with"));
}

AppChooserDialog::~AppChooserDialog()
{
    delete m_gridLayout;
}

void AppChooserDialog::updateChoices(const QStringList &choices)
{
    bool changed = false;

    // Check if we will be adding something
    for (const QString &choice : choices) {
        if (!m_choices.contains(choice)) {
            changed = true;
            m_choices << choice;
        }
    }

    // Check if we will be removing something
    for (const QString &choice : m_choices) {
        if (!choices.contains(choice)) {
            changed = true;
            m_choices.removeAll(choice);
        }
    }

    // If something changed, clear the layout and add the items again
    if (changed) {
        int rowCount = m_gridLayout->rowCount();
        int columnCount = m_gridLayout->columnCount();

        for (int i = 0; i < rowCount; ++i) {
            for (int j  = 0; j < columnCount; ++j) {
                QLayoutItem *item = m_gridLayout->itemAtPosition(i, j);
                if (item) {
                    QWidget *widget = item->widget();
                    if (widget) {
                        m_gridLayout->removeWidget(widget);
                        widget->deleteLater();
                    }
                }
            }
        }

        addDialogItems();
    }
}

QString AppChooserDialog::selectedApplication() const
{
    if (m_selectedApplication.isEmpty()) {
        return m_defaultApp;
    }

    return m_selectedApplication;
}

void AppChooserDialog::addDialogItems()
{
    int i = 0, j = 0;
    for (const QString &choice : m_choices) {
        const QString desktopFile = choice + QStringLiteral(".desktop");
        const QStringList desktopFilesLocations = QStandardPaths::locateAll(QStandardPaths::ApplicationsLocation, desktopFile, QStandardPaths::LocateFile);
        for (const QString &desktopFile : desktopFilesLocations) {
            QString applicationIcon;
            QString applicationName;
            QSettings settings(desktopFile, QSettings::IniFormat);
            settings.beginGroup(QStringLiteral("Desktop Entry"));
            if (settings.contains(QStringLiteral("X-GNOME-FullName"))) {
                applicationName = settings.value(QStringLiteral("X-GNOME-FullName")).toString();
            } else {
                applicationName = settings.value(QStringLiteral("Name")).toString();
            }
            applicationIcon = settings.value(QStringLiteral("Icon")).toString();

            AppChooserDialogItem *item = new AppChooserDialogItem(applicationName, applicationIcon, choice, this);
            m_gridLayout->addWidget(item, i, j++, Qt::AlignHCenter);

            connect(item, &AppChooserDialogItem::clicked, this, [this] (const QString &selectedApplication) {
                m_selectedApplication = selectedApplication;
                QDialog::accept();
            });

            if (choice == m_defaultApp) {
                item->setDown(true);
                item->setChecked(true);
            }

            if (j == 3) {
                i++;
                j = 0;
            }
        }
    }
}
