#include <QObject>
#include <QString>

class Utils : public QObject
{
public:
    static QString getImagePath(QString imageName);
};
