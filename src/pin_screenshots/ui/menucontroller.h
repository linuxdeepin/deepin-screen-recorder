/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Wang Cong <wangcong@uniontech.com>
 *
 * Maintainer: He MingYang <hemingyang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    void enterEvent(QEvent *e);
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
