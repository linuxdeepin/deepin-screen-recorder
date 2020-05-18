/*
 * Copyright © 2017 Red Hat, Inc
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

#include "accessdialog.h"
#include "ui_accessdialog.h"

#include <QLoggingCategory>
#include <QSettings>
#include <QStandardPaths>
#include <QPushButton>

Q_LOGGING_CATEGORY(XdgDesktopPortalKdeAccessDialog, "xdp-kde-access-dialog")

AccessDialog::AccessDialog(QDialog *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_dialog(new Ui::AccessDialog)
{
    m_dialog->setupUi(this);

    connect(m_dialog->buttonBox, &QDialogButtonBox::accepted, this, &AccessDialog::accept);
    connect(m_dialog->buttonBox, &QDialogButtonBox::rejected, this, &AccessDialog::reject);

    m_dialog->iconLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-question")).pixmap(QSize(64, 64)));

    setWindowTitle(i18n("Request device access"));
}

AccessDialog::~AccessDialog()
{
    delete m_dialog;
}

void AccessDialog::setAcceptLabel(const QString &label)
{
    m_dialog->buttonBox->button(QDialogButtonBox::Ok)->setText(label);
}

void AccessDialog::setBody(const QString &body)
{
    m_dialog->bodyLabel->setText(body);
}

void AccessDialog::setIcon(const QString &icon)
{
    m_dialog->iconLabel->setPixmap(QIcon::fromTheme(icon).pixmap(QSize(64, 64)));
}

void AccessDialog::setRejectLabel(const QString &label)
{
    m_dialog->buttonBox->button(QDialogButtonBox::Cancel)->setText(label);
}

void AccessDialog::setSubtitle(const QString &subtitle)
{
    m_dialog->subtitleLabel->setText(subtitle);
}

void AccessDialog::setTitle(const QString &title)
{
    m_dialog->titleLabel->setText(title);
}


