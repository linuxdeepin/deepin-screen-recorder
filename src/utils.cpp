#include <QString>
#include <QDir>
#include <QApplication>
#include "utils.h"

QString Utils::getImagePath(QString imageName)
{
    QDir dir(qApp->applicationDirPath());
    dir.cdUp();
    
    return QDir(dir.filePath("image")).filePath(imageName);
}
