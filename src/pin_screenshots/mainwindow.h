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

#include "service/ocrinterface.h"
#include "ui/menucontroller.h"
#include "ui/toolbar.h"

#include <DWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QShortcut>

DWIDGET_USE_NAMESPACE

#define PADDING 10
/**
 * @brief 贴图的主界面
 */
class MainWindow : public DWidget
{
    Q_OBJECT
    enum Direction { UP = 0, DOWN = 1, LEFT, RIGHT, LEFTTOP, LEFTBOTTOM, RIGHTBOTTOM, RIGHTTOP, NONE };
    struct ScreenInfo {
        int x;
        int y;
        int width;
        int height;
        QString name;
        ~ScreenInfo() {}
    };
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
    /**
     * @brief 保存图片
     */
    void saveImg();

public slots:
    /**
     * @brief 贴图保存实现
     */
    void onSave();
    /**
     * @brief 贴图退出实现
     */
    void onExit();
    /**
     * @brief 开启ocr
     */
    void onOpenOCR();
protected:
    /**
     * @brief 贴图主窗口的初始化函数
     */
    void initMainWindow();

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
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    /**
     * @brief 快捷键初始化
     */
    void initShortcut();
    /**
     * @brief 边界计算
     */
    bool boundaryCalculation(QRect &rect);
    /**
     * @brief 计算屏幕
     */
    void CalculateScreenSize();

    /**
     * @brief 处理鼠标右键的操作的函数接口
     */
    void handleMouseRightBtn(QMouseEvent *event);
    /**
     * @brief 发送通知
     */
    void sendNotify(QString savePath, bool bSaveState);
    /**
     * @brief 更新工具栏显示位置
     */
    void updateToolBarPosition(); // 工具栏显示位置

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
    QImage m_image; //图片
    QString m_imageName; //图片名称
    QString m_lastImagePath; // 上一次图片的保存路径
    /**
     * @brief OCR接口
     */
    OcrInterface *m_ocrInterface;
    double proportion = 0.0; //宽高比
    QList<ScreenInfo> m_screenInfo; //所有屏幕信息
    QSize m_screenSize; // 屏幕大小
    /**
     * @brief 屏幕的缩放比例
     * 当屏幕缩放比例为1.25时，此时的屏幕实际大小为1536*864
     *   1.1.如果需将当前屏幕的点换算到1920*1080上需乘上m_pixelRatio
     *   1.2.如果需将1920*1080上的点换算到此屏幕应该除以m_pixelRatio
     */
    qreal m_pixelRatio = 1.0;

    /**
     * @brief 贴图的右键菜单
     */
    MenuController *m_menuController;
    /**
     * @brief 工具栏
     */
    ToolBar *m_toolBar;
    /**
     * @brief 保存信息
     */
    QPair<QString, QString> m_saveInfo;
};

#endif // MAINWINDOW_H
