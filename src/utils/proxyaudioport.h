#ifndef PROXYAUDIOPORT_H
#define PROXYAUDIOPORT_H

#include <QDBusMetaType>
#include <QString>
#include <QDBusArgument>
#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include </usr/include/libdframeworkdbus-2.0/types/audioport.h>
#else

class AudioPort
{
public:
    QString name;
    QString description;
    uchar availability;  // 0 for Unknown, 1 for Not Available, 2 for Available.

    friend QDebug operator<<(QDebug argument, const AudioPort &port)
    {
        argument << port.description;

        return argument;
    }

    friend QDBusArgument &operator<<(QDBusArgument &argument, const AudioPort &port)
    {
        argument.beginStructure();
        argument << port.name << port.description << port.availability;
        argument.endStructure();

        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, AudioPort &port)
    {
        argument.beginStructure();
        argument >> port.name >> port.description >> port.availability;
        argument.endStructure();

        return argument;
    }

    bool operator==(const AudioPort what) const
    {
        return what.name == name && what.description == description && what.availability == availability;
    }

    bool operator!=(const AudioPort what) const
    {
        return what.name != name || what.description != description || what.availability != availability;
    }
};

Q_DECLARE_METATYPE(AudioPort)

void registerAudioPortMetaType();

#endif

#endif  // PROXYAUDIOPORT_H
