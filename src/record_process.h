#include <QThread>
#include <QProcess>
#include <QObject>

class RecordProcess : public QThread
{
    Q_OBJECT
    
    
public:
    static const int RECORD_TYPE_VIDEO = 0;
    static const int RECORD_TYPE_GIF = 1;
    
    RecordProcess(QObject *parent = 0);
    void setRecordInfo(int recordX, int recordY, int record_width, int recordHeight, QString areaName);
    void setRecordType(int recordType);
    void stopRecord();
    void recordGIF();
    void recordVideo();
    void initProcess();

protected:
    void run();

private:
    QProcess* process;

    int recordX;
    int recordY;
    int recordWidth;
    int recordHeight;
    int recordType;
    
    QString savePath;
    QString saveBaseName;
    QString saveTempDir;
    QString saveDir;
    QString saveAreaName;
};
