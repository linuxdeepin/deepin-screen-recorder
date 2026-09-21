// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREELANDFULLSCREENGRABBER_H
#define TREELANDFULLSCREENGRABBER_H

#include <QImage>
#include <QObject>

class QScreen;

/**
 * @brief Non-interactive full-screen grab for Treeland.
 *
 * The treeland_capture_unstable_v1 selector always waits for user input, so
 * the fullscreen (Print key / D-Bus FullscreenScreenshot) path grabs each
 * output through ext-image-copy-capture-v1 with an
 * ext_output_image_capture_source_manager_v1 source instead. Frames are
 * requested in SHM buffers because screenshot consumers need CPU-readable
 * pixels (the DMA-BUF path used for recording is tiled).
 */
class TreelandFullScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit TreelandFullScreenGrabber(QObject *parent = nullptr);
    ~TreelandFullScreenGrabber() override;

    /**
     * @brief Grab every screen and composite them into one image.
     * @return Image in output pixels, or a null image on failure/timeout.
     */
    QImage grab(int timeoutMs = 15000);

private:
    QImage grabOutput(QScreen *screen, int timeoutMs);

    class Private;
    Private *d;
};

#endif // TREELANDFULLSCREENGRABBER_H
