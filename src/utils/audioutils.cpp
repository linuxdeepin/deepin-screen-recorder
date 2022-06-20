/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "audioutils.h"
#include "dbusutils.h"

#include <QDebug>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusError>
#include <QDBusMessage>

#include <com_deepin_daemon_audio.h>
#include <com_deepin_daemon_audio_sink.h>
#include <com_deepin_daemon_audio_source.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

AudioUtils::AudioUtils(QObject *parent)
{
    Q_UNUSED(parent);
}

/*
 * never used
void AudioUtils::initProcess()
{
    // Create process and handle finish signal.
    process = new QProcess();

    // Disable scaling of byzanz-record (GTK3 based) here, because we pass subprocesses
    // absolute device geometry information, byzanz-record should not scale the information
    // one more time.
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("GDK_SCALE", "1");
    process->setProcessEnvironment(env);

    //    `connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
}
bool AudioUtils::isSystemAudioOutput()
{
    char buf[1024] = {0};
    int fd[2];
    int backfd;
    pipe(fd);
    backfd = dup(STDOUT_FILENO); //备份标准输出，用于恢复
    dup2(fd[1], STDOUT_FILENO); //将标准输出重定向到fd[1]
    system("pacmd list-sinks | grep -A1 \"* index\" | grep \"alsa_output.platform-snd_aloop.0.analog-stereo\" | wc -l");
    read(fd[0], buf, 1024);
    dup2(backfd, STDOUT_FILENO); //恢复标准输出
    QString str_output = buf;
    qDebug() << str_output;
    return str_output.startsWith("1");
}
*/
/*
 * never used
bool AudioUtils::isMicrophoneOutput()
{
    char buf[1024] = {0};
    int fd[2];
    int backfd;
    pipe(fd);
    backfd = dup(STDOUT_FILENO); //备份标准输出，用于恢复
    dup2(fd[1], STDOUT_FILENO); //将标准输出重定向到fd[1]
    system("pacmd list-sinks | grep -A1 \"* index\" | grep \"alsa_output.pci-0000_00_1f.4.analog-stereo\" | wc -l");
    read(fd[0], buf, 1024);
    dup2(backfd, STDOUT_FILENO); //恢复标准输出
    QString str_output = buf;
    qDebug() << str_output;
    return str_output.startsWith("1");
}
void AudioUtils::setupMicrophoneOutput()
{
    initProcess();
    QStringList arguments;
    arguments << QString("set-default-sink");
    arguments << QString("alsa_output.pci-0000_00_1f.4.analog-stereo");
    process->start("pactl", arguments);
}

void AudioUtils::setupSystemAudioOutput()
{
    initProcess();
    QStringList arguments;
    arguments << QString("set-default-sink");
    arguments << QString("alsa_output.platform-snd_aloop.0.analog-stereo");
    process->start("pactl", arguments);
}

void AudioUtils::setupAudioSink(QString sink)
{
    initProcess();
    QStringList arguments;
    arguments << QString("set-default-sink");
    arguments << sink;
    process->start("pacmd", arguments);
}
*/
//bool AudioUtils::canVirtualCardOutput()
//{
//    QStringList options;
//    options << "-c";
//    options << "pacmd list-sinks | sed  -n '/.*index:.*\\([0-9]\\+\\).*/{n;p}' |  sed -n 's/name: <\\(.*\\)>/\\1/p' | grep 'alsa_output.platform-snd_aloop.0.analog-stereo' | wc -l";
//    QProcess process;
//    process.start("bash", options);
//    process.waitForFinished();
//    process.waitForReadyRead();
//    QString str_output = process.readAllStandardOutput();
//    process.close();
//    return str_output.startsWith("1");
//}
/*
 * never used
bool AudioUtils::canMicrophoneInput()
{
    QVariant v = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", "/com/deepin/daemon/Audio",
                                            "com.deepin.daemon.Audio", "DefaultSource");
    if (v.isValid()) {
        QDBusObjectPath path = v.value<QDBusObjectPath>();
        //qDebug() <<"path: "<<path.path();
        QDBusInterface ainterface("com.deepin.daemon.Audio", path.path(),
                                  "com.deepin.daemon.Audio.Source",
                                  QDBusConnection::sessionBus());
        if (!ainterface.isValid()) {
            return false;
        }
        //调用远程的value方法
        QDBusReply<QDBusObjectPath> reply = ainterface.call("GetMeter");
        if (reply.isValid()) {
            path = reply.value();
            //qDebug()<<"path1" << path.path();
            QVariant vv = DBusUtils::redDBusProperty("com.deepin.daemon.Audio", path.path(),
                                                    "com.deepin.daemon.Audio.Meter", "Volume");
            if (vv.isValid()) {
                double volume = vv.toDouble();
                //qDebug()<<"volume:" <<volume;
                if (0.0001 < volume) {
                    return true;
                } else {
                    return false;
                }
                //return volume != 0.0;
            }
        } else {
            return  false;
        }
    }
    return false;

}
*/
// never used
//QString AudioUtils::currentAudioSink()
//{
//    QStringList options;
//    options << "-c";
//    options << "pacmd list-sinks | sed  -n '/\\*.*index:.*\\([0-9]\\+\\).*/{n;p}' | sed -n 's/name: <\\(.*\\)>/\\1/p' | sed -e 's/^[\\t]*//'";
//    QProcess process;
//    process.start("bash", options);
//    process.waitForFinished();
//    process.waitForReadyRead();
//    QString str_output = process.readAllStandardOutput();
//    process.close();
//    return str_output;
//}

//never used
//QString AudioUtils::currentAudioSource()
//{
//    QStringList options;
//    options << "-c";
//    options << "pacmd list-sources | sed  -n '/\\*.*index:.*\\([0-9]\\+\\).*/{n;p}' | sed -n 's/name: <\\(.*\\)>/\\1/p' | sed -e 's/^[\\t]*//'";
//    QProcess process;
//    process.start("bash", options);
//    process.waitForFinished();
//    process.waitForReadyRead();
//    QString str_output = process.readAllStandardOutput();
//    process.close();
//    return str_output;
//}
QString AudioUtils::currentAudioChannel()
{
    const QString serviceName {"com.deepin.daemon.Audio"};

    QScopedPointer<com::deepin::daemon::Audio> audioInterface;
    QScopedPointer<com::deepin::daemon::audio::Sink> defaultSink;

    audioInterface.reset(
        new com::deepin::daemon::Audio(
            serviceName,
            "/com/deepin/daemon/Audio",
            QDBusConnection::sessionBus(),
            this)
    );

    defaultSink.reset(
        new com::deepin::daemon::audio::Sink(
            serviceName,
            audioInterface->defaultSink().path(),
            QDBusConnection::sessionBus(),
            this)
    );
    if (defaultSink->isValid()) {
        QString sinkName = defaultSink->name();
        QStringList options;
        options << "-c";
        options << QString("pacmd list-sources | grep -PB 1 %1 | head -n 1 | perl -pe 's/.* //g'").arg(sinkName);

        QProcess process;
        process.start("bash", options);
        process.waitForFinished();
        process.waitForReadyRead();
        QString str_output = process.readAllStandardOutput();
        qDebug() << options << str_output;
        return str_output;
    }
    return "";
}

QString AudioUtils::getDefaultDeviceName(DefaultAudioType mode)
{
    QString device = "";
    const QString serviceName {"com.deepin.daemon.Audio"};

    QScopedPointer<com::deepin::daemon::Audio> audioInterface;
    audioInterface.reset(
        new com::deepin::daemon::Audio(
            serviceName,
            "/com/deepin/daemon/Audio",
            QDBusConnection::sessionBus(),
            this)
    );
    if (mode == DefaultAudioType::Sink) {
        //1.首先取出默认系统声卡
        QScopedPointer<com::deepin::daemon::audio::Sink> defaultSink;
        defaultSink.reset(
            new com::deepin::daemon::audio::Sink(
                serviceName,
                audioInterface->defaultSink().path(),
                QDBusConnection::sessionBus(),
                this)
        );
        if (defaultSink->isValid()) {
            qInfo() << "default sink name is : " << defaultSink->name();
            qInfo() << "default sink activePort name : " << defaultSink->activePort().name;
            qInfo() << "             activePort description : " << defaultSink->activePort().description;
            qInfo() << "             activePort availability : " << defaultSink->activePort().availability;
            device = defaultSink->name();
            if (!device.isEmpty() && !device.endsWith(".monitor")) {
                device += ".monitor";
            }
        }
        //2.如果默认系统声卡不是物理声卡和蓝牙声卡，需找出真实的物理声卡
        if (!device.startsWith("alsa", Qt::CaseInsensitive) && !device.startsWith("blue", Qt::CaseInsensitive)) {
            QList<QScopedPointer<com::deepin::daemon::audio::Sink>> sinks;
            QScopedPointer<com::deepin::daemon::audio::Sink> realSink;
            for (int i = 0; i < audioInterface->sinks().size(); i++) {
                realSink.reset(
                    new com::deepin::daemon::audio::Sink(
                        serviceName,
                        audioInterface->sinks()[i].path(),
                        QDBusConnection::sessionBus(),
                        this)
                );
                if (realSink->isValid()) {
                    qInfo() << "realSink name is : " << realSink->name();
                    qInfo() << "realSink activePort name : " << realSink->activePort().name;
                    qInfo() << "             activePort description : " << realSink->activePort().description;
                    qInfo() << "             activePort availability : " << realSink->activePort().availability;
                    device = realSink->name();
                    if (device.startsWith("alsa", Qt::CaseInsensitive)) {
                        device += ".monitor";
                        break;
                    } else {
                        device = "";
                    }
                }
            }
        }
    } else if (mode == DefaultAudioType::Source) {
        QScopedPointer<com::deepin::daemon::audio::Source> defaultSource;
        defaultSource.reset(
            new com::deepin::daemon::audio::Source(
                serviceName,
                audioInterface->defaultSource().path(),
                QDBusConnection::sessionBus(),
                this)
        );
        if (defaultSource->isValid()) {
            qInfo() << "default source name is : " << defaultSource->name();
            qInfo() << "default source activePort name : " << defaultSource->activePort().name;
            qInfo() << "               activePort description : " << defaultSource->activePort().description;
            qInfo() << "               activePort availability : " << defaultSource->activePort().availability;
            device = defaultSource->name();
            if (device.endsWith(".monitor")) {
                device.clear();
            }
        }
    } else {
        qCritical() << "The passed parameter is incorrect! Please pass in 1 or 2!";
    }
    return device;
}
