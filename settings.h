#include <QObject>
#include <QSettings>

class Settings : public QObject 
{
    Q_OBJECT
    
public:
    Settings(QObject *parent = 0);
    QString configPath();
    
    QVariant option(const QString &key);
    void setOption(const QString &key, const QVariant &value);
    
private:
    QSettings* settings;
    QString groupName;
};

