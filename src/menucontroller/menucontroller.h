/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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
class MenuController : public QObject
{
    Q_OBJECT
public:
    explicit MenuController(QObject *parent = nullptr);
    ~MenuController();

signals:
    void shapePressed(QString currentShape);
    void unDoAction();
    void saveBtnPressed(SaveAction action);
    void saveAction();
    void closeAction();
    void menuNoFocus();

public slots:
    void showMenu(QPoint pos);
    void setUndoEnable(bool status);

protected:
    void enterEvent(QEvent *e);
private:
    DMenu *m_menu;
    QAction *m_unDoAct;
    QAction *m_saveAct;
    QAction *m_closeAct;
    qreal m_ration;
};
#endif // MENUCONTROLLER_H
