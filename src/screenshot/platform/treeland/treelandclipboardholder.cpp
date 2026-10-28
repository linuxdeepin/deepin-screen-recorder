// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandclipboardholder.h"

#include "protocols/data-control/ext-data-control-v1-client-protocol.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QHash>
#include <QList>
#include <QProcess>
#include <QStandardPaths>
#include <QThread>
#include <QUrl>

#include <wayland-client.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace Dsr::Screenshot {

namespace {

struct ClipboardState {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    wl_seat *seat = nullptr;
    ext_data_control_manager_v1 *manager = nullptr;
    ext_data_control_device_v1 *device = nullptr;
    ext_data_control_source_v1 *source = nullptr;
    QHash<QByteArray, QByteArray> payloads;
    bool selectionCancelled = false;
    bool deviceFinished = false;
};

void writeReadyFile(const QString &path, const QByteArray &content)
{
    if (path.isEmpty())
        return;
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        file.write(content);
}

void sourceSend(void *data,
                ext_data_control_source_v1 *,
                const char *mimeType,
                int32_t fd)
{
    auto *state = static_cast<ClipboardState *>(data);
    const QByteArray request = QByteArray::fromRawData(
            mimeType ? mimeType : "", mimeType ? int(std::strlen(mimeType)) : 0);
    const QByteArray payload = state->payloads.value(request);

    qsizetype written = 0;
    while (written < payload.size()) {
        const ssize_t result = write(fd,
                                     payload.constData() + written,
                                     size_t(payload.size() - written));
        if (result < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        if (result == 0)
            break;
        written += result;
    }
    close(fd);
}

void sourceCancelled(void *data, ext_data_control_source_v1 *)
{
    static_cast<ClipboardState *>(data)->selectionCancelled = true;
}

const ext_data_control_source_v1_listener kSourceListener{
        sourceSend,
        sourceCancelled,
};

void deviceDataOffer(void *,
                     ext_data_control_device_v1 *,
                     ext_data_control_offer_v1 *offer)
{
    if (offer)
        ext_data_control_offer_v1_destroy(offer);
}

void deviceSelection(void *,
                     ext_data_control_device_v1 *,
                     ext_data_control_offer_v1 *offer)
{
    if (offer)
        ext_data_control_offer_v1_destroy(offer);
}

void deviceFinished(void *data, ext_data_control_device_v1 *)
{
    static_cast<ClipboardState *>(data)->deviceFinished = true;
}

void devicePrimarySelection(void *,
                            ext_data_control_device_v1 *,
                            ext_data_control_offer_v1 *offer)
{
    if (offer)
        ext_data_control_offer_v1_destroy(offer);
}

const ext_data_control_device_v1_listener kDeviceListener{
        deviceDataOffer,
        deviceSelection,
        deviceFinished,
        devicePrimarySelection,
};

void registryGlobal(void *data,
                    wl_registry *registry,
                    uint32_t name,
                    const char *interface,
                    uint32_t version)
{
    auto *state = static_cast<ClipboardState *>(data);
    if (std::strcmp(interface, ext_data_control_manager_v1_interface.name) == 0
            && !state->manager) {
        state->manager = static_cast<ext_data_control_manager_v1 *>(
                wl_registry_bind(registry,
                                 name,
                                 &ext_data_control_manager_v1_interface,
                                 qMin(version, 1u)));
    } else if (std::strcmp(interface, wl_seat_interface.name) == 0
               && !state->seat) {
        state->seat = static_cast<wl_seat *>(
                wl_registry_bind(registry, name, &wl_seat_interface, qMin(version, 1u)));
    }
}

void registryGlobalRemove(void *, wl_registry *, uint32_t)
{
}

const wl_registry_listener kRegistryListener{
        registryGlobal,
        registryGlobalRemove,
};

} // namespace

int runTreelandClipboardHolder(const QString &kind,
                               const QString &path,
                               const QString &readyFile)
{
    ClipboardState state;
    QList<QByteArray> offeredMimeTypes;
    if (kind == QStringLiteral("file")) {
        const QUrl url = QUrl::fromLocalFile(path);
        if (!url.isLocalFile()) {
            writeReadyFile(readyFile, QByteArrayLiteral("error:invalid path"));
            return 2;
        }
        state.payloads.insert(QByteArrayLiteral("text/uri-list"),
                              url.toEncoded() + '\n');
        state.payloads.insert(QByteArrayLiteral("x-special/gnome-copied-files"),
                              QByteArrayLiteral("copy\n") + url.toEncoded());
        offeredMimeTypes = {QByteArrayLiteral("text/uri-list"),
                            QByteArrayLiteral("x-special/gnome-copied-files")};
    } else if (kind == QStringLiteral("image")) {
        QFile image(path);
        if (!image.open(QIODevice::ReadOnly)) {
            writeReadyFile(readyFile, QByteArrayLiteral("error:cannot read image"));
            return 2;
        }
        state.payloads.insert(QByteArrayLiteral("image/png"), image.readAll());
        offeredMimeTypes = {QByteArrayLiteral("image/png")};
    } else {
        writeReadyFile(readyFile, QByteArrayLiteral("error:unsupported clipboard kind"));
        return 2;
    }

    // A receiver can close the pipe early. Keep the holder process alive so a
    // failed transfer cannot terminate it via SIGPIPE.
    std::signal(SIGPIPE, SIG_IGN);

    state.display = wl_display_connect(nullptr);
    if (!state.display) {
        std::fprintf(stderr, "TreeLand clipboard holder: cannot connect to Wayland\n");
        writeReadyFile(readyFile, QByteArrayLiteral("error:wayland connection failed"));
        return 3;
    }

    state.registry = wl_display_get_registry(state.display);
    wl_registry_add_listener(state.registry, &kRegistryListener, &state);
    if (wl_display_roundtrip(state.display) < 0) {
        writeReadyFile(readyFile, QByteArrayLiteral("error:wayland registry failed"));
        wl_display_disconnect(state.display);
        return 4;
    }

    if (!state.manager || !state.seat) {
        std::fprintf(stderr,
                     "TreeLand clipboard holder: ext_data_control_manager_v1 "
                     "or wl_seat is unavailable\n");
        writeReadyFile(readyFile,
                       QByteArrayLiteral("error:data-control protocol unavailable"));
        wl_registry_destroy(state.registry);
        wl_display_disconnect(state.display);
        return 5;
    }

    state.device = ext_data_control_manager_v1_get_data_device(state.manager,
                                                               state.seat);
    ext_data_control_device_v1_add_listener(state.device, &kDeviceListener, &state);

    state.source = ext_data_control_manager_v1_create_data_source(state.manager);
    ext_data_control_source_v1_add_listener(state.source, &kSourceListener, &state);
    for (const QByteArray &mimeType : offeredMimeTypes)
        ext_data_control_source_v1_offer(state.source, mimeType.constData());
    ext_data_control_device_v1_set_selection(state.device, state.source);

    if (wl_display_roundtrip(state.display) < 0) {
        writeReadyFile(readyFile, QByteArrayLiteral("error:set selection failed"));
        ext_data_control_source_v1_destroy(state.source);
        ext_data_control_device_v1_destroy(state.device);
        ext_data_control_manager_v1_destroy(state.manager);
        wl_seat_release(state.seat);
        wl_registry_destroy(state.registry);
        wl_display_disconnect(state.display);
        return 6;
    }

    writeReadyFile(readyFile, QByteArrayLiteral("ready"));
    std::fprintf(stderr,
                 "TreeLand clipboard holder: %s selection published for %s\n",
                 kind.toLocal8Bit().constData(),
                 path.toLocal8Bit().constData());

    while (!state.selectionCancelled && !state.deviceFinished) {
        if (wl_display_dispatch(state.display) < 0)
            break;
    }

    ext_data_control_source_v1_destroy(state.source);
    ext_data_control_device_v1_destroy(state.device);
    ext_data_control_manager_v1_destroy(state.manager);
    wl_seat_release(state.seat);
    wl_registry_destroy(state.registry);
    wl_display_disconnect(state.display);
    return 0;
}

bool launchTreelandClipboardHolder(const QString &kind,
                                   const QString &path,
                                   QString *error)
{
    if (error)
        error->clear();

    const QString readyFile = QDir(QStandardPaths::writableLocation(
                                           QStandardPaths::TempLocation))
                                      .filePath(QStringLiteral("dsr-clipboard-%1-%2.ready")
                                                        .arg(QCoreApplication::applicationPid())
                                                        .arg(QDateTime::currentMSecsSinceEpoch()));
    QFile::remove(readyFile);

    qint64 pid = 0;
    const bool started = QProcess::startDetached(
            QCoreApplication::applicationFilePath(),
            {QStringLiteral("--clipboard-hold"), kind, path, readyFile},
            QDir::currentPath(),
            &pid);
    if (!started) {
        if (error)
            *error = QStringLiteral("无法启动剪贴板持有进程");
        return false;
    }

    QElapsedTimer elapsed;
    elapsed.start();
    QByteArray status;
    while (elapsed.elapsed() < 2000) {
        QFile file(readyFile);
        if (file.open(QIODevice::ReadOnly)) {
            status = file.readAll();
            break;
        }
        QThread::msleep(20);
    }
    QFile::remove(readyFile);

    if (!status.startsWith("ready")) {
        if (error) {
            *error = status.isEmpty()
                    ? QStringLiteral("剪贴板持有进程未就绪")
                    : QString::fromUtf8(status);
        }
        return false;
    }

    qInfo("TreeLand clipboard holder ready: %lld", static_cast<long long>(pid));
    return true;
}

} // namespace Dsr::Screenshot
