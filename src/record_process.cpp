#include <QThread>
#include <QApplication>
#include <QDebug>
#include <QDate>
#include <QProcess>
#include <QtDBus>
#include <QDir>
#include <QStandardPaths>
#include <QObject>
#include "record_process.h"
#include "utils.h"

RecordProcess::RecordProcess(QObject *parent) : QThread(parent)
{
    saveTempDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();
    saveDir = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
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
    recordType == RECORD_TYPE_GIF ? recordGIF() : recordVideo();

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
    initProcess();
    
    QStringList arguments;
    arguments << QString("--duration=%1").arg(864000);
    arguments << QString("--x=%1").arg(recordX) << QString("--y=%1").arg(recordY);
    arguments << QString("--width=%1").arg(recordWidth) << QString("--height=%1").arg(recordHeight);
    arguments << savePath;

    process->start("byzanz-record", arguments);
}

void RecordProcess::recordVideo()
{
    initProcess();
    
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

void RecordProcess::initProcess() {
    // Create process and handle finish singal.
    process = new QProcess();
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));

    // Build temp save path.
    QDateTime date = QDateTime::currentDateTime();
    saveBaseName = QString("%1_%2_%3.%4").arg("deepin-record").arg(saveAreaName).arg(date.toString("yyyyMMddhhmmss")).arg(recordType == RECORD_TYPE_GIF ? "gif" : "mp4");
    savePath = QDir(saveTempDir).filePath(saveBaseName);
    
    // Remove same cache file first.
    QFile file(savePath);
    file.remove();
}

void RecordProcess::stopRecord()
{
    // Exit record process.
    process->terminate();
    
    // Wait thread.
    wait();

    // Popup notify.
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());

    QStringList actions;
    actions << "_open" << "Open";
    
    QString newSavePath = QDir(saveDir).filePath(saveBaseName);
    QFile::rename(savePath, newSavePath);
    
    QVariantMap hints;
    hints["x-deepin-action-_open"] = QString("xdg-open,%1").arg(newSavePath);
    
    
    QList<QVariant> arg;
    arg << (QCoreApplication::applicationName())                                    // appname
        << ((unsigned int) 0)                                                       // id
        << Utils::getImagePath("deepin-recorder.svg")                               // icon
        << "Record successful"                                                      // summary
        << QString("Save at: %1").arg(newSavePath)                                  // body
        << actions                                                                  // actions
        << hints                                                                    // hints
        << (int) -1;                                                                // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
    
    QApplication::quit();
}
