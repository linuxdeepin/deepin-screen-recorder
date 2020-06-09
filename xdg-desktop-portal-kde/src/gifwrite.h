#ifndef GIFWRITE_H
#define GIFWRITE_H

#include <QMap>
#include <QMutex>
#include <QThread>
#include "waylandintegration.h"
#include "waylandintegration_p.h"
#include "gif.h"

class GifWrite : public QThread
{
    Q_OBJECT
public:
    GifWrite(WaylandIntegration::WaylandIntegrationPrivate* context,QObject *parent = nullptr);

    ~GifWrite();

    void init(int width,
              int height,
              int delay);

    void run();

    void insertFrame(GifFrame frame);
    GifFrame getFrame();
    int getCacheSize();

    bool getBCache();
    void setBCache(bool bCache);

private:
    WaylandIntegration::WaylandIntegrationPrivate * m_context;
    QMap<uint32_t,GifFrame> m_cacheMap;
    QMutex m_cacheMutex;
    QMutex m_bCacheMutex;
    bool m_bCache;

};
#endif // GIFWRITE_H
