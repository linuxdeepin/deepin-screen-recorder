// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pinsavemenumanager.h"
#include "../settings.h"
#include "../../utils/log.h"

#include <QAction>
#include <QActionGroup>
#include <QFileInfo>
#include <QDir>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

PinSaveMenuManager::PinSaveMenuManager(QWidget *parent)
    : QObject(parent)
    , m_saveMenu(nullptr)
    , m_saveOptionGroup(nullptr)
    , m_askEveryTimeAction(nullptr)
    , m_specifiedLocationAction(nullptr)
    , m_specifiedLocationSubMenu(nullptr)
    , m_customLocationGroup(nullptr)
    , m_saveToSpecialPathAction(nullptr)
    , m_changeSaveToSpecialPath(nullptr)
    , m_currentSavePathType(ASK)
{
    qCWarning(dsrApp) << "PinSaveMenuManager constructor called";
    createMenu();
    setupConnections();
    initializeFromConfig();
    qCWarning(dsrApp) << "PinSaveMenuManager constructor finished";
}

PinSaveMenuManager::~PinSaveMenuManager()
{
    if (m_saveMenu) {
        m_saveMenu->deleteLater();
    }
}

void PinSaveMenuManager::createMenu()
{
    qCWarning(dsrApp) << "PinSaveMenuManager::createMenu() called";
    m_saveMenu = new DMenu(qobject_cast<QWidget*>(parent()));
    DFontSizeManager::instance()->bind(m_saveMenu, DFontSizeManager::T6);
    
    createSaveOptionActions();
    createLocationActions();
}

void PinSaveMenuManager::createSaveOptionActions()
{
    // 创建保存选项组 - 包含"每次询问"和"指定位置"，实现互斥
    m_saveOptionGroup = new QActionGroup(this);
    m_saveOptionGroup->setExclusive(true);
    
    // 每次询问选项
    m_askEveryTimeAction = new QAction(tr("Each inquiry"), m_saveMenu);
    m_askEveryTimeAction->setCheckable(true);
    m_saveOptionGroup->addAction(m_askEveryTimeAction);
    
    // 指定位置选项 - 需要设置为checkable以参与互斥，同时还有子菜单
    m_specifiedLocationAction = new QAction(tr("Specified Location"), m_saveMenu);
    m_specifiedLocationAction->setCheckable(true);
    m_saveOptionGroup->addAction(m_specifiedLocationAction);
    
    // 保存位置标题
    QAction *saveTitleAction = new QAction(tr("Save to"), m_saveMenu);
    saveTitleAction->setDisabled(true);
    
    // 添加到菜单
    m_saveMenu->addAction(saveTitleAction);
    // 注意：m_specifiedLocationSubMenu 将在 createLocationActions 中创建并添加
    m_saveMenu->addAction(m_askEveryTimeAction);
    
    // 保存到映射
    m_savePathActions.insert(ASK, m_askEveryTimeAction);
}

void PinSaveMenuManager::createLocationActions()
{
    qCWarning(dsrApp) << "createLocationActions called";
    // 创建指定位置子菜单，设置给"指定位置"Action
    m_specifiedLocationSubMenu = new DMenu(tr("Specified Location"), m_saveMenu);
    DFontSizeManager::instance()->bind(m_specifiedLocationSubMenu, DFontSizeManager::T6);
    m_specifiedLocationSubMenu->menuAction()->setCheckable(true);
    m_saveOptionGroup->addAction(m_specifiedLocationSubMenu->menuAction());
    
    // 创建自定义位置选项组
    m_customLocationGroup = new QActionGroup(this);
    m_customLocationGroup->setExclusive(true);
    
    // 历史路径选项 - 动态创建
    m_saveToSpecialPathAction = new QAction(m_specifiedLocationSubMenu);
    m_saveToSpecialPathAction->setCheckable(true);
    m_customLocationGroup->addAction(m_saveToSpecialPathAction);
    
    // "保存时选择位置"选项
    m_changeSaveToSpecialPath = new QAction(tr("Set a path on save"), m_specifiedLocationSubMenu);
    m_changeSaveToSpecialPath->setCheckable(true);
    m_customLocationGroup->addAction(m_changeSaveToSpecialPath);
    m_specifiedLocationSubMenu->addAction(m_changeSaveToSpecialPath);
    
    // 添加子菜单到主菜单
    m_saveMenu->addMenu(m_specifiedLocationSubMenu);
    
    // 保存到映射
    m_savePathActions.insert(FOLDER, m_saveToSpecialPathAction);
    m_savePathActions.insert(FOLDER_CHANGE, m_changeSaveToSpecialPath);
}


void PinSaveMenuManager::setupConnections()
{
    // 主要保存选项信号连接（互斥组）
    connect(m_saveOptionGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
            this, &PinSaveMenuManager::onSaveOptionTriggered);
    
    // 自定义位置选项信号连接
    connect(m_customLocationGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
            this, &PinSaveMenuManager::onLocationActionTriggered);
    
}

void PinSaveMenuManager::initializeFromConfig()
{
    // 从Settings读取配置
    QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
    qCWarning(dsrApp) << "PinSaveMenuManager::initializeFromConfig - saveInfo:" << saveInfo;
    
    // 没有配置文件时，给定一个默认值
    if (saveInfo.first == 0) {
        // 首次使用时，默认选择"每次询问"选项
        m_currentSavePathType = ASK;
        qCWarning(dsrApp) << "No configuration found, using default: ASK";
        // 保存默认设置到配置文件
        QPair<int, int> defaultSaveInfo(ASK, saveInfo.second);
        Settings::instance()->setSaveOption(defaultSaveInfo);
    } else {
        m_currentSavePathType = saveInfo.first;
    }
    
    // 处理自定义位置的特殊情况
    if (m_currentSavePathType == FOLDER || m_currentSavePathType == FOLDER_CHANGE) {
        QString specialPath = Settings::instance()->getSavePath();
        qCWarning(dsrApp) << "Retrieved special path from settings:" << specialPath;
        
        if (specialPath.isEmpty() || !QFileInfo::exists(specialPath)) {
            qCWarning(dsrApp) << "Special path not found, path empty:" << specialPath.isEmpty() 
                              << "path exists:" << QFileInfo::exists(specialPath);
            // 没有历史路径时，强制设置为FOLDER_CHANGE模式
            m_currentSavePathType = FOLDER_CHANGE;
            qCWarning(dsrApp) << "No history path, switching to FOLDER_CHANGE mode";
            updateSubMenuForFirstTime();
        } else {
            // 如果存在历史路径，根据getIsChangeSavePath()决定选中哪个选项
            if (Settings::instance()->getIsChangeSavePath()) {
                qCWarning(dsrApp) << "History path exists, but isChangeSavePath is true, using FOLDER_CHANGE";
                m_currentSavePathType = FOLDER_CHANGE;
                // 初始化历史路径选项但选中"保存时选择位置"
                initChangeSaveToSpecialAction(specialPath);
                updateSubMenuForExistingPath();
                // 确保选中"保存时选择位置"选项
                if (m_changeSaveToSpecialPath) {
                    m_changeSaveToSpecialPath->setChecked(true);
                }
                if (m_saveToSpecialPathAction) {
                    m_saveToSpecialPathAction->setChecked(false);
                }
            } else {
                qCWarning(dsrApp) << "History path exists and isChangeSavePath is false, using FOLDER";
                m_currentSavePathType = FOLDER;
                // 初始化历史路径选项并选中它
                initChangeSaveToSpecialAction(specialPath);
                updateSubMenuForExistingPath();
                // 确保选中历史路径选项
                if (m_saveToSpecialPathAction) {
                    m_saveToSpecialPathAction->setChecked(true);
                }
                if (m_changeSaveToSpecialPath) {
                    m_changeSaveToSpecialPath->setChecked(false);
                }
            }
        }
    }
    
    // 设置菜单状态
    qCWarning(dsrApp) << "Setting menu state for SavePathType:" << m_currentSavePathType;
    if (m_currentSavePathType == ASK) {
        m_askEveryTimeAction->setChecked(true);
        qCWarning(dsrApp) << "Set 'Ask every time' action as checked";
    } else {
        m_specifiedLocationSubMenu->menuAction()->setChecked(true);
        qCWarning(dsrApp) << "Set 'Specified Location' submenu as checked";
        
        if (m_savePathActions.contains(m_currentSavePathType)) {
            QAction *action = m_savePathActions.value(m_currentSavePathType);
            if (action) {
                action->setChecked(true);
                qCWarning(dsrApp) << "Set specific location action as checked:" << action->text();
            } else {
                qCWarning(dsrApp) << "Action is null for SavePathType:" << m_currentSavePathType;
            }
        } else {
            qCWarning(dsrApp) << "No action found for SavePathType:" << m_currentSavePathType;
        }
    }
    
    qCWarning(dsrApp) << "PinSaveMenuManager initialized - SavePathType:" << m_currentSavePathType;
}

void PinSaveMenuManager::onSaveOptionTriggered(QAction *action)
{
    if (action == m_askEveryTimeAction) {
        m_currentSavePathType = ASK;
        
        // 清除指定位置的所有选项
        m_specifiedLocationSubMenu->menuAction()->setChecked(false);
        if (m_saveToSpecialPathAction) m_saveToSpecialPathAction->setChecked(false);
        if (m_changeSaveToSpecialPath) m_changeSaveToSpecialPath->setChecked(false);
        
        qCDebug(dsrApp) << "PinSaveMenuManager: switched to AskEachTime";
        
    } else if (action == m_specifiedLocationSubMenu->menuAction()) {
        // 检查子菜单中是否有选中项
        bool hasCheckedChild = false;
        
        if (m_saveToSpecialPathAction && m_saveToSpecialPathAction->isChecked()) {
            m_currentSavePathType = FOLDER;
            hasCheckedChild = true;
        } else if (m_changeSaveToSpecialPath && m_changeSaveToSpecialPath->isChecked()) {
            m_currentSavePathType = FOLDER_CHANGE;
            hasCheckedChild = true;
        }
        
        if (hasCheckedChild) {
            m_askEveryTimeAction->setChecked(false);
            qCDebug(dsrApp) << "PinSaveMenuManager: switched to SpecifiedLocation";
        } else {
            // 如果没有子项被选中，恢复"每次询问"的选中状态
            action->setChecked(false);
            m_askEveryTimeAction->setChecked(true);
            return;
        }
    }
    
    updateConfigSettings();
    emit saveOptionChanged(m_currentSavePathType);
}

void PinSaveMenuManager::onLocationActionTriggered(QAction *action)
{
    // 确保指定位置菜单被选中
    m_specifiedLocationSubMenu->menuAction()->setChecked(true);
    m_askEveryTimeAction->setChecked(false);
    
    if (action == m_saveToSpecialPathAction) {
        m_currentSavePathType = FOLDER;
        Settings::instance()->setIsChangeSavePath(false);
        
    } else if (action == m_changeSaveToSpecialPath) {
        m_currentSavePathType = FOLDER_CHANGE;
        Settings::instance()->setIsChangeSavePath(true);
    }
    
    updateConfigSettings();
    emit saveOptionChanged(m_currentSavePathType);
    
    qCDebug(dsrApp) << "Location action triggered:" << action->text() 
                    << "SavePathType:" << m_currentSavePathType;
}


void PinSaveMenuManager::updateConfigSettings()
{
    // 保持格式不变，只更新路径类型
    QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
    saveInfo.first = m_currentSavePathType;
    Settings::instance()->setSaveOption(saveInfo);
    qCDebug(dsrApp) << "PinSaveMenuManager::updateConfigSettings - saved:" << saveInfo;
}

int PinSaveMenuManager::getCurrentSaveOption() const
{
    return m_currentSavePathType;
}

void PinSaveMenuManager::updateCustomPath(const QString &path)
{
    if (!path.isEmpty() && QFileInfo::exists(path)) {
        Settings::instance()->setSavePath(path);
        
        // 更新历史路径选项
        initChangeSaveToSpecialAction(path);
        updateSubMenuForExistingPath();
        
        // 如果当前是FOLDER_CHANGE模式，切换到FOLDER模式
        if (m_currentSavePathType == FOLDER_CHANGE) {
            m_currentSavePathType = FOLDER;
            Settings::instance()->setIsChangeSavePath(false);
            
            if (m_saveToSpecialPathAction) {
                m_saveToSpecialPathAction->setChecked(true);
            }
            if (m_changeSaveToSpecialPath) {
                m_changeSaveToSpecialPath->setChecked(false);
            }
        }
        
        updateConfigSettings();
        emit customPathChanged(formatDisplayPath(path));
        emit saveOptionChanged(m_currentSavePathType);
        
        qCDebug(dsrApp) << "Custom path updated to:" << path;
    }
}

void PinSaveMenuManager::updateSubMenuForFirstTime()
{
    // 首次使用或没有路径时，只显示"保存时选择位置"
    if (m_saveToSpecialPathAction) {
        m_saveToSpecialPathAction->setVisible(false);
    }
    if (m_changeSaveToSpecialPath) {
        m_changeSaveToSpecialPath->setText(tr("Set a path on save"));
        m_changeSaveToSpecialPath->setVisible(true);
    }
}

void PinSaveMenuManager::updateSubMenuForExistingPath()
{
    qCWarning(dsrApp) << "updateSubMenuForExistingPath called";
    // 有现有路径时，显示路径选项和"保存时更新位置"
    if (m_saveToSpecialPathAction) {
        m_saveToSpecialPathAction->setVisible(true);
        qCWarning(dsrApp) << "Set history path action visible:" << m_saveToSpecialPathAction->text();
    }
    if (m_changeSaveToSpecialPath) {
        m_changeSaveToSpecialPath->setText(tr("Change the path on save"));
        m_changeSaveToSpecialPath->setVisible(true);
        qCWarning(dsrApp) << "Set change path action visible:" << m_changeSaveToSpecialPath->text();
    }
}

void PinSaveMenuManager::initChangeSaveToSpecialAction(const QString &specialPath)
{
    qCWarning(dsrApp) << "initChangeSaveToSpecialAction called with path:" << specialPath;
    if (!m_saveToSpecialPathAction) {
        qCWarning(dsrApp) << "m_saveToSpecialPathAction is null, returning";
        return;
    }
    
    // 处理路径显示逻辑为【/上级文件夹/本文件夹】，本文件夹限制10个字符
    QString displayPath = formatDisplayPath(specialPath);
    
    m_saveToSpecialPathAction->setText(displayPath);
    m_saveToSpecialPathAction->setToolTip(specialPath);
    m_saveToSpecialPathAction->setCheckable(true);
    
    // 确保添加到菜单和映射中
    if (!m_specifiedLocationSubMenu->actions().contains(m_saveToSpecialPathAction)) {
        m_specifiedLocationSubMenu->insertAction(m_changeSaveToSpecialPath, m_saveToSpecialPathAction);
    }
    
    m_savePathActions.insert(FOLDER, m_saveToSpecialPathAction);
}

QString PinSaveMenuManager::formatDisplayPath(const QString &fullPath) const
{
    if (fullPath.isEmpty()) {
        return tr("Set a path on save");
    }
    
    QFileInfo fileInfo(fullPath);
    QString folderName = fileInfo.fileName();
    QString parentDir = fileInfo.dir().dirName();
    
    // 限制本文件夹名称为10个字符
    if (folderName.length() > 10) {
        folderName = folderName.left(7) + "...";
    }
    
    // 格式：/上级文件夹/本文件夹
    return QString("/%1/%2").arg(parentDir).arg(folderName);
}
