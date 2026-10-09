// Copyright (C) 2019 ~ 2026 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "service/ocrinterface.h"
#include "table/tablerecognizerloader.h"
#include "ui/menucontroller.h"
#include "ui/tablerecognizingwidget.h"
#include "ui/tablerecognizetoast.h"
#include "ui/tableresultdialog.h"
#include "ui/toolbarwidget.h"

#include <DWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QShortcut>
#include <QTimer>
#include <QMenu>

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
     * @brief 打开图片并进行表格识别
     * @param 图片对象
     * @param 图片名称
     * @param 贴图窗口显示的坐标
     * @return
     */
    bool openImageForTable(const QImage &image, const QString &name = "", const QPoint &point = QPoint(0, 0));
    /**
     * @brief 保存图片
     */
    void saveImg();
    /**
     * @brief 获取贴图窗口的显示位置
     */
    QPoint getShowPosition();
    /**
     * @brief 检查工具栏可见性
     */
    void checkToolbarVisibility(); 
    /**
     * @brief 启动隐藏工具栏定时器
     */
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

    /**
     * @brief 在贴图工具栏点击“表格识别”按钮：对当前贴图内容重新识别
     */
    void onOpenTableRecognition();

    /**
     * @brief 表格识别结束
     */
    void onTableRecognitionFinished(bool success, const QString &html,
                                    Dtk::TableRecognizer::TableError error);

    void saveToClipboard();
    /**
     * @brief 启动隐藏工具栏定时器
     */
    void startToolbarHideTimer(); 
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
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    /**
     * @brief 快捷键初始化
     */
    void initShortcut();
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
    bool toolbarAttachedToWindow() const;

    /**
     * @brief 启动表格识别流程
     */
    void startTableRecognition();
    /**
     * @brief 显示表格识别提示条
     */
    void showTableToast(const QString &message, const QString &actionText, bool closable);
    /**
     * @brief 清理表格识别相关控件
     */
    void cleanupTableToast();
    /// 表格识别浮层/提示条是独立置顶窗口，主窗口映射后需要重新置顶
    void raiseTableOverlays();

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
    bool m_isfirstTime = false;
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
    ToolBarWidget *m_toolBar;
    /**
     * @brief 保存信息
     */
    //QPair<QString, QString> m_saveInfo;
    QPair<int, int> m_saveInfo;
    /**
     * @brief 保存贴图窗口的显示位置
     */
    QPoint m_showPosition;
    QTimer *m_mouseMonitorTimer = nullptr;
    QTimer *m_hideToolbarTimer = nullptr;

    /**
     * @brief 是否处于表格识别流程
     */
    bool m_isTableRecognizeMode = false;
    /**
     * @brief 表格识别中浮层
     */
    TableRecognizingWidget *m_tableRecognizingWidget = nullptr;
    /**
     * @brief 表格识别提示条
     */
    TableRecognizeToast *m_tableToast = nullptr;
    /**
     * @brief 表格识别成功弹窗（窗口内浮层，随窗口析构）
     */
    TableResultDialog *m_tableResultDialog = nullptr;
    /**
     * @brief 表格识别能力动态加载器
     */
    TableRecognizerLoader *m_tableRecognizerLoader = nullptr;
    /**
     * @brief 是否已经挂接过主窗口映射信号（用于给独立浮层补置顶）
     */
    bool m_tableOverlayRaiseHooked = false;
    /**
     * @brief 表格识别是否等待贴图窗口映射完成后再启动
     */
    bool m_tableRecognizePending = false;
};

#endif // MAINWINDOW_H
