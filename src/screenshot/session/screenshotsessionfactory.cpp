// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenshotsessionfactory.h"

#include "screenshotsessioncontroller.h"
#include "../export/localscreenshotresultsink.h"
#include "../platform/treeland/treelandmaskoverlayhost.h"
#include "../platform/treeland/treelandscreenshotbackend.h"
#include "../platform/treeland/treelandsnaphintsource.h"
#include "../platform/treeland/treelandsnapproxy.h"
#include "../platform/treeland/treelandwindowtagger.h"
#include "../platform/treeland/treelandrecordingsession.h"

#include <QGuiApplication>
#include <QProcessEnvironment>

namespace Dsr::Screenshot {

namespace {

bool isTreelandSession()
{
    if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland"),
                                                     Qt::CaseInsensitive)) {
        return false;
    }
    return QProcessEnvironment::systemEnvironment()
            .value(QStringLiteral("DDE_CURRENT_COMPOSITOR")) == QStringLiteral("TreeLand");
}

} // namespace

bool ScreenshotSessionFactory::isCurrentPlatformSupported()
{
    return isTreelandSession();
}

ScreenshotSessionController *ScreenshotSessionFactory::createForCurrentPlatform(QObject *parent,
                                                                                QString *error)
{
    if (!isTreelandSession()) {
        if (error)
            *error = QStringLiteral("当前平台没有可用的截图会话后端");
        return nullptr;
    }

    auto *snapSession = new TreelandSnapSession();
    auto *backend = new TreelandScreenshotBackend(snapSession);
    auto *snapHints = new TreelandSnapHintSource(snapSession);
    auto *windowTagger = new TreelandWindowTagger(snapSession);
    auto *overlay = new TreelandMaskOverlayHost(windowTagger);
    auto *resultSink = new LocalScreenshotResultSink();
    auto *annotationEditor = overlay->annotationEditor();

    auto *controller = new ScreenshotSessionController(backend,
                                                       overlay,
                                                       snapHints,
                                                       windowTagger,
                                                       resultSink,
                                                       annotationEditor,
                                                       parent);
    controller->setRecordingSession(new TreelandRecordingSession);
    snapSession->setParent(controller);
    return controller;
}

} // namespace Dsr::Screenshot
