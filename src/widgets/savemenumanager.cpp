// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "savemenumanager.h"
#include "../utils/configsettings.h"
#include "../utils/log.h"

#include <QAction>
#include <QActionGroup>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

SaveMenuManager::SaveMenuManager(QWidget *parent)
    : QObject(parent)
    , m_saveMenu(nullptr)
    , m_saveOptionGroup(nullptr)
    , m_askEachTimeAction(nullptr)
    , m_specifiedLocationAction(nullptr)
    , m_specifiedLocationSubMenu(nullptr)
    , m_locationGroup(nullptr)
    , m_chooseOnSaveAction(nullptr)
    , m_desktopAction(nullptr)
    , m_picturesAction(nullptr)
    , m_customPathAction(nullptr)
    , m_updateOnSaveAction(nullptr)
    , m_currentSaveOption(SaveOption::AskEachTime)
    , m_currentLocationState(LocationState::ChooseOnSave)
    , m_currentCustomPath()
{
    createMenu();
    setupConnections();
    initializeFromConfig();
}

SaveMenuManager::~SaveMenuManager()
{
    if (m_saveMenu) {
        m_saveMenu->deleteLater();
    }
}

void SaveMenuManager::createMenu()
{
    m_saveMenu = new DMenu(qobject_cast<QWidget*>(parent()));
    DFontSizeManager::instance()->bind(m_saveMenu, DFontSizeManager::T6);
    
    createSaveOptionActions();
    createLocationActions();
}

void SaveMenuManager::createSaveOptionActions()
{
    // 创建保存选项组 - 包含"每次询问"和"指定位置"，实现互斥
    m_saveOptionGroup = new QActionGroup(this);
    m_saveOptionGroup->setExclusive(true);
    
    // 每次询问选项
    m_askEachTimeAction = new QAction(tr("Each inquiry"), m_saveMenu);
    m_askEachTimeAction->setCheckable(true);
    m_saveOptionGroup->addAction(m_askEachTimeAction);
    
    // 指定位置选项 - 需要设置为checkable以参与互斥，同时还有子菜单
    m_specifiedLocationAction = new QAction(tr("Specified location"), m_saveMenu);
    m_specifiedLocationAction->setCheckable(true);
    m_saveOptionGroup->addAction(m_specifiedLocationAction);
    
    // 添加到菜单
    m_saveMenu->addAction(m_askEachTimeAction);
    m_saveMenu->addAction(m_specifiedLocationAction);
}

void SaveMenuManager::createLocationActions()
{
    // 创建指定位置子菜单，设置给"指定位置"Action
    m_specifiedLocationSubMenu = new DMenu(tr("Specified location"), m_saveMenu);
    DFontSizeManager::instance()->bind(m_specifiedLocationSubMenu, DFontSizeManager::T6);
    
    // 创建位置选项组
    m_locationGroup = new QActionGroup(this);
    m_locationGroup->setExclusive(true);
    
    // "保存到桌面"
    m_desktopAction = new QAction(tr("Desktop"), m_specifiedLocationSubMenu);
    m_desktopAction->setCheckable(true);
    m_locationGroup->addAction(m_desktopAction);
    m_specifiedLocationSubMenu->addAction(m_desktopAction);
    
    // "保存到图片" 
    m_picturesAction = new QAction(tr("Pictures"), m_specifiedLocationSubMenu);
    m_picturesAction->setCheckable(true);
    m_locationGroup->addAction(m_picturesAction);
    m_specifiedLocationSubMenu->addAction(m_picturesAction);
    
    // "保存时选择位置"
    m_chooseOnSaveAction = new QAction(tr("Select a location when saving"), m_specifiedLocationSubMenu);
    m_chooseOnSaveAction->setCheckable(true);
    m_chooseOnSaveAction->setChecked(true); // 默认选中
    m_locationGroup->addAction(m_chooseOnSaveAction);
    m_specifiedLocationSubMenu->addAction(m_chooseOnSaveAction);
    
    // 自定义路径选项 - 当有路径时显示
    m_customPathAction = new QAction("", m_specifiedLocationSubMenu);
    m_customPathAction->setCheckable(true);
    m_customPathAction->setVisible(false); // 默认隐藏，有路径时才显示
    m_locationGroup->addAction(m_customPathAction);
    m_specifiedLocationSubMenu->addAction(m_customPathAction);
    
    // "保存时更新位置" - 当有路径时显示
    m_updateOnSaveAction = new QAction(tr("Update the location when saving"), m_specifiedLocationSubMenu);
    m_updateOnSaveAction->setCheckable(true);
    m_updateOnSaveAction->setVisible(false); // 默认隐藏，有路径时才显示
    m_locationGroup->addAction(m_updateOnSaveAction);
    m_specifiedLocationSubMenu->addAction(m_updateOnSaveAction);
    
    // 将子菜单正确设置给"指定位置"Action - 这是标准的二级菜单做法
    m_specifiedLocationAction->setMenu(m_specifiedLocationSubMenu);
}

void SaveMenuManager::setupConnections()
{
    // 主要保存选项信号连接（互斥组）
    connect(m_saveOptionGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
            this, &SaveMenuManager::onSaveOptionTriggered);
    
    // 位置选项信号连接
    connect(m_locationGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
            this, &SaveMenuManager::onLocationActionTriggered);
}

void SaveMenuManager::initializeFromConfig()
{
    int saveWay = ConfigSettings::instance()->getValue("shot", "save_ways").toInt();
    int saveOp = ConfigSettings::instance()->getValue("shot", "save_op").toInt();
    int locationState = ConfigSettings::instance()->getValue("shot", "location_state").toInt();
    QString savedPath = ConfigSettings::instance()->getValue("shot", "save_dir").toString();
    bool isChangeOnSave = ConfigSettings::instance()->getValue("shot", "save_dir_change").toBool();
    qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - save_ways:" << saveWay 
                      << "save_op:" << saveOp << "location_state:" << locationState
                      << "save_dir:" << savedPath << "save_dir_change:" << isChangeOnSave;
    
    if (saveWay == 0) { // Ask - 每次询问（默认）
        m_currentSaveOption = SaveOption::AskEachTime;
        m_askEachTimeAction->setChecked(true);
        m_specifiedLocationAction->setChecked(false);
        m_currentLocationState = LocationState::ChooseOnSave;
        
        // 每次询问模式下，保留当前save_dir作为参考路径
        if (!savedPath.isEmpty() && QFileInfo::exists(savedPath)) {
            m_currentCustomPath = savedPath;
            qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - AskEachTime mode, keeping save_dir as reference:" << savedPath;
            updateSubMenuForExistingPath();
        } else {
            updateSubMenuForFirstTime();
        }
        
        m_chooseOnSaveAction->setChecked(false);
        m_desktopAction->setChecked(false);
        m_picturesAction->setChecked(false);
        m_customPathAction->setChecked(false);
        m_updateOnSaveAction->setChecked(false);
        
    } else { // SpecifyLocation - 指定位置
        m_currentSaveOption = SaveOption::SpecifiedLocation;
        m_askEachTimeAction->setChecked(false);
        m_specifiedLocationAction->setChecked(true);
        
        if (locationState == static_cast<int>(LocationState::Desktop)) {
            // 保存到桌面
            m_currentLocationState = LocationState::Desktop;
            m_desktopAction->setChecked(true);
            qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - 保存到桌面";
            
            // 检查是否有历史自定义路径
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath) &&
                savedPath != getStandardPath(LocationState::Desktop) &&
                savedPath != getStandardPath(LocationState::Pictures)) {
                m_currentCustomPath = savedPath;
                updateSubMenuForExistingPath();
                qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - 保留历史路径:" << savedPath;
            } else {
                updateSubMenuForFirstTime();
                qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - 无历史路径";
            }
            
        } else if (locationState == static_cast<int>(LocationState::Pictures)) {
            // 保存到图片
            m_currentLocationState = LocationState::Pictures;
            m_picturesAction->setChecked(true);
            qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - 保存到图片";
            
            // 检查是否有历史自定义路径
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath) &&
                savedPath != getStandardPath(LocationState::Desktop) &&
                savedPath != getStandardPath(LocationState::Pictures)) {
                m_currentCustomPath = savedPath;
                updateSubMenuForExistingPath();
                qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - 保留历史路径:" << savedPath;
            } else {
                updateSubMenuForFirstTime();
                qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - 无历史路径";
            }
            
        } else if (saveOp == 1) { // 兼容旧配置
            m_currentLocationState = LocationState::Desktop;
            m_desktopAction->setChecked(true);
            ConfigSettings::instance()->setValue("shot", "location_state", static_cast<int>(LocationState::Desktop));
            
            // 检查历史路径
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath) &&
                savedPath != getStandardPath(LocationState::Desktop) &&
                savedPath != getStandardPath(LocationState::Pictures)) {
                m_currentCustomPath = savedPath;
                updateSubMenuForExistingPath();
            } else {
                updateSubMenuForFirstTime();
            }
            qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - 兼容旧配置，保存到桌面";
            
        } else if (saveOp == 2) { // 兼容旧配置
            m_currentLocationState = LocationState::Pictures;
            m_picturesAction->setChecked(true);
            ConfigSettings::instance()->setValue("shot", "location_state", static_cast<int>(LocationState::Pictures));
            
            // 检查历史路径
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath) &&
                savedPath != getStandardPath(LocationState::Desktop) &&
                savedPath != getStandardPath(LocationState::Pictures)) {
                m_currentCustomPath = savedPath;
                updateSubMenuForExistingPath();
            } else {
                updateSubMenuForFirstTime();
            }
            qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - 兼容旧配置，保存到图片";
            
        } else if (isChangeOnSave) {
            // save_dir_change=true
            m_currentLocationState = LocationState::ChooseOnSave;
            m_chooseOnSaveAction->setChecked(true);
            qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - SpecifiedLocation mode, choose on save (save_dir_change=true)";
            
            // 如果有路径，显示为可选项
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath)) {
                m_currentCustomPath = savedPath;
                updateSubMenuForExistingPath();
            } else {
                updateSubMenuForFirstTime();
            }
            
        } else {
            // save_dir_change=false：使用固定路径
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath)) {
                m_currentCustomPath = savedPath;
                m_currentLocationState = LocationState::CustomPath;
                updateSubMenuForExistingPath();
                m_customPathAction->setChecked(true);
                qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - SpecifiedLocation mode, using fixed save_dir:" << savedPath;
            } else {
                // 路径无效，回退到"保存时选择"
                m_currentLocationState = LocationState::ChooseOnSave;
                updateSubMenuForFirstTime();
                m_chooseOnSaveAction->setChecked(true);
                qCWarning(dsrApp) << "SaveMenuManager::initializeFromConfig - SpecifiedLocation mode, invalid save_dir, fallback to choose on save";
            }
        }
    }
    m_updateOnSaveAction->setChecked(isChangeOnSave);
    qCDebug(dsrApp) << "SaveMenuManager initialized - SaveOption:" << static_cast<int>(m_currentSaveOption)
                    << "LocationState:" << static_cast<int>(m_currentLocationState)
                    << "CustomPath:" << m_currentCustomPath;
}

void SaveMenuManager::onSaveOptionTriggered(QAction *action)
{
    if (action == m_askEachTimeAction) {
        m_currentSaveOption = SaveOption::AskEachTime;
        m_currentLocationState = LocationState::ChooseOnSave;
        
        qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - switching to AskEachTime, m_currentCustomPath before:" << m_currentCustomPath;
        
        // 每次询问模式不改变当前路径，保持现有路径作为参考
        
        // 清除子菜单选中状态
        m_chooseOnSaveAction->setChecked(false);
        m_customPathAction->setChecked(false);
        m_updateOnSaveAction->setChecked(false);
        
        updateConfigSettings();
        emit saveOptionChanged(m_currentSaveOption);
        
        qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - switched to AskEachTime, m_currentCustomPath after:" << m_currentCustomPath;
        
    } else if (action == m_specifiedLocationAction) {
        m_currentSaveOption = SaveOption::SpecifiedLocation;
        
        // 切换到"指定位置"时，检查配置状态
        int locationState = ConfigSettings::instance()->getValue("shot", "location_state").toInt();
        int saveOp = ConfigSettings::instance()->getValue("shot", "save_op").toInt();
        QString savedPath = ConfigSettings::instance()->getValue("shot", "save_dir").toString();
        bool isChangeOnSave = ConfigSettings::instance()->getValue("shot", "save_dir_change").toBool();
        qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - switching to SpecifiedLocation"
                          << "location_state:" << locationState << "save_op:" << saveOp
                          << "save_dir:" << savedPath << "save_dir_change:" << isChangeOnSave;
        
        // 根据配置恢复子菜单状态
        if (locationState == static_cast<int>(LocationState::Desktop) || saveOp == 1) {
            // 保存到桌面
            m_currentLocationState = LocationState::Desktop;
            m_desktopAction->setChecked(true);
            
            // 检查是否有历史自定义路径
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath) &&
                savedPath != getStandardPath(LocationState::Desktop) &&
                savedPath != getStandardPath(LocationState::Pictures)) {
                m_currentCustomPath = savedPath;
                updateSubMenuForExistingPath();
                qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - 恢复保存到桌面，保留历史路径";
            } else {
                updateSubMenuForFirstTime();
                qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - 恢复保存到桌面，无历史路径";
            }
            
        } else if (locationState == static_cast<int>(LocationState::Pictures) || saveOp == 2) {
            // 保存到图片
            m_currentLocationState = LocationState::Pictures;
            m_picturesAction->setChecked(true);
            
            // 检查是否有历史自定义路径
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath) &&
                savedPath != getStandardPath(LocationState::Desktop) &&
                savedPath != getStandardPath(LocationState::Pictures)) {
                m_currentCustomPath = savedPath;
                updateSubMenuForExistingPath();
                qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - 恢复保存到图片，保留历史路径";
            } else {
                updateSubMenuForFirstTime();
                qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - 恢复保存到图片，无历史路径";
            }
            
        } else if (isChangeOnSave) {
            // save_dir_change=true：保存时选择路径
            m_currentLocationState = LocationState::ChooseOnSave;
            m_chooseOnSaveAction->setChecked(true);
            qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - choose on save (save_dir_change=true)";
            
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath)) {
                m_currentCustomPath = savedPath;
                updateSubMenuForExistingPath();
            } else {
                updateSubMenuForFirstTime();
            }
        } else {
            // save_dir_change=false：使用固定路径
            if (!savedPath.isEmpty() && QFileInfo::exists(savedPath)) {
                m_currentCustomPath = savedPath;
                m_currentLocationState = LocationState::CustomPath;
                updateSubMenuForExistingPath();
                m_customPathAction->setChecked(true);
                qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - using fixed save_dir:" << savedPath;
            } else {
                // 路径无效，回退到"保存时选择"
                m_currentLocationState = LocationState::ChooseOnSave;
                updateSubMenuForFirstTime();
                m_chooseOnSaveAction->setChecked(true);
                qCWarning(dsrApp) << "SaveMenuManager::onSaveOptionTriggered - invalid save_dir, fallback to choose on save";
            }
        }
        updateConfigSettings();
        emit saveOptionChanged(m_currentSaveOption, m_currentLocationState);
        
        qCDebug(dsrApp) << "Save option changed to: SpecifiedLocation";
    }
}

void SaveMenuManager::onLocationActionTriggered(QAction *action)
{
    // 点击子菜单项意味着选择了"指定位置"
    m_currentSaveOption = SaveOption::SpecifiedLocation;
    
    // 确保父级"指定位置"被选中，"每次询问"被取消选中
    m_askEachTimeAction->setChecked(false);
    m_specifiedLocationAction->setChecked(true);
    
    if (action == m_chooseOnSaveAction) {
        m_currentLocationState = LocationState::ChooseOnSave;
        
    } else if (action == m_desktopAction) {
        m_currentLocationState = LocationState::Desktop;
        qCWarning(dsrApp) << "SaveMenuManager::onLocationActionTriggered - 选择保存到桌面";
        
    } else if (action == m_picturesAction) {
        m_currentLocationState = LocationState::Pictures;
        qCWarning(dsrApp) << "SaveMenuManager::onLocationActionTriggered - 选择保存到图片";
        
    } else if (action == m_customPathAction) {
        // 点击自定义路径，保持当前路径不变
        m_currentLocationState = LocationState::CustomPath;
        
    } else if (action == m_updateOnSaveAction) {
        // 点击"保存时更新位置"，切换到选择状态
        m_currentLocationState = LocationState::ChooseOnSave;
    }
    
    updateConfigSettings();
    emit saveOptionChanged(m_currentSaveOption, m_currentLocationState);
    
    qCDebug(dsrApp) << "Location action triggered:" << action->text() 
                    << "State:" << static_cast<int>(m_currentLocationState);
}

void SaveMenuManager::updateConfigSettings()
{
    qCWarning(dsrApp) << "SaveMenuManager::updateConfigSettings - current option:" << static_cast<int>(m_currentSaveOption) << "location state:" << static_cast<int>(m_currentLocationState) << "custom path:" << m_currentCustomPath;
    

    ConfigSettings::instance()->setValue("shot", "location_state", static_cast<int>(m_currentLocationState));
    
    if (m_currentSaveOption == SaveOption::AskEachTime) {
        ConfigSettings::instance()->setValue("shot", "save_ways", 0); // SaveWays::Ask
        qCWarning(dsrApp) << "SaveMenuManager::updateConfigSettings - 每次询问: 只设置 save_ways=Ask(0)";
        
    } else { // SpecifiedLocation
        ConfigSettings::instance()->setValue("shot", "save_ways", 1); // SaveWays::SpecifyLocation
        
        switch (m_currentLocationState) {
        case LocationState::Desktop:
            // 保存到桌面
            ConfigSettings::instance()->setValue("shot", "save_op", 1); // SaveAction::SaveToDesktop
            ConfigSettings::instance()->setValue("shot", "save_dir_change", false);
            qCWarning(dsrApp) << "SaveMenuManager::updateConfigSettings - 保存到桌面: save_op=1, 不更新save_dir";
            break;
            
        case LocationState::Pictures:
            // 保存到图片
            ConfigSettings::instance()->setValue("shot", "save_op", 2); // SaveAction::SaveToImage
            ConfigSettings::instance()->setValue("shot", "save_dir_change", false);
            qCWarning(dsrApp) << "SaveMenuManager::updateConfigSettings - 保存到图片: save_op=2, 不更新save_dir";
            break;
            
        case LocationState::CustomPath:
            // 自定义固定路径
            ConfigSettings::instance()->setValue("shot", "save_op", 3); // SaveAction::SaveToSpecificDir
            ConfigSettings::instance()->setValue("shot", "save_dir_change", false);
            if (!m_currentCustomPath.isEmpty()) {
                ConfigSettings::instance()->setValue("shot", "save_dir", m_currentCustomPath);
            }
            qCWarning(dsrApp) << "SaveMenuManager::updateConfigSettings - 自定义固定路径: save_op=3, save_dir=" << m_currentCustomPath;
            break;
            
        case LocationState::ChooseOnSave:
            // 保存时选择路径
            ConfigSettings::instance()->setValue("shot", "save_op", 3); // SaveAction::SaveToSpecificDir
            ConfigSettings::instance()->setValue("shot", "save_dir_change", true);
            qCWarning(dsrApp) << "SaveMenuManager::updateConfigSettings - 保存时选择: save_op=3, save_dir_change=true";
            break;
        }
    }
    
    qCWarning(dsrApp) << "Config最终状态 - save_ways:" << 
        ConfigSettings::instance()->getValue("shot", "save_ways").toInt()
        << "save_op:" << ConfigSettings::instance()->getValue("shot", "save_op").toInt()
        << "location_state:" << ConfigSettings::instance()->getValue("shot", "location_state").toInt()
        << "save_dir_change:" << ConfigSettings::instance()->getValue("shot", "save_dir_change").toBool()
        << "save_dir:" << ConfigSettings::instance()->getValue("shot", "save_dir").toString();
}

SaveOption SaveMenuManager::getCurrentSaveOption() const
{
    return m_currentSaveOption;
}

LocationState SaveMenuManager::getCurrentLocationState() const
{
    return m_currentLocationState;
}

QString SaveMenuManager::getCurrentCustomPath() const
{
    return m_currentCustomPath;
}

void SaveMenuManager::updateCustomPath(const QString &path)
{
    if (!path.isEmpty() && QFileInfo::exists(path)) {
        m_currentCustomPath = path;
        
        // 根据当前保存模式决定如何处理
        if (m_currentSaveOption == SaveOption::AskEachTime) {
            // "每次询问"模式下，只更新参考路径到 save_ask_dir
            ConfigSettings::instance()->setValue("shot", "save_ask_dir", path);
            qCWarning(dsrApp) << "SaveMenuManager::updateCustomPath - AskEachTime mode, updated save_ask_dir:" << path;
        } else {
            // "指定位置"模式下，正式设置为指定路径
            m_currentLocationState = LocationState::CustomPath;
            updateSubMenuForExistingPath();
            m_customPathAction->setChecked(true);
            updateConfigSettings();
            qCWarning(dsrApp) << "SaveMenuManager::updateCustomPath - SpecifiedLocation mode, updated save_dir:" << path;
        }
        
        emit customPathChanged(formatDisplayPath(path));
        emit saveOptionChanged(m_currentSaveOption, m_currentLocationState);
        
        qCDebug(dsrApp) << "Custom path updated to:" << path;
    }
}


void SaveMenuManager::updateSubMenuForFirstTime()
{
    // 首次使用或没有路径时，只显示"保存时选择位置"
    m_chooseOnSaveAction->setVisible(true);
    m_customPathAction->setVisible(false);
    m_updateOnSaveAction->setVisible(false);
}

void SaveMenuManager::updateSubMenuForExistingPath()
{
    // 有现有路径时，"保存时选择位置"被"保存时更新位置"替换
    // 只显示：路径 + "保存时更新位置"
    m_chooseOnSaveAction->setVisible(false);  // 隐藏"保存时选择位置"
    m_customPathAction->setVisible(true);
    m_customPathAction->setText(formatDisplayPath(m_currentCustomPath));
    m_updateOnSaveAction->setVisible(true);   // 显示"保存时更新位置"
}

QString SaveMenuManager::formatDisplayPath(const QString &fullPath) const
{
    if (fullPath.isEmpty()) {
        return tr("Select a location when saving");
    }
    
    QFileInfo fileInfo(fullPath);
    QString folderName = fileInfo.fileName();
    QString parentDir = fileInfo.dir().dirName();
    
    // 限制本文件夹名称为10个字符
    if (folderName.length() > 10) {
        folderName = folderName.left(10) + "...";
    }
    
    // 格式：/上级文件夹/本文件夹
    return QString("/%1/%2").arg(parentDir).arg(folderName);
}

QString SaveMenuManager::getStandardPath(LocationState state) const
{
    switch (state) {
    case LocationState::Desktop:
        return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    case LocationState::Pictures:
        return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    default:
        return QString();
    }
}
