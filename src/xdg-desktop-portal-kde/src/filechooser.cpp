/*
 * Copyright © 2016-2018 Red Hat, Inc
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

#include "filechooser.h"
#include "utils.h"

#include <QDialogButtonBox>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QLoggingCategory>
#include <QFile>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUrl>

#include <KLocalizedString>
#include <KFileWidget>

Q_LOGGING_CATEGORY(XdgDesktopPortalKdeFileChooser, "xdp-kde-file-chooser")

// Keep in sync with qflatpakfiledialog from flatpak-platform-plugin
Q_DECLARE_METATYPE(FileChooserPortal::Filter)
Q_DECLARE_METATYPE(FileChooserPortal::Filters)
Q_DECLARE_METATYPE(FileChooserPortal::FilterList)
Q_DECLARE_METATYPE(FileChooserPortal::FilterListList)

QDBusArgument &operator << (QDBusArgument &arg, const FileChooserPortal::Filter &filter)
{
    arg.beginStructure();
    arg << filter.type << filter.filterString;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >> (const QDBusArgument &arg, FileChooserPortal::Filter &filter)
{
    uint type;
    QString filterString;
    arg.beginStructure();
    arg >> type >> filterString;
    filter.type = type;
    filter.filterString = filterString;
    arg.endStructure();

    return arg;
}

QDBusArgument &operator << (QDBusArgument &arg, const FileChooserPortal::FilterList &filterList)
{
    arg.beginStructure();
    arg << filterList.userVisibleName << filterList.filters;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >> (const QDBusArgument &arg, FileChooserPortal::FilterList &filterList)
{
    QString userVisibleName;
    FileChooserPortal::Filters filters;
    arg.beginStructure();
    arg >> userVisibleName >> filters;
    filterList.userVisibleName = userVisibleName;
    filterList.filters = filters;
    arg.endStructure();

    return arg;
}

FileDialog::FileDialog(QDialog *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_fileWidget(new KFileWidget(QUrl(), this))
{
    setLayout(new QVBoxLayout);
    layout()->addWidget(m_fileWidget);

    m_buttons = new QDialogButtonBox(this);
    m_buttons->addButton(m_fileWidget->okButton(), QDialogButtonBox::AcceptRole);
    m_buttons->addButton(m_fileWidget->cancelButton(), QDialogButtonBox::RejectRole);
    connect(m_buttons, &QDialogButtonBox::rejected, m_fileWidget, &KFileWidget::slotCancel);
    connect(m_fileWidget->okButton(), &QAbstractButton::clicked, m_fileWidget, &KFileWidget::slotOk);
    connect(m_fileWidget, &KFileWidget::accepted, m_fileWidget, &KFileWidget::accept);
    connect(m_fileWidget, &KFileWidget::accepted, this, &QDialog::accept);
    connect(m_fileWidget->cancelButton(), &QAbstractButton::clicked, this, &QDialog::reject);
    layout()->addWidget(m_buttons);
}

FileDialog::~FileDialog()
{
}

FileChooserPortal::FileChooserPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qDBusRegisterMetaType<Filter>();
    qDBusRegisterMetaType<Filters>();
    qDBusRegisterMetaType<FilterList>();
    qDBusRegisterMetaType<FilterListList>();
}

FileChooserPortal::~FileChooserPortal()
{
}

uint FileChooserPortal::OpenFile(const QDBusObjectPath &handle,
                           const QString &app_id,
                           const QString &parent_window,
                           const QString &title,
                           const QVariantMap &options,
                           QVariantMap &results)
{
    Q_UNUSED(app_id);

    qCDebug(XdgDesktopPortalKdeFileChooser) << "OpenFile called with parameters:";
    qCDebug(XdgDesktopPortalKdeFileChooser) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeFileChooser) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeFileChooser) << "    title: " << title;
    qCDebug(XdgDesktopPortalKdeFileChooser) << "    options: " << options;

    bool modalDialog = true;
    bool multipleFiles = false;
    QString acceptLabel;
    QStringList nameFilters;
    QStringList mimeTypeFilters;

    /* TODO
     * choices a(ssa(ss)s)
     * List of serialized combo boxes to add to the file chooser.
     *
     * For each element, the first string is an ID that will be returned with the response, te second string is a user-visible label.
     * The a(ss) is the list of choices, each being a is an ID and a user-visible label. The final string is the initial selection,
     * or "", to let the portal decide which choice will be initially selected. None of the strings, except for the initial selection, should be empty.
     *
     * As a special case, passing an empty array for the list of choices indicates a boolean choice that is typically displayed as a check button, using "true" and "false" as the choices.
     * Example: [('encoding', 'Encoding', [('utf8', 'Unicode (UTF-8)'), ('latin15', 'Western')], 'latin15'), ('reencode', 'Reencode', [], 'false')]
     */

    if (options.contains(QStringLiteral("accept_label"))) {
        acceptLabel = options.value(QStringLiteral("accept_label")).toString();
    }

    if (options.contains(QStringLiteral("modal"))) {
        modalDialog = options.value(QStringLiteral("modal")).toBool();
    }

    if (options.contains(QStringLiteral("multiple"))) {
        multipleFiles = options.value(QStringLiteral("multiple")).toBool();
    }

    if (options.contains(QStringLiteral("filters"))) {
        FilterListList filterListList = qdbus_cast<FilterListList>(options.value(QStringLiteral("filters")));
        for (const FilterList &filterList : filterListList) {
            QStringList filterStrings;
            for (const Filter &filterStruct : filterList.filters) {
                if (filterStruct.type == 0) {
                    filterStrings << filterStruct.filterString;
                } else {
                    mimeTypeFilters << filterStruct.filterString;
                }
            }

            if (!filterStrings.isEmpty()) {
                nameFilters << QStringLiteral("%1|%2").arg(filterStrings.join(QLatin1Char(' '))).arg(filterList.userVisibleName);
            }
        }
    }

    QScopedPointer<FileDialog, QScopedPointerDeleteLater> fileDialog(new FileDialog());
    Utils::setParentWindow(fileDialog.data(), parent_window);
    fileDialog->setWindowTitle(title);
    fileDialog->setModal(modalDialog);
    fileDialog->m_fileWidget->setMode(multipleFiles ? KFile::Mode::Files | KFile::Mode::ExistingOnly : KFile::Mode::File | KFile::Mode::ExistingOnly);
    fileDialog->m_fileWidget->okButton()->setText(!acceptLabel.isEmpty() ? acceptLabel : i18n("Open"));

    if (!nameFilters.isEmpty()) {
        fileDialog->m_fileWidget->setFilter(nameFilters.join(QLatin1Char('\n')));
    }

    if (!mimeTypeFilters.isEmpty()) {
        fileDialog->m_fileWidget->setMimeFilter(mimeTypeFilters);
    }

    if (fileDialog->exec() == QDialog::Accepted) {
        QStringList files;
        for (const QString &filename : fileDialog->m_fileWidget->selectedFiles()) {
           QUrl url = QUrl::fromLocalFile(filename);
           files << url.toDisplayString();
        }
        results.insert(QStringLiteral("uris"), files);
        return 0;
    }

    return 1;
}

uint FileChooserPortal::SaveFile(const QDBusObjectPath &handle,
                           const QString &app_id,
                           const QString &parent_window,
                           const QString &title,
                           const QVariantMap &options,
                           QVariantMap &results)
{
    Q_UNUSED(app_id);

    qCDebug(XdgDesktopPortalKdeFileChooser) << "SaveFile called with parameters:";
    qCDebug(XdgDesktopPortalKdeFileChooser) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdeFileChooser) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdeFileChooser) << "    title: " << title;
    qCDebug(XdgDesktopPortalKdeFileChooser) << "    options: " << options;

    bool modalDialog = true;
    QString acceptLabel;
    QString currentName;
    QString currentFolder;
    QString currentFile;
    QStringList nameFilters;
    QStringList mimeTypeFilters;

    // TODO parse options - choices

    if (options.contains(QStringLiteral("modal"))) {
        modalDialog = options.value(QStringLiteral("modal")).toBool();
    }

    if (options.contains(QStringLiteral("accept_label"))) {
        acceptLabel = options.value(QStringLiteral("accept_label")).toString();
    }

    if (options.contains(QStringLiteral("current_name"))) {
        currentName = options.value(QStringLiteral("current_name")).toString();
    }

    if (options.contains(QStringLiteral("current_folder"))) {
        currentFolder = QFile::decodeName(options.value(QStringLiteral("current_folder")).toByteArray());
    }

    if (options.contains(QStringLiteral("current_file"))) {
        currentFile = QFile::decodeName(options.value(QStringLiteral("current_file")).toByteArray());
    }

    if (options.contains(QStringLiteral("filters"))) {
        FilterListList filterListList = qdbus_cast<FilterListList>(options.value(QStringLiteral("filters")));
        for (const FilterList &filterList : filterListList) {
            QStringList filterStrings;
            for (const Filter &filterStruct : filterList.filters) {
                if (filterStruct.type == 0) {
                    filterStrings << filterStruct.filterString;
                } else {
                    mimeTypeFilters << filterStruct.filterString;
                }
            }

            if (!filterStrings.isEmpty()) {
                nameFilters << QStringLiteral("%1|%2").arg(filterStrings.join(QLatin1Char(' '))).arg(filterList.userVisibleName);
            }
        }
    }

    QScopedPointer<FileDialog, QScopedPointerDeleteLater> fileDialog(new FileDialog());
    Utils::setParentWindow(fileDialog.data(), parent_window);
    fileDialog->setWindowTitle(title);
    fileDialog->setModal(modalDialog);
    fileDialog->m_fileWidget->setOperationMode(KFileWidget::Saving);
    fileDialog->m_fileWidget->setConfirmOverwrite(true);

    if (!currentFolder.isEmpty()) {
        fileDialog->m_fileWidget->setUrl(QUrl::fromLocalFile(currentFolder));
    }

    if (!currentFile.isEmpty()) {
        fileDialog->m_fileWidget->setSelectedUrl(QUrl::fromLocalFile(currentFile));
    }

    if (!currentName.isEmpty()) {
        const QUrl url = fileDialog->m_fileWidget->baseUrl();
        fileDialog->m_fileWidget->setSelectedUrl(QUrl::fromLocalFile(QStringLiteral("%1/%2").arg(url.toDisplayString(QUrl::StripTrailingSlash), currentName)));
    }

    if (!acceptLabel.isEmpty()) {
        fileDialog->m_fileWidget->okButton()->setText(acceptLabel);
    }

    if (!nameFilters.isEmpty()) {
        fileDialog->m_fileWidget->setFilter(nameFilters.join(QLatin1Char('\n')));
    }

    if (!mimeTypeFilters.isEmpty()) {
        fileDialog->m_fileWidget->setMimeFilter(mimeTypeFilters);
    }

    if (fileDialog->exec() == QDialog::Accepted) {
        QStringList files;
        QUrl url = QUrl::fromLocalFile(fileDialog->m_fileWidget->selectedFile());
        files << url.toDisplayString();
        results.insert(QStringLiteral("uris"), files);
        return 0;
    }

    return 1;
}

