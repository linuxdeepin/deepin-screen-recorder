// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "voice_record_process.h"
#include <QAudioEncoderSettings>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDir>
#include <QTime>
#include <QUrl>
#include "utils.h"
#include <QDebug>
#include "log.h"


VoiceRecordProcess::VoiceRecordProcess(QObject *parent) : QThread(parent)
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess constructor.";
    audioRecorder = new QAudioRecorder(this);
    qCDebug(dsrApp) << "support codecs:" << audioRecorder->supportedAudioCodecs();
    qCDebug(dsrApp) << "support containers:" << audioRecorder->supportedContainers();

    QAudioEncoderSettings audioSettings;
    audioSettings.setQuality(QMultimedia::HighQuality);
    qCDebug(dsrApp) << "Audio settings quality set to HighQuality.";

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    audioRecorder->setAudioSettings(audioSettings);
    audioRecorder->setContainerFormat("audio/x-wav");
    qCDebug(dsrApp) << "Audio settings and container format (audio/x-wav) set for Qt 5.9+.";
#else
    audioSettings.setCodec("audio/PCM");
    audioRecorder->setAudioSettings(audioSettings);
    audioRecorder->setContainerFormat("wav");
    qCDebug(dsrApp) << "Audio settings and container format (wav) set for Qt < 5.9.";
#endif

    audioProbe = new QAudioProbe(this);
    qCDebug(dsrApp) << "QAudioProbe instance created.";
    if (audioProbe->setSource(audioRecorder)) {
        connect(audioProbe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(renderLevel(QAudioBuffer)));
        qCDebug(dsrApp) << "Audio probe source set and connected to renderLevel slot.";
    } else {
        qCWarning(dsrApp) << "Failed to set audio probe source.";
    }
    qCDebug(dsrApp) << "Exiting VoiceRecordProcess constructor.";
}

void VoiceRecordProcess::run()
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::run.";
    recordPath = generateRecordingFilepath();
    qCDebug(dsrApp) << "recordPath" + recordPath;
    audioRecorder->setOutputLocation(recordPath);
    QDateTime currentTime = QDateTime::currentDateTime();
    lastUpdateTime = currentTime;
    audioRecorder->record();
    qCDebug(dsrApp) << "Audio recording started.";
    qCDebug(dsrApp) << "Exiting VoiceRecordProcess::run.";
}

void VoiceRecordProcess::startRecord()
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::startRecord.";
    QThread::start();
    qCDebug(dsrApp) << "QThread started for audio recording.";
    qCDebug(dsrApp) << "Exiting VoiceRecordProcess::startRecord.";
}
void VoiceRecordProcess::stopRecord()
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::stopRecord.";
    audioRecorder->stop();
    qCDebug(dsrApp) << "Audio recording stopped.";
    //Todo: 记录时间
    qCDebug(dsrApp) << "Exiting VoiceRecordProcess::stopRecord.";
}
void VoiceRecordProcess::exitRecord()
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::exitRecord.";
    stopRecord();
    qCDebug(dsrApp) << "stopRecord called from exitRecord.";
    //记录文件处理
    qCDebug(dsrApp) << "Exiting VoiceRecordProcess::exitRecord.";
}
void VoiceRecordProcess::pauseRecord()
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::pauseRecord.";
    audioRecorder->pause();
    qCDebug(dsrApp) << "Audio recording paused.";
    qCDebug(dsrApp) << "Exiting VoiceRecordProcess::pauseRecord.";
}

void VoiceRecordProcess::resumeRecord()
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::resumeRecord.";
    QDateTime currentTime = QDateTime::currentDateTime();
    lastUpdateTime = currentTime;
    qCDebug(dsrApp) << "Last update time set to current time for resuming.";
    audioRecorder->record();
    qCDebug(dsrApp) << "Audio recording resumed.";
    qCDebug(dsrApp) << "Exiting VoiceRecordProcess::resumeRecord.";
}

void VoiceRecordProcess::renderLevel(const QAudioBuffer &buffer)
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::renderLevel.";
    Q_UNUSED(buffer);
    qCDebug(dsrApp) << "Exiting VoiceRecordProcess::renderLevel.";
}

QString VoiceRecordProcess::generateRecordingFilepath()
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::generateRecordingFilepath.";
    QString filePath = QDir(Utils::getRecordingSaveDirectory()).filePath(QString("%1 (%2).wav").arg(tr("New recording")).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss")));
    qCDebug(dsrApp) << "Generated recording filepath:" << filePath;
    return filePath;
}

QString VoiceRecordProcess::getRecordingFilepath()
{
    qCDebug(dsrApp) << "Entering VoiceRecordProcess::getRecordingFilepath.";
    qCDebug(dsrApp) << "Returning recording filepath:" << recordPath;
    return recordPath;
}


