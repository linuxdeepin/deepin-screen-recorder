/*
 * Copyright © 2018 Red Hat, Inc
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

#ifndef XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_H
#define XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_H

#include <QObject>
#include <QSize>
#include <QVariant>

#include <QtGlobal>

namespace WaylandIntegration
{

class WaylandOutput
{
public:
    enum OutputType {
        Laptop,
        Monitor,
        Television
    };
    void setManufacturer(const QString &manufacturer) { m_manufacturer = manufacturer; }
    QString manufacturer() const { return m_manufacturer; }

    void setModel(const QString &model) { m_model = model; }
    QString model() const { return m_model; }

    void setResolution(const QSize &resolution) { m_resolution = resolution; }
    QSize resolution() const { return m_resolution; }

    void setOutputType(const QString &type);
    OutputType outputType() const { return m_outputType; }

    void setWaylandOutputName(int outputName) { m_waylandOutputName = outputName; }
    int waylandOutputName() const { return m_waylandOutputName; }

    void setWaylandOutputVersion(int outputVersion) { m_waylandOutputVersion = outputVersion; }
    int waylandOutputVersion() const { return m_waylandOutputVersion; }

private:
    QString m_manufacturer;
    QString m_model;
    QSize m_resolution;
    OutputType m_outputType;

    // Needed for later output binding
    int m_waylandOutputName;
    int m_waylandOutputVersion;
};

class WaylandIntegration : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void newBuffer(uint8_t *screenData);
};

    void init(int &argc, char **argv);

    bool isEGLInitialized();

    //bool startStreaming(const WaylandOutput &output);
    void stopStreaming();

    QMap<quint32, WaylandOutput> screens();
    QVariant streams();

    WaylandIntegration *waylandIntegration();

}

#endif // XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_H


