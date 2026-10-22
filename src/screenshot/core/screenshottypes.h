// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHOT_TYPES_H
#define SCREENSHOT_TYPES_H

#include <QImage>
#include <QList>
#include <QMetaType>
#include <QRect>
#include <QString>

namespace Dsr::Screenshot {

enum class Mode {
    Region,
    FullScreen,
    TopWindow
};

enum class CursorIntent {
    Default,
    Cross,
    Move,
    ResizeHorizontal,
    ResizeVertical,
    ResizeDiagonalForward,
    ResizeDiagonalBackward
};

struct BackendCapabilities {
    bool frozenFrame = false;
    bool compositorSnap = false;
    bool multiOutputMasks = false;
    bool topWindowCapture = false;
    bool cursorInFrame = false;
};

struct Request {
    Mode mode = Mode::Region;
    bool includeCursor = false;
    bool enableWindowSnap = true;
    QRect initialRegion;
    QString outputPath;
    QByteArray outputFormat;
};

struct OutputDescriptor {
    QString outputId;
    QRect logicalGeometry;
    qreal devicePixelRatio = 1.0;
};

struct OutputFrame {
    OutputDescriptor output;
    QImage image;
    qreal pixelScale = 1.0;
    quint64 timestampNs = 0;
};

struct FrozenFrameSet {
    quint64 generation = 0;
    QList<OutputFrame> outputs;
    QRect virtualDesktopRect;

    bool isEmpty() const
    {
        return outputs.isEmpty();
    }
};

struct Result {
    QImage image;
    QRect globalSelection;
    quint64 frameGeneration = 0;
    QList<OutputDescriptor> outputs;

    bool isValid() const
    {
        return !image.isNull() && !globalSelection.isEmpty();
    }
};

} // namespace Dsr::Screenshot

Q_DECLARE_METATYPE(Dsr::Screenshot::FrozenFrameSet)
Q_DECLARE_METATYPE(Dsr::Screenshot::Result)

#endif // SCREENSHOT_TYPES_H
