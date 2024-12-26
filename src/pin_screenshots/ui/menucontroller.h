// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include <DMenu>
DWIDGET_USE_NAMESPACE
enum SaveAction : unsigned int;
/**
 * @brief 这是贴图的右键菜单类
 */
class MenuController : public QObject
{
    Q_OBJECT
public:
    explicit MenuController(QObject *parent = nullptr);
    ~MenuController();

signals:
    void saveAction();
    void closeAction();
    void menuNoFocus();

public slots:
    void showMenu(QPoint pos);

protected:
    /**
     * @brief 初始化函数
     */
    void initMenuController();
    /**
     * @brief 进入事件
     * @param e
     */
    // qt6 接口有变动
#if (QT_VERSION_MAJOR == 5)
    void enterEvent(QEvent *e);
#elif (QT_VERSION_MAJOR == 6)
    void enterEvent(QEnterEvent *e);;
#endif
private:
    DMenu *m_menu;
    /**
     * @brief 保存
     */
    QAction *m_saveAct;
    /**
     * @brief 关闭
     */
    QAction *m_closeAct;
};
#endif // MENUCONTROLLER_H
