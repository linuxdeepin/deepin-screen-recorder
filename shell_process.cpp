#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QObject>
#include "shell_process.h"

ShellProcess::ShellProcess(QObject *parent) : QThread(parent)  
{  
}  

void ShellProcess::setRecordInfo(int rx, int ry, int rw, int rh) 
{
    record_x = rx;
    record_y = ry;
    record_width = rw;
    record_height = rh;
}        

void ShellProcess::run()
{
    process = new QProcess();
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    
    QString path = "%1/Desktop/%2";
    QString savepath = path.arg(QDir::homePath()).arg("deepin-record.gif");
    
    QFile file(savepath);
    file.remove();
    
    QStringList arguments;
    arguments << QString("--x=%1").arg(record_x) << QString("--y=%1").arg(record_y);
    arguments << QString("--width=%1").arg(record_width) << QString("--height=%1").arg(record_height);
    arguments << savepath;
    
    process->start("byzanz-record", arguments);
    
    process->waitForFinished(-1);
    if (process->exitCode() !=0) {
        qDebug() << "error" << process->readAllStandardError();
    } else{
        qDebug() << "ok" << process->readAllStandardOutput() << process->readAllStandardError();
    }
}

void ShellProcess::stopRecord() {
    process->kill();
}
