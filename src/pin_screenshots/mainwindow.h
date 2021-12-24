/*

* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.

*

* Author:     wangcong <wangcong@uniontech.com>

*

* Maintainer: wangcong <wangcong@uniontech.com>

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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QDebug>
DWIDGET_USE_NAMESPACE

#define PADDING 2
/**
 * @brief 贴图的主界面
 */
class MainWindow : public DWidget
{
    Q_OBJECT
    enum Direction { UP = 0, DOWN = 1, LEFT, RIGHT, LEFTTOP, LEFTBOTTOM, RIGHTBOTTOM, RIGHTTOP, NONE };
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /**
     * @brief 打开图片
     * @param 图片的路径
     * @return 打开是否成功
     */
    bool openFile(const QString &filePaths);

    /**
     * @brief 打开图片
     * @param 图片对象
     * @return
     */
    bool openImage(const QImage &image);

    /**
     * @brief 打开图片
     * @param 图片对象
     * @param 图片名称
     * @return
     */
    bool openImageAndName(const QImage &image, const QString &name = "", const QPoint &point = QPoint(0, 0));
protected:
    /**
     * @brief 根据鼠标的位置，改变光标的形状
     * @param currentGlobalPoint
     */
    void region(const QPoint &currentGlobalPoint);  //鼠标的位置,改变光标
    //鼠标按下移动及释放事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *e) override;

private:
    /**
     * @brief 按下鼠标的位置
     */
    QPoint dragPosition;
    /**
     * @brief 鼠标左键是否按下
     */
    bool isLeftPressDown;
    /**
     * @brief 窗口大小改变时，记录改变方向
     */
    Direction dir;

    QImage m_image;

};

#endif // MAINWINDOW_H
