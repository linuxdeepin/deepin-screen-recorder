#ifndef UTILS_H
#define UTILS_H
#include <QObject>

class Utils :public QObject
{
public:
    static bool isWaylandMode;
};

#endif // UTILS_H
