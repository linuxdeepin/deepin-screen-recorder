// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_CLIPBOARD_HOLDER_H
#define TREELAND_CLIPBOARD_HOLDER_H

#include <QString>

namespace Dsr::Screenshot {

// Internal helper mode. It publishes the clipboard payload and stays alive
// until the compositor or clipboard manager takes ownership.
int runTreelandClipboardHolder(const QString &kind,
                               const QString &path,
                               const QString &readyFile);

// Starts the helper process and waits until the data-control selection has
// been published. Supported kinds are "file" and "image".
bool launchTreelandClipboardHolder(const QString &kind,
                                   const QString &path,
                                   QString *error = nullptr);

} // namespace Dsr::Screenshot

#endif // TREELAND_CLIPBOARD_HOLDER_H
