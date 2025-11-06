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
#include <QStandardPaths>
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
    , m_desktopAction(nullptr)
    , m_picturesAction(nullptr)
    , m_saveToSpecialPathAction(nullptr)
    , m_changeSaveToSpecialPath(nullptr)
    , m_currentSavePathType(ASK)
{
    createMenu();
    setupConnections();
    initializeFromConfig();
}

PinSaveMenuManager::~PinSaveMenuManager()
{
    if (m_saveMenu) {
        m_saveMenu->deleteLater();
    }
}

void PinSaveMenuManager::createMenu()
{
    m_saveMenu = new DMenu(qobject_cast<QWidget*>(parent()));
    DFontSizeManager::instance()->bind(m_saveMenu, DFontSizeManager::T6);
    
    createSaveOptionActions();
    createLocationActions();
}

void PinSaveMenuManager::createSaveOptionActions()
{
    m_saveOptionGroup = new QActionGroup(this);
    m_saveOptionGroup->setExclusive(true);
    
    m_askEveryTimeAction = new QAction(tr("Each inquiry"), m_saveMenu);
    m_askEveryTimeAction->setCheckable(true);
    m_saveOptionGroup->addAction(m_askEveryTimeAction);
    
    m_specifiedLocationAction = new QAction(tr("Specified Location"), m_saveMenu);
    m_specifiedLocationAction->setCheckable(true);
    m_saveOptionGroup->addAction(m_specifiedLocationAction);
    
    m_saveMenu->addAction(m_askEveryTimeAction);
    
    m_savePathActions.insert(ASK, m_askEveryTimeAction);
}

void PinSaveMenuManager::createLocationActions()
{
    qCWarning(dsrApp) << "createLocationActions called";
    m_specifiedLocationSubMenu = new DMenu(tr("Specified Location"), m_saveMenu);
    DFontSizeManager::instance()->bind(m_specifiedLocationSubMenu, DFontSizeManager::T6);
    m_specifiedLocationSubMenu->menuAction()->setCheckable(true);
    m_saveOptionGroup->addAction(m_specifiedLocationSubMenu->menuAction());
    
    // 创建自定义位置选项组
    m_customLocationGroup = new QActionGroup(this);
    m_customLocationGroup->setExclusive(true);
    
    // 保存到桌面
    m_desktopAction = new QAction(tr("Desktop"), m_specifiedLocationSubMenu);
    m_desktopAction->setCheckable(true);
    m_customLocationGroup->addAction(m_desktopAction);
    m_specifiedLocationSubMenu->addAction(m_desktopAction);
    
    // 保存到图片
    m_picturesAction = new QAction(tr("Pictures"), m_specifiedLocationSubMenu);
    m_picturesAction->setCheckable(true);
    m_customLocationGroup->addAction(m_picturesAction);
    m_specifiedLocationSubMenu->addAction(m_picturesAction);
    
    // 保存时选择位置
    m_changeSaveToSpecialPath = new QAction(tr("Set a path on save"), m_specifiedLocationSubMenu);
    m_changeSaveToSpecialPath->setCheckable(true);
    m_customLocationGroup->addAction(m_changeSaveToSpecialPath);
    m_specifiedLocationSubMenu->addAction(m_changeSaveToSpecialPath);
    
    // 历史路径选项
    m_saveToSpecialPathAction = new QAction(m_specifiedLocationSubMenu);
    m_saveToSpecialPathAction->setCheckable(true);
    m_customLocationGroup->addAction(m_saveToSpecialPathAction);
    
    // 添加子菜单到主菜单
    m_saveMenu->addMenu(m_specifiedLocationSubMenu);
    
    // 保存到映射
    m_savePathActions.insert(DESKTOP, m_desktopAction);
    m_savePathActions.insert(PICTURES, m_picturesAction);
    m_savePathActions.insert(FOLDER, m_saveToSpecialPathAction);
    m_savePathActions.insert(FOLDER_CHANGE, m_changeSaveToSpecialPath);
}


void PinSaveMenuManager::setupConnections()
{
    connect(m_saveOptionGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
            this, &PinSaveMenuManager::onSaveOptionTriggered);
    
    connect(m_customLocationGroup, QOverload<QAction *>::of(&QActionGroup::triggered),
            this, &PinSaveMenuManager::onLocationActionTriggered);
    
}

void PinSaveMenuManager::initializeFromConfig()
{
    QPair<int, int> saveInfo = Settings::instance()->getSaveOption();
    
    if (saveInfo.first == 0) {
        m_currentSavePathType = ASK;
        QPair<int, int> defaultSaveInfo(ASK, saveInfo.second);
        Settings::instance()->setSaveOption(defaultSaveInfo);
    } else {
        m_currentSavePathType = saveInfo.first;
    }
    
    QString specialPath = Settings::instance()->getSavePath();
    const bool hasHistoryPath = !specialPath.isEmpty() && QFileInfo::exists(specialPath);
    qCWarning(dsrApp) << "Retrieved special path from settings:" << specialPath;
    if (hasHistoryPath) {
        initChangeSaveToSpecialAction(specialPath);
        updateSubMenuForExistingPath();
    } else {
        updateSubMenuForFirstTime();
    }

    if (!hasHistoryPath && m_currentSavePathType == FOLDER) {
        m_currentSavePathType = FOLDER_CHANGE;
    }
    
    if (m_currentSavePathType == ASK) {
        m_askEveryTimeAction->setChecked(true);
    } else {
        m_specifiedLocationSubMenu->menuAction()->setChecked(true);
        
        if (m_savePathActions.contains(m_currentSavePathType)) {
            QAction *action = m_savePathActions.value(m_currentSavePathType);
            if (action) {
                action->setChecked(true);
            } else {
            }
        } else {
            qCWarning(dsrApp) << "No action found for SavePathType:" << m_currentSavePathType;
        }
    }
    
    qCDebug(dsrApp) << "PinSaveMenuManager initialized - SavePathType:" << m_currentSavePathType;
}

void PinSaveMenuManager::onSaveOptionTriggered(QAction *action)
{
    if (action == m_askEveryTimeAction) {
        m_currentSavePathType = ASK;
        
        // 清除指定位置的所有选项
        m_specifiedLocationSubMenu->menuAction()->setChecked(false);
        if (m_desktopAction) m_desktopAction->setChecked(false);
        if (m_picturesAction) m_picturesAction->setChecked(false);
        if (m_saveToSpecialPathAction) m_saveToSpecialPathAction->setChecked(false);
        if (m_changeSaveToSpecialPath) m_changeSaveToSpecialPath->setChecked(false);
        
        qCDebug(dsrApp) << "PinSaveMenuManager: switched to AskEachTime";
        
    } else if (action == m_specifiedLocationSubMenu->menuAction()) {
        // 检查子菜单中是否有选中项
        bool hasCheckedChild = false;
        
        if (m_desktopAction && m_desktopAction->isChecked()) {
            m_currentSavePathType = DESKTOP;
            hasCheckedChild = true;
        } else if (m_picturesAction && m_picturesAction->isChecked()) {
            m_currentSavePathType = PICTURES;
            hasCheckedChild = true;
        } else if (m_saveToSpecialPathAction && m_saveToSpecialPathAction->isChecked()) {
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
    m_specifiedLocationSubMenu->menuAction()->setChecked(true);
    m_askEveryTimeAction->setChecked(false);
    
    if (action == m_desktopAction) {
        m_currentSavePathType = DESKTOP;
        Settings::instance()->setIsChangeSavePath(false);
    } else if (action == m_picturesAction) {
        m_currentSavePathType = PICTURES;
        Settings::instance()->setIsChangeSavePath(false);
    } else if (action == m_saveToSpecialPathAction) {
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
        
        initChangeSaveToSpecialAction(path);
        updateSubMenuForExistingPath();
        
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
    
    if (folderName.length() > 10) {
        folderName = folderName.left(7) + "...";
    }
    
    return QString("/%1/%2").arg(parentDir).arg(folderName);
}
