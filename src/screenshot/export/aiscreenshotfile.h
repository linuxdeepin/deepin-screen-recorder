// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QImage>
#include <QTemporaryFile>

#include <memory>

namespace Dsr::Screenshot {

inline std::unique_ptr<QTemporaryFile> saveAiScreenshotFile(const QImage &image, int quality = -1)
{
    if (image.isNull())
        return {};

    auto file = std::make_unique<QTemporaryFile>(
            QDir::temp().filePath(QStringLiteral("deepin-ai-screenshot-XXXXXX.png")));
    if (!file->open() || !image.save(file.get(), "PNG", quality) || !file->flush())
        return {};

    file->close();
    return file;
}

inline QString retainAiScreenshotPath(QTemporaryFile &file)
{
    // Copilot receives a path and may read it after the screenshot process exits.
    file.setAutoRemove(false);
    return file.fileName();
}

} // namespace Dsr::Screenshot
