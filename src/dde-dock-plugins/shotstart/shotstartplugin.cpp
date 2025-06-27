// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shotstartplugin.h"
#include <DApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QDBusInterface>
#include "../../utils/log.h"

#define ShotShartPlugin "shot-start-plugin"
#define ShotShartApp "deepin-screen-recorder"  // 使用截图录屏的翻译

#ifndef QUICK_ITEM_KEY
const QString QUICK_ITEM_KEY = QStringLiteral("quick_item_key");
#endif
const int DETECT_SERV_INTERVAL = 2000;  // 检测服务存在的定时器间隔

Q_LOGGING_CATEGORY(dsrApp, "shot-start-plugin");

ShotStartPlugin::ShotStartPlugin(QObject *parent)
    : QObject(parent)
    , m_iconWidget(nullptr)
    , m_quickPanelWidget(nullptr)
    , m_tipsWidget(nullptr)

{
    qCDebug(dsrApp) << "ShotStartPlugin constructor called.";
    qCDebug(dsrApp) << "Initializing ShotStartPlugin";
    m_isRecording = false;
    m_checkTimer = nullptr;
    m_bDockQuickPanel = false;
    qCDebug(dsrApp) << "ShotStartPlugin constructor finished. m_isRecording:" << m_isRecording << ", m_bDockQuickPanel:" << m_bDockQuickPanel;
}

const QString ShotStartPlugin::pluginName() const
{
    qCDebug(dsrApp) << "pluginName method called, returning:" << ShotShartPlugin;
    return ShotShartPlugin;
}

const QString ShotStartPlugin::pluginDisplayName() const
{
    qCDebug(dsrApp) << "pluginDisplayName method called, returning translated Screenshot.";
    return tr("Screenshot");
}

void ShotStartPlugin::init(PluginProxyInterface *proxyInter)
{
    qCDebug(dsrApp) << "init method called.";
    qCInfo(dsrApp) << "Initializing shot start plugin";
#ifndef UNIT_TEST

#ifdef DOCK_API_VERSION
#if (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))
    m_bDockQuickPanel = true;
    qCDebug(dsrApp) << "Dock API version is >= 2.0.0, m_bDockQuickPanel set to true.";
#else
    qCDebug(dsrApp) << qPrintable("dock version less than 2.0.0");
#endif  // (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))

#else
    // runtime version check
    bool ret;
    int version = qApp->property("dock_api_version").toInt(&ret);
    qCInfo(dsrApp) << "runtime dock version" << version << ret;
    if (ret && version >= ((2 << 16) | (0 << 8) | (0))) {
        m_bDockQuickPanel = true;
        qCDebug(dsrApp) << "Runtime dock version check passed, m_bDockQuickPanel set to true.";
    } else {
        qCDebug(dsrApp) << "Runtime dock version check failed or version too low.";
    }
#endif  // DOCK_API_VERSION

    if (m_bDockQuickPanel) {
        qCInfo(dsrApp) << "The current dock version support quick panels";
    }

    qCInfo(dsrApp) << "load translation ...";
    // 加载翻译
    QString appName = qApp->applicationName();
    qCDebug(dsrApp) << "1 >>qApp->applicationName(): " << qApp->applicationName();
    qApp->setApplicationName(ShotShartApp);
    qCDebug(dsrApp) << "2 >>qApp->applicationName(): " << qApp->applicationName();
    bool isLoad = qApp->loadTranslator();
    qApp->setApplicationName(appName);
    qCDebug(dsrApp) << "3 >>qApp->applicationName(): " << qApp->applicationName();
    qCInfo(dsrApp) << "translation load" << (isLoad ? "success" : "failed");
    qCDebug(dsrApp) << "Translation loaded:" << isLoad;

#endif  // UNIT_TEST

    m_proxyInter = proxyInter;

    if (m_iconWidget.isNull()) {
        qCDebug(dsrApp) << "Creating new icon widget";
        m_iconWidget.reset(new IconWidget);
    }

    if (m_quickPanelWidget.isNull()) {
        qCDebug(dsrApp) << "Creating new quick panel widget";
        m_quickPanelWidget.reset(new QuickPanelWidget);
        // "截图"快捷面板不再响应录制中动画效果，固定为截图图标
        m_quickPanelWidget->changeType(QuickPanelWidget::SHOT);
    }
    if (m_tipsWidget.isNull()) {
        qCDebug(dsrApp) << "Creating new tips widget";
        m_tipsWidget.reset(new TipsWidget);
    }

    if (m_bDockQuickPanel || !pluginIsDisable()) {
        qCInfo(dsrApp) << "Adding plugin to dock";
        m_proxyInter->itemAdded(this, pluginName());
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ShotRecorder.PanelStatus") &&
        sessionBus.registerObject("/com/deepin/ShotRecorder/PanelStatus", this, QDBusConnection::ExportScriptableSlots)) {
        qCInfo(dsrApp) << "dbus service registration success!";
        qCDebug(dsrApp) << "DBus service registration successful.";
    } else {
        qCWarning(dsrApp) << "dbus service registration failed!";
    }

    connect(m_quickPanelWidget.data(), &QuickPanelWidget::clicked, this, &ShotStartPlugin::onClickQuickPanel);
    qCDebug(dsrApp) << "Connected quickPanelWidget clicked signal to onClickQuickPanel slot.";
    qCDebug(dsrApp) << "init method finished.";
}

bool ShotStartPlugin::pluginIsDisable()
{
    qCDebug(dsrApp) << "pluginIsDisable method called.";
    if (m_bDockQuickPanel) {
        qCWarning(dsrApp) << "The current dock version does not support quick panels!!";
        return false;
    }
    bool isDisabled = m_proxyInter->getValue(this, "disabled", true).toBool();
    qCDebug(dsrApp) << "Plugin disabled state:" << isDisabled << ", returning it.";
    return isDisabled;
}

void ShotStartPlugin::pluginStateSwitched()
{
    qCDebug(dsrApp) << "pluginStateSwitched method called.";
    const bool disabledNew = !pluginIsDisable();
    qCInfo(dsrApp) << "Plugin state switched, new disabled state:" << disabledNew;
    m_proxyInter->saveValue(this, "disabled", disabledNew);
    if (disabledNew) {
        qCDebug(dsrApp) << "Removing plugin from dock";
        m_proxyInter->itemRemoved(this, pluginName());
        qCDebug(dsrApp) << "Plugin removed from dock.";
    } else {
        qCDebug(dsrApp) << "Adding plugin to dock";
        m_proxyInter->itemAdded(this, pluginName());
        qCDebug(dsrApp) << "Plugin added to dock.";
    }
    qCDebug(dsrApp) << "pluginStateSwitched method finished.";
}

#if defined(DOCK_API_VERSION) && (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))

/**
 * @return The Tray plugin supports the quick panel type
 */
Dock::PluginFlags ShotStartPlugin::flags() const
{
    qCDebug(dsrApp) << "flags method called, returning quick panel flags.";
    return Dock::Type_Quick | Dock::Quick_Panel_Single | Dock::Attribute_Normal;
}

#endif

QWidget *ShotStartPlugin::itemWidget(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemWidget method called with itemKey:" << itemKey;
    if (itemKey == QUICK_ITEM_KEY) {
        qCDebug(dsrApp) << "Item key is QUICK_ITEM_KEY, returning quickPanelWidget.";
        return m_quickPanelWidget.data();
    } else if (itemKey == ShotShartPlugin) {
        qCDebug(dsrApp) << "Item key is ShotShartPlugin, returning iconWidget.";
        return m_iconWidget.data();
    }
    qCDebug(dsrApp) << "Item key not recognized, returning nullptr.";
    return nullptr;
}

QWidget *ShotStartPlugin::itemTipsWidget(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemTipsWidget method called with itemKey:" << itemKey;
    qCDebug(dsrApp) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey != ShotShartPlugin) {
        qCDebug(dsrApp) << "Item key is not ShotShartPlugin, returning nullptr.";
        return nullptr;
    }
    m_tipsWidget->setText(tr("Screenshot") + m_iconWidget->getSysShortcuts("screenshot"));
    qCDebug(dsrApp) << "Tips widget text set, returning tipsWidget.";
    return m_tipsWidget.data();
}

int ShotStartPlugin::itemSortKey(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemSortKey method called with itemKey:" << itemKey;
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    int sortKey = m_proxyInter->getValue(this, key, 1).toInt();
    qCDebug(dsrApp) << "Sort key for" << itemKey << ":" << sortKey << ", returning it.";
    return sortKey;
}

void ShotStartPlugin::setSortKey(const QString &itemKey, const int order)
{
    qCDebug(dsrApp) << "setSortKey method called with itemKey:" << itemKey << "order:" << order;
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
    qCDebug(dsrApp) << "Sort key for" << itemKey << "set to:" << order;
    qCDebug(dsrApp) << "setSortKey method finished.";
}

const QString ShotStartPlugin::itemCommand(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemCommand method called with itemKey:" << itemKey;
    qCDebug(dsrApp) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey == ShotShartPlugin) {
        qCDebug(dsrApp) << "(itemCommand) Input Common Plugin Widget!";
        // 录屏过程不响应点击
        if (!m_isRecording) {
            qCDebug(dsrApp) << "Get DBus Interface";
            qCDebug(dsrApp) << "Not recording, returning StartScreenshot DBus command.";
            return "dbus-send --print-reply --dest=com.deepin.Screenshot /com/deepin/Screenshot "
                   "com.deepin.Screenshot.StartScreenshot";
        } else {
            qCDebug(dsrApp) << "Recording in progress, ignoring screenshot command";
            qCDebug(dsrApp) << "Recording in progress, returning empty string.";
        }
    } else {
        qCWarning(dsrApp) << "(itemCommand) Input unknow widget!";
        qCDebug(dsrApp) << "Unknown widget, returning empty string.";
    }
    qCDebug(dsrApp) << "itemCommand method finished.";
    return QString("");
}

const QString ShotStartPlugin::itemContextMenu(const QString &)
{
    qCDebug(dsrApp) << "itemContextMenu method called, returning empty string as context menu is not provided.";
    // 拆分截图和录屏托盘图标，不再提供右键菜单
    return QString();
}

void ShotStartPlugin::invokedMenuItem(const QString &, const QString &, const bool)
{
    qCDebug(dsrApp) << "invokedMenuItem method called, no action as context menu is not provided.";
    // 拆分截图和录屏托盘图标，不再提供右键菜单
    return;
}

bool ShotStartPlugin::onStart()
{
    qCDebug(dsrApp) << "onStart method called.";
    qCDebug(dsrApp) << "Disable screenshot tray icon";
    m_isRecording = true;
    m_iconWidget->setEnabled(false);
    m_iconWidget->update();
    qCDebug(dsrApp) << "Icon widget disabled and updated.";

    m_quickPanelWidget->setEnabled(false);
    qCDebug(dsrApp) << "Quick panel widget disabled.";
    qCDebug(dsrApp) << "(onStart) Is Recording? " << m_isRecording;
    qCDebug(dsrApp) << "onStart method finished, returning true.";
    return true;
}

void ShotStartPlugin::onStop()
{
    qCDebug(dsrApp) << "onStop method called.";
    qCDebug(dsrApp) << "(onStop) Is Recording? " << m_isRecording;
    m_isRecording = false;

    if (!m_iconWidget.isNull()) {
        m_iconWidget->setEnabled(true);
        m_iconWidget->update();
        qCDebug(dsrApp) << "Icon widget not null, enabled and updated.";
    } else {
        qCDebug(dsrApp) << "Icon widget is null, cannot enable or update.";
    }

    if (!m_quickPanelWidget.isNull()) {
        m_quickPanelWidget->setEnabled(true);
        qCDebug(dsrApp) << "Quick panel widget not null, enabled.";
    } else {
        qCDebug(dsrApp) << "Quick panel widget is null, cannot enable.";
    }

    qCDebug(dsrApp) << "Enable screenshot tray icon";
    qCDebug(dsrApp) << "onStop method finished.";
}

void ShotStartPlugin::onRecording()
{
    qCDebug(dsrApp) << "onRecording method called.";
    qCDebug(dsrApp) << "(onRecording) Is Recording" << m_isRecording;
    m_nextCount++;
    if (1 == m_nextCount) {
        qCDebug(dsrApp) << "m_nextCount is 1.";
        if (!m_checkTimer) {
            qCDebug(dsrApp) << "Creating new check timer";
            m_checkTimer = new QTimer(this);
        } else {
            qCDebug(dsrApp) << "Check timer already exists.";
        }
        connect(m_checkTimer, &QTimer::timeout, this, [=] {
            // 说明录屏还在进行中
            if (m_count < m_nextCount) {
                qCDebug(dsrApp) << "Recording in progress, updating count";
                m_count = m_nextCount;
            }
            // 说明录屏已经停止了
            else {
                qCWarning(dsrApp) << qPrintable("Unsafe stop recoding!");
                onStop();
                m_checkTimer->stop();
                qCDebug(dsrApp) << "Recording stopped, calling onStop and stopping check timer.";
            }
        });
        qCDebug(dsrApp) << "Starting check timer with interval:" << DETECT_SERV_INTERVAL;
        m_checkTimer->start(DETECT_SERV_INTERVAL);
    } else {
        qCDebug(dsrApp) << "m_nextCount is not 1.";
    }

    if (m_checkTimer && !m_checkTimer->isActive()) {
        qCDebug(dsrApp) << "Restarting inactive check timer";
        m_checkTimer->start(DETECT_SERV_INTERVAL);
    } else {
        qCDebug(dsrApp) << "Check timer is null or already active.";
    }
    qCDebug(dsrApp) << "onRecording method finished.";
}

void ShotStartPlugin::onPause()
{
    qCDebug(dsrApp) << "onPause method called. (DoNothing)";
    // DoNothing
    qCDebug(dsrApp) << "onPause method finished.";
}

void ShotStartPlugin::onClickQuickPanel()
{
    qCDebug(dsrApp) << "onClickQuickPanel method called.";
    // 录制中不再响应快捷面板
    qCDebug(dsrApp) << "(onClickQuickPanel) 点击快捷面板";
    if (!m_isRecording) {
        qCDebug(dsrApp) << "Get Shot DBus Interface";
        m_proxyInter->requestSetAppletVisible(this, pluginName(), false);
        QDBusInterface shotDBusInterface(
            "com.deepin.Screenshot", "/com/deepin/Screenshot", "com.deepin.Screenshot", QDBusConnection::sessionBus());
        shotDBusInterface.asyncCall("StartScreenshot");
        qCDebug(dsrApp) << "Shot and Recorder plugin start run!";
        qCDebug(dsrApp) << "Not recording, initiating StartScreenshot DBus call.";
    } else {
        qCDebug(dsrApp) << "Recording in progress, ignoring quick panel click.";
    }
    qCDebug(dsrApp) << "onClickQuickPanel method finished.";
}

ShotStartPlugin::~ShotStartPlugin()
{
    qCDebug(dsrApp) << "ShotStartPlugin destructor called.";
    if (nullptr != m_iconWidget) {
        m_iconWidget->deleteLater();
        qCDebug(dsrApp) << "m_iconWidget deleted.";
    } else {
        qCDebug(dsrApp) << "m_iconWidget is null, no deletion needed.";
    }

     if (nullptr != m_tipsWidget) {
        m_tipsWidget->deleteLater();
        qCDebug(dsrApp) << "m_tipsWidget deleted.";
     } else {
        qCDebug(dsrApp) << "m_tipsWidget is null, no deletion needed.";
    }

     if (nullptr != m_quickPanelWidget) {
        m_quickPanelWidget->deleteLater();
        qCDebug(dsrApp) << "m_quickPanelWidget deleted.";
    } else {
        qCDebug(dsrApp) << "m_quickPanelWidget is null, no deletion needed.";
    }
    qCDebug(dsrApp) << "ShotStartPlugin destructor finished.";
}

