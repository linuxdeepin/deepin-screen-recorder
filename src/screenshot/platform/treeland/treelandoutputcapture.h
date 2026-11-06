// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELAND_OUTPUT_CAPTURE_H
#define TREELAND_OUTPUT_CAPTURE_H

#include <QImage>
#include <QRect>
#include <QString>

#include <cstdint>

namespace Dsr::Screenshot {

/**
 * Captures the selected output through ext-image-copy-capture.
 *
 * The capture session is created with no capture options, so the compositor
 * does not paint the pointer into the frame.  Callers crop the returned output
 * image to the requested scroll selection.  This path deliberately does not
 * use the foreign-toplevel source: TreeLand's toplevel source reuses the
 * surface texture-provider buffer and is not safe while the client is
 * replacing buffers during scrolling.
 */
class TreelandOutputCapture
{
public:
    TreelandOutputCapture();
    ~TreelandOutputCapture();

    bool start(const QString &outputName,
               const QRect &outputGeometry,
               QString *error);
    bool capture(QImage *image, quint64 *timestampNs, QString *error);
    void stop();

    class Private;

private:
    Private *d = nullptr;
};

} // namespace Dsr::Screenshot

#endif // TREELAND_OUTPUT_CAPTURE_H
