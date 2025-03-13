#include "proxyaudioport.h"

#include <QDBusMetaType>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

void registerAudioPortMetaType()
{
    qRegisterMetaType<AudioPort>("AudioPort");
    qDBusRegisterMetaType<AudioPort>();
}

#endif
