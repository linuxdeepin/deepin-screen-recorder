#include <QThread>
#include <QDebug>
#include <QDate>
#include <QProcess>
#include <QtDBus>
#include <QDir>
#include <QStandardPaths>
#include <QObject>
#include "record_process.h"

RecordProcess::RecordProcess(QObject *parent) : QThread(parent)
{
}

void RecordProcess::setRecordInfo(int rx, int ry, int rw, int rh, QString name)
{
    recordX = rx;
    recordY = ry;
    recordWidth = rw;
    recordHeight = rh;
    saveAreaName = name;
}

void RecordProcess::setRecordType(int type)
{
    recordType = type;
}

void RecordProcess::run()
{
    // Start record.
    if (recordType == RECORD_TYPE_GIF) {
        recordGIF();
    } else if (recordType == RECORD_TYPE_VIDEO) {
        recordVideo();
    }

    // Got output or error.
    process->waitForFinished(-1);
    if (process->exitCode() !=0) {
        qDebug() << "Error";
        foreach (auto line, (process->readAllStandardError().split('\n'))) {
            qDebug() << line;
        }
    } else{
        qDebug() << "OK" << process->readAllStandardOutput() << process->readAllStandardError();
    }
}

void RecordProcess::recordGIF()
{
    process = new QProcess();
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));

    QDateTime date = QDateTime::currentDateTime();
    QString path = "%1/Desktop/%2_%3_%4.gif";
    savePath = path.arg(QDir::homePath()).arg("deepin-record").arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss"));
    
    QFile file(savePath);
    file.remove();

    QStringList arguments;
    arguments << QString("--duration=%1").arg(864000);
    arguments << QString("--x=%1").arg(recordX) << QString("--y=%1").arg(recordY);
    arguments << QString("--width=%1").arg(recordWidth) << QString("--height=%1").arg(recordHeight);
    arguments << savePath;

    process->start("byzanz-record", arguments);
}

void RecordProcess::recordVideo()
{
    process = new QProcess();
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));

    QDateTime date = QDateTime::currentDateTime();
    QString path = "%1/Desktop/%2_%3_%4.mp4";
    savePath = path.arg(QDir::homePath()).arg("deepin-record").arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss"));

    QFile file(savePath);
    file.remove();

    // FFmpeg need pass arugment split two part: -option value,
    // otherwise, it will report 'Unrecognized option' error.
    QStringList arguments;
    arguments << QString("-video_size");
    arguments << QString("%1x%2").arg(recordWidth).arg(recordHeight);
    arguments << QString("-framerate");
    arguments << QString("25");
    arguments << QString("-f");
    arguments << QString("x11grab");
    arguments << QString("-i");
    arguments << QString(":0.0+%1,%2").arg(recordX).arg(recordY);
    arguments << savePath;

    process->start("ffmpeg", arguments);
}

void RecordProcess::stopRecord()
{
    // Exit record process.
    process->terminate();

    // Popup notify.
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());

    QStringList actions;
    actions << "id_open" << "view";
    
    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                                    // appname
        << ((unsigned int) 0)                                                       // id
        << QString("%1/image/deepin-recorder.svg").arg(qApp->applicationDirPath())  // icon
        << "Record successful"                                                      // summary
        << QString("Save at: %1").arg(savePath)                                     // body
        << actions                                                                  // actions
        << QVariantMap()                                                            // hints
        << (int) -1;                                                                // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
}
