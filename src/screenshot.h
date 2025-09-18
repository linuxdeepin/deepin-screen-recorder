// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include "main_window.h"

#include <QObject>


class Screenshot : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.deepin.ScreenRecorder")
    Q_PROPERTY(bool IsRecording READ isRecording)
public:
    explicit Screenshot(QObject *parent = nullptr);
    ~Screenshot();
public slots:
    void startScreenshot();
    void customScreenshot(bool hideToolbar, bool notify = true);
    void delayScreenshot(double num);
    void fullscreenScreenshot();
    void topWindowScreenshot();
    void noNotifyScreenshot();
    void OcrScreenshot();
    void ScrollScreenshot();
    void savePathScreenshot(const QString &path);
    void startScreenshotFor3rd(const QString &path);
    void initLaunchMode(const QString &launchmode);

    /**
     * @brief fullScreenRecord 快捷启动全屏录制,文件路径
     */
    void fullScreenRecord(QString fileName = "");

    Q_SCRIPTABLE void stopRecord();
    Q_SCRIPTABLE void stopApp();
    Q_SCRIPTABLE QString getRecorderNormalIcon();
    
signals:
    Q_SCRIPTABLE void RecorderState(const bool isStart); // true begin recorder; false stop recorder;
    Q_SCRIPTABLE void RecordingModeChanged(const bool isRecording); // 录屏模式变化
    void screenshotSaved(const QString &savePath); // 截图保存完成信号，返回保存路径

private:
    bool isRecording() const;

private:
    //void initUI();

//    EventContainer *m_eventContainer = nullptr;
    QString m_launchMode;
    MainWindow m_window;
    bool m_isCustomScreenshot = false; // 标记是否是自定义截图
    bool m_isRecording = false; // 实际录屏状态

};

#endif // SCREENSHOT_H
