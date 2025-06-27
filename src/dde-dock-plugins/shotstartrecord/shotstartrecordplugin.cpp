// SPDX-FileCopyrightText: 2021-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shotstartrecordplugin.h"

#include <DApplication>
#include <QDBusInterface>
#include "../../utils/log.h"
#include "../../dbus_name.h"

#define RecordShartPlugin "shot-start-record-plugin"
#define RecordShartApp "deepin-screen-recorder"  // 使用截图录屏的翻译

#ifndef QUICK_ITEM_KEY
const QString QUICK_ITEM_KEY = QStringLiteral("quick_item_key");
#endif
const int DETECT_SERV_INTERVAL = 2000;  // 检测服务存在的定时器间隔

Q_LOGGING_CATEGORY(dsrApp, "shot-start-record-plugin");

ShotStartRecordPlugin::ShotStartRecordPlugin(QObject *parent)
    : QObject(parent)
    , m_iconWidget(nullptr)
    , m_quickPanelWidget(nullptr)
    , m_tipsWidget(nullptr)

{
    qCDebug(dsrApp) << "ShotStartRecordPlugin constructor called.";
    m_isRecording = false;
    m_checkTimer = nullptr;
    m_bDockQuickPanel = false;
    qCDebug(dsrApp) << "Initial state: m_isRecording =" << m_isRecording << ", m_bDockQuickPanel =" << m_bDockQuickPanel;

    qRegisterMetaType<DockItemInfo>("DockItemInfo");
    qRegisterMetaType<DockItemInfos>("DockItemInfos");
    qCDebug(dsrApp) << "Registered meta types DockItemInfo and DockItemInfos.";
    qCDebug(dsrApp) << "ShotStartRecordPlugin constructor finished.";
}

/**
 * @return pluginName:返回插件名称
 */
const QString ShotStartRecordPlugin::pluginName() const
{
    qCDebug(dsrApp) << "pluginName method called, returning:" << RecordShartPlugin;
    return RecordShartPlugin;
}

/**
 * @return pluginDisplayName:返回插件名称，用于界面显示
 */
const QString ShotStartRecordPlugin::pluginDisplayName() const
{
    qCDebug(dsrApp) << "pluginDisplayName method called, returning translated Record.";
    return tr("Record");
}

/**
 * @brief init:插件初始化入口函数
 * @param proxyInter:主程序进程，上下文指针
 */
void ShotStartRecordPlugin::init(PluginProxyInterface *proxyInter)
{
    qCDebug(dsrApp) << "init method called.";
    qCInfo(dsrApp) << "Initializing plugin with proxy interface";
#ifndef UNIT_TEST
#ifdef DOCK_API_VERSION
#if (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))
    m_bDockQuickPanel = true;
    qCDebug(dsrApp) << "Dock API version is >= 2.0.0, m_bDockQuickPanel set to true.";
#else
    qCDebug(dsrApp) << qPrintable("dock version less than 2.0.0");
    qCDebug(dsrApp) << "Dock API version is less than 2.0.0.";
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
        qCDebug(dsrApp) << "Dock quick panel supported.";
    } else {
        qCDebug(dsrApp) << "Dock quick panel not supported.";
    }

    qCInfo(dsrApp) << "load translation...";
    // 加载翻译
    QString appName = qApp->applicationName();
    qCDebug(dsrApp) << "1 >>qApp->applicationName(): " << qApp->applicationName();
    qApp->setApplicationName(RecordShartApp);
    qCDebug(dsrApp) << "2 >>qApp->applicationName(): " << qApp->applicationName();
    bool isLoad = qApp->loadTranslator();
    qApp->setApplicationName(appName);
    qCDebug(dsrApp) << "3 >>qApp->applicationName(): " << qApp->applicationName();
    qCInfo(dsrApp) << "translation load" << (isLoad ? "success" : "failed");
    qCDebug(dsrApp) << "Translation loaded:" << isLoad;

#endif  // UNIT_TEST

    m_proxyInter = proxyInter;
    qCDebug(dsrApp) << "Proxy interface set.";

    if (m_iconWidget.isNull()) {
        m_iconWidget.reset(new RecordIconWidget);
        qCDebug(dsrApp) << "New icon widget created.";
    } else {
        qCDebug(dsrApp) << "Icon widget already exists.";
    }

    if (m_quickPanelWidget.isNull()) {
        m_quickPanelWidget.reset(new QuickPanelWidget);
        m_quickPanelWidget->changeType(QuickPanelWidget::RECORD);
        qCDebug(dsrApp) << "New quick panel widget created and type set to RECORD.";
    } else {
        qCDebug(dsrApp) << "Quick panel widget already exists.";
    }
    if (m_tipsWidget.isNull()) {
        m_tipsWidget.reset(new TipsWidget);
        qCDebug(dsrApp) << "New tips widget created.";
    } else {
        qCDebug(dsrApp) << "Tips widget already exists.";
    }

    if (m_bDockQuickPanel || !pluginIsDisable()) {
        qCInfo(dsrApp) << "the current plugin has been added to the dock";
        m_proxyInter->itemAdded(this, pluginName());
        qCDebug(dsrApp) << "Plugin added to dock.";
    } else {
        qCDebug(dsrApp) << "Skipping adding plugin to dock due to conditions.";
    }

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.ShotRecorder.Recorder.PanelStatus") &&
        sessionBus.registerObject(
            "/com/deepin/ShotRecorder/Recorder/PanelStatus", this, QDBusConnection::ExportScriptableSlots)) {
        qCInfo(dsrApp) << "dbus service registration success!";
        qCDebug(dsrApp) << "DBus service registration successful.";
    } else {
        qCWarning(dsrApp) << "dbus service registration failed!";
        qCDebug(dsrApp) << "DBus service registration failed.";
    }

    connect(m_quickPanelWidget.data(), &QuickPanelWidget::clicked, this, &ShotStartRecordPlugin::onClickQuickPanel);
    qCDebug(dsrApp) << "Connected quickPanelWidget clicked signal to onClickQuickPanel slot.";
    qCDebug(dsrApp) << "init method finished.";
}

/**
   @return 返回当前插件是否被禁用
 */
bool ShotStartRecordPlugin::pluginIsDisable()
{
    qCDebug(dsrApp) << "pluginIsDisable method called.";
    if (m_bDockQuickPanel) {
        qCWarning(dsrApp) << "The current dock version does not support quick panels!!";
        qCDebug(dsrApp) << "Dock quick panel not supported, returning false.";
        return false;
    }
    bool isDisabled = m_proxyInter->getValue(this, "disabled", true).toBool();
    qCDebug(dsrApp) << "Plugin disabled state:" << isDisabled << ", returning it.";
    return isDisabled;
}

/**
   @brief 切换插件禁用状态
 */
void ShotStartRecordPlugin::pluginStateSwitched()
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
Dock::PluginFlags ShotStartRecordPlugin::flags() const
{
    qCDebug(dsrApp) << "flags method called, returning quick panel flags.";
    return Dock::Type_Quick | Dock::Quick_Panel_Single | Dock::Attribute_Normal;
}

#endif

/**
 * @brief itemWidget:返回插件主控件，用于dde-dock面板上显示
 * @param itemKey:控件名称
 */
QWidget *ShotStartRecordPlugin::itemWidget(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemWidget method called with itemKey:" << itemKey;
    if (itemKey == QUICK_ITEM_KEY) {
        qCDebug(dsrApp) << "Item key is QUICK_ITEM_KEY, returning quickPanelWidget.";
        return m_quickPanelWidget.data();
    } else if (itemKey == RecordShartPlugin) {
        qCDebug(dsrApp) << "Item key is RecordShartPlugin, returning iconWidget.";
        return m_iconWidget.data();
    }
    qCDebug(dsrApp) << "Item key not recognized, returning nullptr.";
    return nullptr;
}

/**
   @return 返回鼠标Hover时的Tips信息，目前为快捷键信息
 */
QWidget *ShotStartRecordPlugin::itemTipsWidget(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemTipsWidget method called with itemKey:" << itemKey;
    qCDebug(dsrApp) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey != RecordShartPlugin) {
        qCDebug(dsrApp) << "Item key is not RecordShartPlugin, returning nullptr.";
        return nullptr;
    }
    m_tipsWidget->setText(tr("Record") + m_iconWidget->getSysShortcuts("deepin-screen-recorder"));
    qCDebug(dsrApp) << "Tips widget text set, returning tipsWidget.";
    return m_tipsWidget.data();
}

/**
   @return 返回当前托盘图标的顺序
 */
int ShotStartRecordPlugin::itemSortKey(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemSortKey method called with itemKey:" << itemKey;
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    int sortKey = m_proxyInter->getValue(this, key, 1).toInt();
    qCDebug(dsrApp) << "Sort key for" << itemKey << ":" << sortKey << ", returning it.";
    return sortKey;
}

/**
   @brief 记录当前托盘图标的顺序
 */
void ShotStartRecordPlugin::setSortKey(const QString &itemKey, const int order)
{
    qCDebug(dsrApp) << "setSortKey method called with itemKey:" << itemKey << "order:" << order;
    qCDebug(dsrApp) << "Setting sort key for item:" << itemKey << "to order:" << order;
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
    qCDebug(dsrApp) << "Sort key saved.";
    qCDebug(dsrApp) << "setSortKey method finished.";
}

/**
   @brief 鼠标单击托盘图标执行指令，唤起截图录屏
 */
const QString ShotStartRecordPlugin::itemCommand(const QString &itemKey)
{
    qCDebug(dsrApp) << "itemCommand method called with itemKey:" << itemKey;
    qCDebug(dsrApp) << "Current itemWidget's itemKey: " << itemKey;
    if (itemKey == RecordShartPlugin) {
        qCDebug(dsrApp) << "(itemCommand) Input Common Plugin Widget!";
        qCDebug(dsrApp) << "Returning stopRecord DBus command.";
        return "dbus-send --print-reply --dest=com.deepin.ScreenRecorder /com/deepin/ScreenRecorder "
               "com.deepin.ScreenRecorder.stopRecord";
    } else {
        qCWarning(dsrApp) << "(itemCommand) Input unknow widget!";
        qCDebug(dsrApp) << "Unknown widget, returning empty string.";
    }
    qCDebug(dsrApp) << "itemCommand method finished.";
    return QString();
}

/**
   @brief 请求右键菜单内容，目前不再提供右键菜单
 */
const QString ShotStartRecordPlugin::itemContextMenu(const QString &)
{
    qCDebug(dsrApp) << "itemContextMenu method called.";
    qCDebug(dsrApp) << "Context menu requested but not provided";
    // 不再提供右键菜单
    qCDebug(dsrApp) << "Returning empty string as context menu is not provided.";
    return QString();
}

/**
   @brief 右键菜单触发，目前不再提供右键菜单
 */
void ShotStartRecordPlugin::invokedMenuItem(const QString &, const QString &, const bool)
{
    qCDebug(dsrApp) << "invokedMenuItem method called.";
    qCDebug(dsrApp) << "Menu item invoked but not handled";
    // 不再提供右键菜单
    qCDebug(dsrApp) << "No action taken as context menu is not provided.";
    return;
}

/**
   @brief 录屏开始，隐藏任务栏托盘图标，快捷面板切换录制中
        任务栏"录制中"图标由recordtime插件提供
   @return 是否启动成功
 */
bool ShotStartRecordPlugin::onStart()
{
    qCDebug(dsrApp) << "onStart method called.";
    qCInfo(dsrApp) << "Starting screen recording";
    m_bPreviousIsVisable = getTrayIconVisible();
    if (m_bPreviousIsVisable) {
        qCDebug(dsrApp) << "Hiding tray icon";
        // 仅隐藏任务栏图标
        setTrayIconVisible(false);
        qCDebug(dsrApp) << "Tray icon set to invisible.";
    } else {
        qCDebug(dsrApp) << "Tray icon was not visible, no change needed.";
    }

    qCDebug(dsrApp) << "Start The Clock!";
    m_isRecording = true;
    m_quickPanelWidget->start();
    qCDebug(dsrApp) << "(onStart) Is Recording? " << m_isRecording;
    m_quickPanelWidget->changeType(QuickPanelWidget::RECORDING);
    qCDebug(dsrApp) << "Quick panel widget type set to RECORDING.";
    qCDebug(dsrApp) << "onStart method finished, returning true.";
    return true;
}

/**
   @brief 录屏结束，复位托盘图标和快捷面板图标
 */
void ShotStartRecordPlugin::onStop()
{
    qCDebug(dsrApp) << "onStop method called.";
    qCInfo(dsrApp) << "Stopping screen recording";
    if (m_bPreviousIsVisable) {
        qCDebug(dsrApp) << "Restoring tray icon visibility";
        // 恢复显示任务栏图标
        setTrayIconVisible(true);
        qCDebug(dsrApp) << "Tray icon set to visible.";
    } else {
        qCDebug(dsrApp) << "Tray icon was not previously visible, no change needed.";
    }

    m_isRecording = false;
    m_quickPanelWidget->stop();
    qCDebug(dsrApp) << "(onStop) Is Recording? " << m_isRecording;
    m_quickPanelWidget->changeType(QuickPanelWidget::RECORD);
    qCDebug(dsrApp) << "Quick panel widget type set to RECORD.";
    qCDebug(dsrApp) << "End The Clock!";
    qCDebug(dsrApp) << "onStop method finished.";
}

/**
   @brief 接收"录制中"消息，防止由于其它原因导致截图录屏退出而托盘图标未复原
 */
void ShotStartRecordPlugin::onRecording()
{
    qCDebug(dsrApp) << "onRecording method called.";
    qCDebug(dsrApp) << "(onRecording) Is Recording" << m_isRecording;
    m_nextCount++;
    if (1 == m_nextCount) {
        qCDebug(dsrApp) << "m_nextCount is 1.";
        if (!m_checkTimer) {
            qCDebug(dsrApp) << "Creating check timer";
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

/**
   @brief 接收暂停信号并设置快捷面板图标状态
 */
void ShotStartRecordPlugin::onPause()
{
    qCDebug(dsrApp) << "onPause method called.";
    qCDebug(dsrApp) << "(onPause) Is Recording? " << m_isRecording;
    m_quickPanelWidget->pause();
    qCDebug(dsrApp) << "Quick panel widget paused.";
    qCDebug(dsrApp) << "Pause The Clock!";
    qCDebug(dsrApp) << "onPause method finished.";
}

/**
   @brief 快捷面板点击时弹出截图录屏录制
 */
void ShotStartRecordPlugin::onClickQuickPanel()
{
    qCDebug(dsrApp) << "(onClickQuickPanel) 点击快捷面板";
    m_proxyInter->requestSetAppletVisible(this, pluginName(), false);
    qCDebug(dsrApp) << "Get Record DBus Interface";
    QDBusInterface recordDBusInterface(
        "com.deepin.ScreenRecorder", "/com/deepin/ScreenRecorder", "com.deepin.ScreenRecorder", QDBusConnection::sessionBus());
    recordDBusInterface.asyncCall("stopRecord");
    qCDebug(dsrApp) << "Recorder plugin stop run!";
}

/**
   @brief 录制过程中给设置任务栏图标隐藏，但快捷面板图标仍显示。
 */
void ShotStartRecordPlugin::setTrayIconVisible(bool visible)
{
    qCDebug(dsrApp) << "Setting tray icon visibility to:" << visible;
    // If the OS version is later than V23 or V25, the new version of the API is used
    QDBusInterface interface(DOCK_NAME, DOCK_PATH, DOCK_INTERFACE);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    qCDebug(dsrApp) << "Using Qt6 DBus interface";
    interface.call("setItemOnDock", "Dock_Quick_Plugins", pluginName(), visible);
#else
    qCDebug(dsrApp) << "Using legacy DBus interface";
    // 使用DBus接口仅隐藏任务栏图标，快捷面板图标仍显示，参数是插件的 displayName（历史原因）
    interface.call("setPluginVisible", pluginDisplayName(), visible);
#endif
    qCDebug(dsrApp) << "setTrayIconVisible method finished.";
}

/**
   @return 当前图标在任务栏是否可见
 */
bool ShotStartRecordPlugin::getTrayIconVisible()
{
    qCDebug(dsrApp) << "Checking tray icon visibility";
    QDBusInterface interface(DOCK_NAME, DOCK_PATH, DOCK_INTERFACE);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    qCDebug(dsrApp) << "Using Qt6 DBus interface";
    QDBusReply<QList<DockItemInfo> > msg = interface.call("plugins");

    if (!msg.isValid()) {
        qWarning() << "get tray item info failed: " << msg.error().message();
        return false;
    }

    auto itemList = msg.value();
    auto currentPluginItr = std::find_if(itemList.begin(), itemList.end(), [=](const DockItemInfo &item){
        return item.itemKey == pluginName();
    });

    if (currentPluginItr == itemList.end()) {
        qWarning() << "can not find current plugin info";
        return false;
    }
    qCDebug(dsrApp) << "Found tray icon, visibility:" << currentPluginItr->visible;
    return currentPluginItr->visible;

#else
    qCDebug(dsrApp) << "Using legacy DBus interface";
    auto msg = interface.call("getPluginVisible", pluginDisplayName());
    if (QDBusMessage::ReplyMessage == msg.type()) {
        bool visible = msg.arguments().takeFirst().toBool();
        qCDebug(dsrApp) << "Received plugin visibility:" << visible;
        return visible;
    }
    qCWarning(dsrApp) << "Failed to get plugin visibility";
    return false;
#endif
}

ShotStartRecordPlugin::~ShotStartRecordPlugin()
{
    qCDebug(dsrApp) << "Destroying ShotStartRecordPlugin";
    if (nullptr != m_iconWidget) {
        qCDebug(dsrApp) << "Cleaning up icon widget";
        m_iconWidget->deleteLater();
    } else {
        qCDebug(dsrApp) << "Icon widget is null, no deletion needed.";
    }

    if (nullptr != m_tipsWidget) {
        qCDebug(dsrApp) << "Cleaning up tips widget";
        m_tipsWidget->deleteLater();
    } else {
        qCDebug(dsrApp) << "Tips widget is null, no deletion needed.";
    }

    if (nullptr != m_quickPanelWidget) {
        qCDebug(dsrApp) << "Cleaning up quick panel widget";
        m_quickPanelWidget->deleteLater();
    } else {
        qCDebug(dsrApp) << "Quick panel widget is null, no deletion needed.";
    }
    qCDebug(dsrApp) << "ShotStartRecordPlugin destructor finished.";
}
