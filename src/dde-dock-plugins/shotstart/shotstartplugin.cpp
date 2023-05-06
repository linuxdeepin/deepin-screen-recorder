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

static bool g_iconFlag = false;
QIcon ShotStartPlugin::icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType)
{
    QString shotIconName = "screenshot";
    if(themeType == DGuiApplicationHelper::ColorType::DarkType){
        shotIconName = "screenshot_dark";
    }
    QIcon shot(QString(":/res/%1.svg").arg(shotIconName));
    QIcon recorder(":/res/screen-recording.svg");
    if (DockPart::QuickShow == dockPart/* || DockPart::DCCSetting == dockPart*/) {
        qInfo() << "是否正在录屏:" << m_isRecording;
        if (m_isRecording) {
            qInfo() << "显示录屏图标..." << m_iconWidget.isNull();
            if (m_iconWidget.isNull()) {
                qDebug() << "录屏图标已显示(icon)";
                return recorder;
            } else {
                QPixmap pixmap;
                if(m_isPauseRecord){
                    g_iconFlag = true;
                }
                if(g_iconFlag){
                    pixmap = m_iconWidget->iconPixMap(recorder, QSize(24, 24));
                }else{
                    pixmap = m_iconWidget->iconPixMap(recorder, QSize(24, 24));
                }
                g_iconFlag = !g_iconFlag;
                if (pixmap.isNull()) {
                    qDebug() << "录屏图标已显示(pixmap is null >> icon)";
                    return recorder;
                } else {
                    qDebug() << "录屏图标已显示(pixmap)";
                    return pixmap;
                }
            }
        } else {
            qInfo() << "显示截图图标...";
            if (m_iconWidget.isNull()) {
                qDebug() << "截图图标已显示(icon)";
                return shot;
            } else {
                QPixmap pixmap;
                pixmap = m_iconWidget->iconPixMap(shot, QSize(20, 20));
                if (pixmap.isNull()) {
                    qDebug() << "截图图标已显示(pixmap is null >> icon)";
                    return shot;
                } else {
                    qDebug() << "截图图标已显示(pixmap)";
                    return pixmap;
                }
            }
        }
    }else if(DockPart::QuickPanel == dockPart){
        return QIcon();
    }
    return shot;
}

PluginFlags ShotStartPlugin::flags() const
{
    if(m_isRecording){
        qInfo() << ">>>>>>>>>>>>>>>> flags() 正在录屏" ;
        return  Type_Common | Quick_Single | Attribute_CanSetting | Attribute_CanDrag | Attribute_CanInsert | Attribute_ForceDock;
    }else{
        qInfo() << ">>>>>>>>>>>>>>>> flags() 没有录屏" ;
        return  Type_Common | Quick_Single | Attribute_CanSetting | Attribute_CanDrag | Attribute_CanInsert;
    }
}

QWidget *ShotStartPlugin::itemWidget(const QString &itemKey)
{
    if(itemKey == QUICK_ITEM_KEY){
        return m_iconWidget.data();
    }
    return nullptr;
}

QWidget *ShotStartPlugin::itemTipsWidget(const QString &itemKey)
{
    if (itemKey != ShotShartPlugin) return nullptr;

    m_tipsWidget->setText(tr("Screenshot") + m_iconWidget->getSysShortcuts("screenshot"));
    return m_tipsWidget.data();
}

int ShotStartPlugin::itemSortKey(const QString &itemKey)
{
    qInfo() << "================= itemKey: " << itemKey;
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, 1).toInt();
}

void ShotStartPlugin::setSortKey(const QString &itemKey, const int order)
{
    qInfo() << "================= order: " << order << "itemKey: " << itemKey;
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
    m_iconWidget->start();
    return true;
}

void ShotStartPlugin::onStop()
{
    if(m_isRecording){
        m_isRecording = false;
    }
    qInfo() << "结束录屏。。。。。。" << m_isRecording;
    m_proxyInter->updateDockInfo(this, ::DockPart::QuickPanel);
    m_proxyInter->updateDockInfo(this, ::DockPart::QuickShow);
    m_iconWidget->stop();
}

void ShotStartPlugin::onRecording()
{
    m_proxyInter->updateDockInfo(this, ::DockPart::QuickPanel);
    m_proxyInter->updateDockInfo(this, ::DockPart::QuickShow);
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
                m_checkTimer->stop();
            }
        });
        m_checkTimer->start(2000);
    }
}

void ShotStartPlugin::onPause()
{
    m_isPauseRecord = true;
}

ShotStartPlugin::~ShotStartPlugin()
{
    if (nullptr != m_iconWidget)
        m_iconWidget->deleteLater();

    if (nullptr != m_tipsWidget)
        m_tipsWidget->deleteLater();
}
