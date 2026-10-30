// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHOT_EXPORT_SERVICE_H
#define SCREENSHOT_EXPORT_SERVICE_H

#include "../core/screenshottypes.h"

#include <QRect>

namespace Dsr::Screenshot {

class IAnnotationEditor;

class ScreenshotExportService
{
public:
    Result render(const FrozenFrameSet &frames,
                  const QRect &globalSelection,
                  IAnnotationEditor *annotations = nullptr,
                  int roundedCornerRadius = 0) const;
};

} // namespace Dsr::Screenshot

#endif // SCREENSHOT_EXPORT_SERVICE_H
