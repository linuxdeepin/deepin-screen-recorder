#include "proxyaudioport.h"
#include "utils/log.h"

#include <QDBusMetaType>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

void registerAudioPortMetaType()
{
    qCDebug(dsrApp) << "Registering AudioPort metatype.";
    qRegisterMetaType<AudioPort>("AudioPort");
    qDBusRegisterMetaType<AudioPort>();
}

#endif
