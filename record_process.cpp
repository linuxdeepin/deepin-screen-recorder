#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QObject>
#include "record_process.h"

RecordProcess::RecordProcess(QObject *parent) : QThread(parent)  
{  
}  

void RecordProcess::setRecordInfo(int rx, int ry, int rw, int rh) 
{
    recordX = rx;
    recordY = ry;
    recordWidth = rw;
    recordHeight = rh;
}        

void RecordProcess::run()
{
    process = new QProcess();
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    
    QString path = "%1/Desktop/%2";
    QString savepath = path.arg(QDir::homePath()).arg("deepin-record.gif");
    
    QFile file(savepath);
    file.remove();
    
    QStringList arguments;
    arguments << QString("--x=%1").arg(recordX) << QString("--y=%1").arg(recordY);
    arguments << QString("--width=%1").arg(recordWidth) << QString("--height=%1").arg(recordHeight);
    arguments << savepath;
    
    process->start("byzanz-record", arguments);
    
    process->waitForFinished(-1);
    if (process->exitCode() !=0) {
        qDebug() << "error" << process->readAllStandardError();
    } else{
        qDebug() << "ok" << process->readAllStandardOutput() << process->readAllStandardError();
    }
}

void RecordProcess::stopRecord() {
    process->kill();
}
