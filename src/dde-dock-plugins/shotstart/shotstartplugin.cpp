// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shotstartplugin.h"
#include <DApplication>
#include <QDesktopWidget>
#include <QDBusInterface>

#define ShotShartPlugin "shot-start-plugin"
#define ShotShartApp "deepin-screen-recorder" // 使用截图录屏的翻译

ShotStartPlugin::ShotStartPlugin(QObject *parent)
    : QObject(parent), m_iconWidget(nullptr), m_tipsWidget(nullptr),m_quickPanelWidget(nullptr)

{
    m_isRecording =false;
    m_checkTimer = nullptr;
}

const QString ShotStartPlugin::pluginName() const
{
    return ShotShartPlugin;
}

const QString ShotStartPlugin::pluginDisplayName() const
{
    return tr("Screen Capture");
}

void ShotStartPlugin::init(PluginProxyInterface *proxyInter)
{
#ifndef UNIT_TEST
    qInfo() << "正在加载翻译...";
    // 加载翻译
    QString appName = qApp->applicationName();
    qDebug() << "1 >>qApp->applicationName(): " << qApp->applicationName();
    qApp->setApplicationName(ShotShartApp);
    qDebug() << "2 >>qApp->applicationName(): " << qApp->applicationName();
    bool isLoad = qApp->loadTranslator();
    qApp->setApplicationName(appName);
    qDebug() << "3 >>qApp->applicationName(): " << qApp->applicationName();
    qInfo() << "翻译加载" << (isLoad ? "成功" : "失败");
#endif

    m_proxyInter = proxyInter;


    if (m_iconWidget.isNull())
        m_iconWidget.reset(new IconWidget);
    if (m_quickPanelWidget.isNull()){
        m_quickPanelWidget.reset(new QuickPanelWidget);
        m_quickPanelWidget->changeType(QuickPanelWidget::SHOT);
    }
    if (m_tipsWidget.isNull())
        m_tipsWidget.reset(new TipsWidget);

    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ShotRecorder.PanelStatus")
            && sessionBus.registerObject("/com/deepin/ShotRecorder/PanelStatus", this, QDBusConnection::ExportScriptableSlots)) {
        qDebug() << "dbus service registration failed!";
    }

    connect(m_quickPanelWidget.data(),&QuickPanelWidget::clicked,this,&ShotStartPlugin::onClickQuickPanel);
}

bool ShotStartPlugin::pluginIsDisable()
{
    return m_proxyInter->getValue(this, "disabled", true).toBool();
}

void ShotStartPlugin::pluginStateSwitched()
{
    const bool disabledNew = !pluginIsDisable();
    m_proxyInter->saveValue(this, "disabled", disabledNew);
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

QWidget *ShotStartPlugin::itemWidget(const QString &itemKey)
{
//    if (itemKey != ShotShartPlugin) return nullptr;

    qInfo() << "Current itemWidget's itemKey: " << itemKey;
    if(itemKey == Dock::QUICK_ITEM_KEY){
        qInfo() << "Input Quick Panel Widget!";
        return m_quickPanelWidget.data();
    }else if (itemKey == ShotShartPlugin){
        qInfo() << "Input Common Plugin Widget!";
        return m_iconWidget.data();
    }
    return nullptr;
}

QWidget *ShotStartPlugin::itemTipsWidget(const QString &itemKey)
{
    qInfo() << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey != ShotShartPlugin) return nullptr;
    m_tipsWidget->setText(tr("Screenshot") + m_iconWidget->getSysShortcuts("screenshot"));
    return m_tipsWidget.data();
}

int ShotStartPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, 1).toInt();
}

void ShotStartPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
}

const QString ShotStartPlugin::itemCommand(const QString &itemKey)
{
//    if (itemKey != ShotShartPlugin) return QString();
    qInfo() << "Current itemWidget's itemKey: " << itemKey;
    if( itemKey == ShotShartPlugin){
        qInfo() << "(itemCommand) Input Common Plugin Widget!";
        if(m_isRecording){
            qInfo() << "Get DBus Interface";
            return "dbus-send --print-reply --dest=com.deepin.ScreenRecorder /com/deepin/ScreenRecorder com.deepin.ScreenRecorder.stopRecord";
        }
        return "dbus-send --print-reply --dest=com.deepin.Screenshot /com/deepin/Screenshot com.deepin.Screenshot.StartScreenshot";
    }else if(itemKey == Dock::QUICK_ITEM_KEY){
        qInfo() << "(itemCommand) Input Quick Panel Widget!";
    }
    return QString();
}

const QString ShotStartPlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin)
        return QString();

    return  m_iconWidget->itemContextMenu();
}

void ShotStartPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{

    Q_UNUSED(checked);
    if (itemKey != ShotShartPlugin) return;

    m_iconWidget->invokedMenuItem(menuId);
}

bool ShotStartPlugin::onStart()
{
    qInfo() << "Start The Clock!";
    m_isRecording = true;
    m_quickPanelWidget->start();
    qInfo() << "(onStart) Is Recording? " << m_isRecording;
    m_quickPanelWidget->changeType(QuickPanelWidget::RECORD);
    return true;
}

void ShotStartPlugin::onStop()
{
    m_isRecording = false;
    m_quickPanelWidget->stop();
    qInfo() << "(onStop) Is Recording? " << m_isRecording;
    m_quickPanelWidget->changeType(QuickPanelWidget::SHOT);
    qInfo() << "End The Clock!";
}

void ShotStartPlugin::onRecording()
{
    qDebug() << "(onRecording) Is Recording" << m_isRecording;
    m_nextCount++;
    if (1 == m_nextCount) {
        m_checkTimer = new QTimer();
        connect(m_checkTimer, &QTimer::timeout, this, [ = ] {
            //说明录屏还在进行中
            if (m_count < m_nextCount)
            {
                m_count = m_nextCount;
            }
            //说明录屏已经停止了
            else
            {
                onStop();
            }
        });
        m_checkTimer->start(2000);
    }
}

void ShotStartPlugin::onPause()
{
    qInfo() << "(onPause) Is Recording? " << m_isRecording;
   m_quickPanelWidget->pause();
   qInfo() << "Pause The Clock!";
}

void ShotStartPlugin::onClickQuickPanel()
{
    qInfo() <<"(onClickQuickPanel) 点击快捷面板";
    if(m_isRecording){
        qInfo() << "Get Record DBus Interface";
        QDBusInterface recordDBusInterface("com.deepin.ScreenRecorder",
                                           "/com/deepin/ScreenRecorder",
                                           "com.deepin.ScreenRecorder",
                                           QDBusConnection::sessionBus());
        recordDBusInterface.asyncCall("stopRecord");
        qInfo() << "Recorder plugin stop run!";
    }else{
        qInfo() << "Get Shot DBus Interface";
        QDBusInterface shotDBusInterface("com.deepin.Screenshot",
                                         "/com/deepin/Screenshot",
                                         "com.deepin.Screenshot",
                                         QDBusConnection::sessionBus());
        shotDBusInterface.asyncCall("StartScreenshot");
        qInfo() << "Shot and Recorder plugin start run!";
    }
}

ShotStartPlugin::~ShotStartPlugin()
{
    if (nullptr != m_iconWidget)
        m_iconWidget->deleteLater();

    if (nullptr != m_tipsWidget)
        m_tipsWidget->deleteLater();

    if (nullptr != m_quickPanelWidget)
        m_quickPanelWidget->deleteLater();

}
