#include <QThread>
#include <QProcess>
#include <QObject>

class RecordProcess : public QThread
{
    Q_OBJECT

public:
    RecordProcess(QObject *parent = 0);
    void setRecordInfo(int recordX, int recordY, int record_width, int recordHeight);
    void stopRecord();

protected:
    void run();

private:
    QProcess* process;

    int recordX;
    int recordY;
    int recordWidth;
    int recordHeight;
};
