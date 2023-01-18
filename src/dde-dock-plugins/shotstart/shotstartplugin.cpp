// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shotstartplugin.h"
#include <DApplication>
#include <QDesktopWidget>


#define ShotShartPlugin "shot-start-plugin"
#define ShotShartApp "deepin-screen-recorder" // 使用截图录屏的翻译

ShotStartPlugin::ShotStartPlugin(QObject *parent)
    : QObject(parent), m_iconWidget(nullptr), m_tipsWidget(nullptr), m_isRecording(false)

{
}

const QString ShotStartPlugin::pluginName() const
{
    return ShotShartPlugin;
}

const QString ShotStartPlugin::pluginDisplayName() const
{
    if (m_isRecording) {
        QTime showTime(0, 0, 0);
        int time = m_baseTime.secsTo(QTime::currentTime());
        showTime = showTime.addSecs(time);
        return showTime.toString("hh:mm:ss");
    }
    return tr("Screen Capture");
}

void ShotStartPlugin::init(PluginProxyInterface *proxyInter)
{
#ifndef UNIT_TEST
    // 加载翻译
    QString appName = qApp->applicationName();
    qApp->setApplicationName(ShotShartApp);
    qApp->loadTranslator();
    qApp->setApplicationName(appName);
#endif

    m_proxyInter = proxyInter;


    if (m_iconWidget.isNull())
        m_iconWidget.reset(new IconWidget);

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
}

QIcon ShotStartPlugin::icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType)
{
    static QIcon icon(":/res/screen-capture-dark.svg");
    static QIcon icon_dark(":/res/screen-capture-dark.svg");
    static QIcon shot(":/res/icon-shot.svg");
    static QIcon recorder(":/res/icon-recorder.svg");
    if (DockPart::QuickShow == dockPart) {
        if (themeType ==  DGuiApplicationHelper::ColorType::LightType) {
            //return m_iconWidget->iconPixMap("screen-capture-dark", QSize(24, 24));
            return icon_dark;
        } else {
            //return m_iconWidget->iconPixMap("screen-capture", QSize(24, 24));
            return icon;
        }
    } else if (DockPart::QuickPanel == dockPart) {
        qInfo() << "是否正在录屏:" << m_isRecording;
        if (m_isRecording) {
            qInfo() << "显示录屏图标..." << m_iconWidget.isNull();
            if(m_iconWidget.isNull()){
                qDebug() << "录屏图标已显示(icon)";
                return recorder;
            }else{
                qDebug() << "录屏图标已显示(pixmap)";
                return m_iconWidget->iconPixMap(recorder, QSize(24, 24));
            }
        } else {
            qInfo() << "显示截图图标...";
            if(m_iconWidget.isNull()){
                qDebug() << "截图图标已显示(icon)";
                return shot;
            }else{
                qDebug() << "截图图标已显示(pixmap)";
                return m_iconWidget->iconPixMap(shot, QSize(24, 24));
            }
        }
    }
    return icon;
}

PluginFlags ShotStartPlugin::flags() const
{
    return  Type_Common | Quick_Single | Attribute_ForceDock | Attribute_CanSetting;
}

QWidget *ShotStartPlugin::itemWidget(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin) return nullptr;

    return m_iconWidget.data();
}

QWidget *ShotStartPlugin::itemTipsWidget(const QString &itemKey)
{
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
    if (itemKey != ShotShartPlugin) return QString();

    if (m_isRecording) {
        //停止录屏
        QDBusInterface shotDBusInterface("com.deepin.ScreenRecorder",
                                         "/com/deepin/ScreenRecorder",
                                         "com.deepin.ScreenRecorder",
                                         QDBusConnection::sessionBus());

        shotDBusInterface.asyncCall("stopRecord");
        return "";
    }

    return "dbus-send --print-reply --dest=com.deepin.Screenshot /com/deepin/Screenshot com.deepin.Screenshot.StartScreenshot";
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
    m_isRecording = true;
    qInfo() << "开始录屏。。。。。。" << m_isRecording;
    m_baseTime = QTime::currentTime();
    m_proxyInter->updateDockInfo(this, ::DockPart::QuickPanel);
}

void ShotStartPlugin::onStop()
{
    m_isRecording = false;
    qInfo() << "结束录屏。。。。。。" << m_isRecording;
    m_proxyInter->updateDockInfo(this, ::DockPart::QuickPanel);
}

void ShotStartPlugin::onRecording()
{
    m_proxyInter->updateDockInfo(this, ::DockPart::QuickPanel);
}

ShotStartPlugin::~ShotStartPlugin()
{
    if (nullptr != m_iconWidget)
        m_iconWidget->deleteLater();

    if (nullptr != m_tipsWidget)
        m_tipsWidget->deleteLater();
}
