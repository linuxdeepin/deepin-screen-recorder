#include <QThread>
#include <QProcess>
#include <QObject>

class ShellProcess : public QThread
{
    Q_OBJECT  

public:
    ShellProcess(QObject *parent = 0);
    void setRecordInfo(int record_x, int record_y, int record_width, int record_height);
    void stopRecord();
    
protected:  
    void run();      
    
private:    
    QProcess* process;
    
    int record_x;
    int record_y;
    int record_width;
    int record_height;
};             

