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
#include <QDebug>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

AudioUtils::AudioUtils(QObject *parent)
{
}
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

bool AudioUtils::canVirtualCardOutput()
{
    QStringList options;
    options << "-c";
    options << "pacmd list-sinks | sed  -n '/.*index:.*\\([0-9]\\+\\).*/{n;p}' |  sed -n 's/name: <\\(.*\\)>/\\1/p' | grep 'alsa_output.platform-snd_aloop.0.analog-stereo' | wc -l";
    QProcess process;
    process.start("bash", options);
    process.waitForFinished();
    process.waitForReadyRead();
    QString str_output = process.readAllStandardOutput();
    process.close();
    return str_output.startsWith("1");
}

QString AudioUtils::currentAudioSink()
{
    QStringList options;
    options << "-c";
    options << "pacmd list-sinks | sed  -n '/\\*.*index:.*\\([0-9]\\+\\).*/{n;p}' | sed -n 's/name: <\\(.*\\)>/\\1/p' | sed -e 's/^[\\t]*//'";
    QProcess process;
    process.start("bash", options);
    process.waitForFinished();
    process.waitForReadyRead();
    QString str_output = process.readAllStandardOutput();
    process.close();
    return str_output;
}
QString AudioUtils::currentAudioSource()
{
    QStringList options;
    options << "-c";
    options << "pacmd list-sources | sed  -n '/\\*.*index:.*\\([0-9]\\+\\).*/{n;p}' | sed -n 's/name: <\\(.*\\)>/\\1/p' | sed -e 's/^[\\t]*//'";
    QProcess process;
    process.start("bash", options);
    process.waitForFinished();
    process.waitForReadyRead();
    QString str_output = process.readAllStandardOutput();
    process.close();
    return str_output;
}
