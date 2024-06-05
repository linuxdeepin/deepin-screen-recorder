// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class ImagePlatform;
class VideoPlatform;
class RecorderWindow;
class QQmlEngine;

class Screenshot : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.ScreenRecorder")

public:
    static Screenshot *instance();
    void parser(const QStringList &arguments);

private:
    explicit Screenshot(QObject *parent = nullptr);
    ~Screenshot() noexcept override;

    QQmlEngine *getQmlEngine();
    void setExportImage(const QImage &image);

public slots:
    void startScreenshot();
    void delayScreenshot(double num);
    void fullscreenScreenshot();
    void topWindowScreenshot();
    void noNotifyScreenshot();
    void ocrScreenshot();
    void scrollScreenshot();
    void savePathScreenshot(const QString &path);
    void startScreenshotFor3rd(const QString &path);

    Q_SCRIPTABLE void stopRecord();
    Q_SCRIPTABLE QString getRecorderNormalIcon();

signals:
    Q_SCRIPTABLE void RecorderState(const bool isStart); // true begin recorder; false stop recorder;

private:
    static Screenshot *s_self;
    QString m_launchMode;
    
    std::unique_ptr<QQmlEngine> m_engine;
    std::unique_ptr<ImagePlatform> m_imagePlatform;
    std::unique_ptr<VideoPlatform> m_videoPlatform;

    std::vector<RecorderWindow*> m_recorderWindows;
};
