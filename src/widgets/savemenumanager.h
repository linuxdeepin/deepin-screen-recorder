// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SAVEMENUMANAGER_H
#define SAVEMENUMANAGER_H

#include <QObject>
#include <DMenu>

DWIDGET_USE_NAMESPACE

class QAction;
class QActionGroup;

enum class SaveOption {
    AskEachTime,        // 每次询问（默认选项）
    SpecifiedLocation   // 指定位置
};

// 指定位置的状态枚举
enum class LocationState {
    ChooseOnSave,       // 保存时选择位置（首次使用默认状态）
    Desktop,            // 保存到桌面
    Pictures,           // 保存到图片文件夹
    CustomPath          // 已设置自定义路径
};

/**
 * @brief 保存菜单管理器
 * 专门管理SaveButton右侧下拉菜单的保存选项
 * 保持代码清晰整洁，便于维护
 */
class SaveMenuManager : public QObject
{
    Q_OBJECT

public:
    explicit SaveMenuManager(QWidget *parent = nullptr);
    ~SaveMenuManager();

    // 获取菜单实例
    DMenu* getMenu() const { return m_saveMenu; }
    
    // 初始化菜单状态
    void initializeFromConfig();
    
    // 获取当前保存设置
    SaveOption getCurrentSaveOption() const;
    LocationState getCurrentLocationState() const;
    QString getCurrentCustomPath() const;
    
    // 更新自定义路径（当用户选择文件夹后调用）
    void updateCustomPath(const QString &path);

signals:
    void saveOptionChanged(SaveOption option, LocationState locationState = LocationState::ChooseOnSave);
    void customPathChanged(const QString &displayPath);

private slots:
    void onSaveOptionTriggered(QAction *action);
    void onLocationActionTriggered(QAction *action);

private:
    void createMenu();
    void createSaveOptionActions();
    void createLocationActions();
    void setupConnections();
    void updateConfigSettings();
    QString formatDisplayPath(const QString &fullPath) const;
    void updateSubMenuForFirstTime();
    void updateSubMenuForExistingPath();
    QString getStandardPath(LocationState state) const;

    // UI 组件
    DMenu *m_saveMenu;
    
    // 主要选项组
    QActionGroup *m_saveOptionGroup;
    QAction *m_askEachTimeAction;
    QAction *m_specifiedLocationAction;
    
    // 指定位置的子菜单
    DMenu *m_specifiedLocationSubMenu;
    QActionGroup *m_locationGroup;
    QAction *m_chooseOnSaveAction;       // "保存时选择位置"
    QAction *m_desktopAction;            // "保存到桌面"
    QAction *m_picturesAction;           // "保存到图片"
    QAction *m_customPathAction;         // 显示自定义路径（当有路径时）
    QAction *m_updateOnSaveAction;       // "保存时更新位置"（当有路径时）
    
    // 当前状态
    SaveOption m_currentSaveOption;
    LocationState m_currentLocationState;
    QString m_currentCustomPath;  // 当前自定义路径（完整路径）
};

#endif // SAVEMENUMANAGER_H
