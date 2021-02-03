/*
 * Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     Zheng Youge<youge.zheng@deepin.com>
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
    QStringList options;

    options << "-c";
    if(QSysInfo::currentCpuArchitecture().startsWith("arm")){
        qDebug() << "ARM";
        options << "pacmd list-sources | grep -PB 1 'USB.*stereo.*monitor>' | head -n 1 | perl -pe 's/.* //g'";
    }else if(QSysInfo::currentCpuArchitecture().startsWith("mips")){
        qDebug() << "MIPS";
        options << "pacmd list-sources | grep -v 'hdmi' | grep -PB 1 'output.*monitor>' | head -n 1 | perl -pe 's/.* //g'";
    }else {
        qDebug() << "OTHER" << QSysInfo::currentCpuArchitecture();
        options << "pacmd list-sources | grep -PB 1 'analog.*monitor>' | head -n 1 | perl -pe 's/.* //g'";
    }
    QProcess process;
    process.start("bash", options);
    process.waitForFinished();
    process.waitForReadyRead();
    QString str_output = process.readAllStandardOutput();
    process.close();
    if(str_output.isEmpty() && QSysInfo::currentCpuArchitecture().startsWith("arm")){
        options.clear();
        options << "-c" << "pacmd list-sources | grep -v 'hdmi' | grep -PB 1 'output.*monitor>' | head -n 1 | perl -pe 's/.* //g'";
        process.start("bash", options);
        process.waitForFinished();
        process.waitForReadyRead();
        str_output = process.readAllStandardOutput();
        process.close();
    }
    return str_output;
}
