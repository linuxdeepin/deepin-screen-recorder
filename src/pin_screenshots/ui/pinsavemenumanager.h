// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PINSAVEMENUMANAGER_H
#define PINSAVEMENUMANAGER_H

#include <QObject>
#include <DMenu>
#include <QActionGroup>
#include <QMap>

DWIDGET_USE_NAMESPACE

// 保存选项枚举
enum SavePathType {
    ASK = 1,           // 每次询问
    FOLDER = 4,        // 指定文件夹
    FOLDER_CHANGE = 5  // 保存时选择文件夹
};

/**
 * @brief Pin程序的保存菜单管理器
 * 专门为pin程序提供保存选项菜单，与主程序的SaveMenuManager功能类似
 * 但使用pin程序自己的Settings系统和枚举定义
 */
class PinSaveMenuManager : public QObject
{
    Q_OBJECT

public:
    explicit PinSaveMenuManager(QWidget *parent = nullptr);
    ~PinSaveMenuManager();

    // 获取菜单实例
    DMenu* getMenu() const { return m_saveMenu; }
    
    // 初始化菜单状态（从Settings读取配置）
    void initializeFromConfig();
    
    // 获取当前保存设置
    int getCurrentSaveOption() const;
    
    // 更新自定义路径（当用户选择文件夹后调用）
    void updateCustomPath(const QString &path);

signals:
    void saveOptionChanged(int savePathType);
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
    void initChangeSaveToSpecialAction(const QString &specialPath);

    // UI 组件
    DMenu *m_saveMenu;
    
    // 主要选项组
    QActionGroup *m_saveOptionGroup;
    QAction *m_askEveryTimeAction;
    QAction *m_specifiedLocationAction;
    
    // 指定位置的子菜单（简化版）
    DMenu *m_specifiedLocationSubMenu;
    QActionGroup *m_customLocationGroup;
    QAction *m_saveToSpecialPathAction;     // 显示历史路径
    QAction *m_changeSaveToSpecialPath;     // "保存时选择位置"
    
    // 当前状态
    int m_currentSavePathType;  // SavePathType
    
    // 路径映射
    QMap<int, QAction *> m_savePathActions;
};

#endif // PINSAVEMENUMANAGER_H
