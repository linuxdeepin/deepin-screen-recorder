// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DELAYTIME_H
#define DELAYTIME_H

#include <QThread>
/**
 * @brief The DelayTime class
 * 通过线程的方式实现的定时器、延时器
 */
class DelayTime  : public QThread
{
    Q_OBJECT

public:
    DelayTime(int msec = 1000);
    ~DelayTime();
    /**
     * @brief 停止
     */
    void stop();
    /**
     * @brief setForceToExitApp 设置是否需要强制退出应用程序
     * @param isForceToExitApp true:是 false:否
     */
    void setForceToExitApp(bool isForceToExitApp);
signals:
    /**
     * @brief progress 报告进度
     * @param value 进度值
     */
    void progress(const int value);

    /**
     * @brief doWork 定时结束后需要执行的任务
     */
    void doWork();
private:
    void run();    //虚函数
    /**
     * @brief forceToExitApp 强制退出应用程序
     */
    void forceToExitApp();
private:
    bool m_stopFlag;
    int m_delayTime;
    bool m_isForceToExitApp;
};

#endif // DELAYTIME_H
