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
#include "mainwindow.h"

#include <QDebug>
#include <QDBusInterface>
#include <QDesktopWidget>
#include <QScreen>

#define MOVENUM 1
#define WHEELNUM 10

MainWindow::MainWindow(QWidget *parent)
    : DWidget(parent)
{
    m_ocrInterface = nullptr;
    //去菜单栏，置顶窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
//    setAttribute(Qt::WA_TranslucentBackground, true); //设置透明
    //设置可以进行鼠标操作
    setMouseTracking(true);
    resize(500, 500);

    isLeftPressDown = false;
    dir = UP;
    this->setMinimumSize(10,10);
    initShortcut(); // 初始化快捷键
}

MainWindow::~MainWindow()
{

}

bool MainWindow::openFile(const QString &filePaths)
{
    qDebug() << "func: " << __func__ ;
    //测试
    m_image.load(filePaths);
    update();
    return true;
}

bool MainWindow::openImage(const QImage &image)
{
    qDebug() << "func: " << __func__ ;
    m_image = image;
    resize(m_image.width(), m_image.height());

    update();
    return true;
}

bool MainWindow::openImageAndName(const QImage &image, const QString &name, const QPoint &point)
{
    Q_UNUSED(name);
    qDebug() << "func: " << __func__ ;
    m_image = image;
    resize(m_image.width(), m_image.height());
    move(point);
    update();

    return true;
}
// 开启OCR
void MainWindow::openOCR()
{
    // 测试
    if(m_ocrInterface == nullptr)
        m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), this);
    m_ocrInterface->openImageAndName(m_image, "/home/uos/Pictures/screenMapping/ocr.jpg");
}

// 贴图保存
void MainWindow::saveImg()
{
    // 测试
    m_image.save("/home/zhangwc/uos/screenMapping/test.png");
    qApp->quit();
}

void MainWindow::region(const QPoint &cursorGlobalPoint)
{
    // 获取窗体在屏幕上的位置区域，tl为topleft点，rb为rightbottom点
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());

    int x = cursorGlobalPoint.x();
    int y = cursorGlobalPoint.y();

    if (tl.x() + PADDING >= x && tl.x() <= x && tl.y() + PADDING >= y && tl.y() <= y) {
        // 左上角
        dir = LEFTTOP;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));  // 设置鼠标形状
    } else if (x >= rb.x() - PADDING && x <= rb.x() && y >= rb.y() - PADDING && y <= rb.y()) {
        // 右下角
        dir = RIGHTBOTTOM;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if (x <= tl.x() + PADDING && x >= tl.x() && y >= rb.y() - PADDING && y <= rb.y()) {
        //左下角
        dir = LEFTBOTTOM;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if (x <= rb.x() && x >= rb.x() - PADDING && y >= tl.y() && y <= tl.y() + PADDING) {
        // 右上角
        dir = RIGHTTOP;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if (x <= tl.x() + PADDING && x >= tl.x()) {
        // 左边
//        dir = LEFT;
//        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if (x <= rb.x() && x >= rb.x() - PADDING) {
        // 右边
//        dir = RIGHT;
//        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if (y >= tl.y() && y <= tl.y() + PADDING) {
        // 上边
//        dir = UP;
//        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else if (y <= rb.y() && y >= rb.y() - PADDING) {
        // 下边
//        dir = DOWN;
//        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else {
        // 默认
        dir = NONE;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        isLeftPressDown = true;
        if (dir != NONE) {
            this->mouseGrabber();
        } else {
            dragPosition = event->globalPos() - this->frameGeometry().topLeft();
        }
        break;
    case Qt::RightButton:
        this->close();
        break;
    default:
        QWidget::mousePressEvent(event);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint gloPoint = event->globalPos();
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint br = mapToGlobal(rect.bottomRight());
    QPoint bl = mapToGlobal(rect.bottomLeft());
    QPoint tr = mapToGlobal(rect.topRight());

    if (!isLeftPressDown) {
        this->region(gloPoint);
    } else {

        if (dir != NONE) {
            QRect rMove(tl, br);
            double proportion = rMove.width() / rMove.height();
            QPoint newPoint;
            switch (dir) {
 #if 0
            case LEFT:
                if (rb.x() - gloPoint.x() <= this->minimumWidth())
                    rMove.setX(tl.x());
                else
                    rMove.setX(gloPoint.x());
                break;
            case RIGHT:
                rMove.setWidth(gloPoint.x() - tl.x());
                break;
            case UP:
                if (rb.y() - gloPoint.y() <= this->minimumHeight())
                    rMove.setY(tl.y());
                else
                    rMove.setY(gloPoint.y());
                break;
            case DOWN:
                rMove.setHeight(gloPoint.y() - tl.y());
                break;
 #endif
            case LEFTTOP:
                newPoint = QPointF(br.x() - gloPoint.x(), (br.x() - gloPoint.x()) / proportion).toPoint();
                rMove.setX(br.x() - newPoint.x());
                rMove.setY(br.y() - newPoint.y());
                break;
            case RIGHTTOP:
                newPoint = QPointF(tl.x() + (bl.y() - gloPoint.y())*proportion, gloPoint.y()).toPoint();
                rMove.setTopRight(newPoint);
                break;
            case LEFTBOTTOM:
                newPoint = QPointF(gloPoint.x(), tr.y()+ ((tr.x() - gloPoint.x()) / proportion)).toPoint();
                rMove.setBottomLeft(newPoint);
                break;
            case RIGHTBOTTOM:
                newPoint = QPointF(gloPoint.x() - tl.x(), (gloPoint.x() - tl.x()) / proportion).toPoint();
                rMove.setWidth(gloPoint.x() - tl.x());
                rMove.setHeight(newPoint.y());
                break;
            default:
                break;
            }
            this->setGeometry(rMove);
        } else {
            this->setCursor(QCursor(Qt::ClosedHandCursor));
            move(event->globalPos() - dragPosition);
            event->accept();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isLeftPressDown = false;
        if (dir != NONE) {
            this->releaseMouse();
        }
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (!m_image.isNull()) {
        QPainter pp(this);
        QRect Temp(0, 0, this->width(), this->height());

        pp.drawImage(Temp, m_image);
    }


}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int x = this->pos().x();
    int y = this->pos().y();
    if (event->key() == Qt::Key_Left){
        this->move(x - MOVENUM, y);
    }else if (event->key() == Qt::Key_Right) {
        this->move(x + MOVENUM, y);
    }else if (event->key() == Qt::Key_Up) {
        this->move(x, y - MOVENUM);
    }else if (event->key() == Qt::Key_Down) {
        this->move(x, y + MOVENUM);
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    QRect rect;
    int width = this->rect().width();
    int height = this->rect().height();
    QPointF unionPoint(event->posF().x() / width, event->posF().y() / height);
    //qDebug() << unionPoint;
    int x = this->pos().x();
    int y = this->pos().y();
    if(event->delta() < 0 && (width < this->minimumWidth() || height < this->minimumHeight()))
        return;
    if(event->delta() > 0){
        rect.setTopLeft(QPoint(x - WHEELNUM, y - WHEELNUM));
        rect.setTopRight(QPoint(x + width + WHEELNUM, y - WHEELNUM));
        rect.setBottomLeft(QPoint(x - WHEELNUM, y + height + WHEELNUM));
        rect.setBottomRight(QPoint(x + width + WHEELNUM, y + height + WHEELNUM));
    }else {
        rect.setTopLeft(QPoint(x + WHEELNUM, y + WHEELNUM));
        rect.setTopRight(QPoint(x + width - WHEELNUM, y + WHEELNUM));
        rect.setBottomLeft(QPoint(x + WHEELNUM, y + height - WHEELNUM));
        rect.setBottomRight(QPoint(x + width - WHEELNUM, y + height - WHEELNUM));
    }
    QPointF globalPos = event->globalPos(); // 鼠标在屏幕中的坐标
    QPointF newPoint(rect.width()*unionPoint.x(), rect.height()*unionPoint.y()); //扩大后，鼠标在贴图中的坐标
    QPointF topLeft(globalPos.x() - newPoint.x(), globalPos.y() - newPoint.y()); // 贴图左上角的点在屏幕中的坐标
    rect = QRect(topLeft.toPoint(), rect.size());

    qDebug()<<"=====rect : "<<rect;
    if(boundaryCalculation(rect))
        return;
    this->setGeometry(rect.x(),rect.y(),rect.width(),rect.height());
}

// 初始化快捷键
void MainWindow::initShortcut()
{
    // Esc —— 退出
    QShortcut *Keyesc = new QShortcut(QKeySequence("Escape"), this);
    connect(Keyesc, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keyesc (key: esc)";
        qApp->quit();
    });

    // Ctrl+S —— 保存
    QShortcut *Keysave = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(Keysave, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keysave (key: ctrl+s)";
        saveImg();
    });

    // Alt+O —— 文字识别
    QShortcut *Keyocr = new QShortcut(QKeySequence("Alt+O"), this);
    connect(Keyocr, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keyocr (key: alt+o)";
        openOCR();
    });


    // F1 —— 显示帮助 测试
    QShortcut *Keyhelp = new QShortcut(QKeySequence("F1"), this);
    connect(Keyhelp, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keyhelp (key: F1)";
        QDBusInterface interFace("com.deepin.Manual.Open",
                                 "/com/deepin/Manual/Open",
                                 "com.deepin.Manual.Open",
                                 QDBusConnection::sessionBus());
        // 帮助手册跳转到子标题，传入子标题标签(标签文档内唯一，固定为英文，与翻译无关)
        QList<QVariant> arg;
        arg << (QString("deepin-screen-recorder"))                  // 应用名称
            << QString("scrollshot");                         // 帮助文案中的标题名称
        interFace.callWithArgumentList(QDBus::AutoDetect, "OpenTitle", arg);
        qApp->quit();
    });
}
// 贴图窗口边界计算
bool MainWindow::boundaryCalculation(QRect &rect)
{
    //qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QRect mainRect = QApplication::desktop()->screen()->geometry();
    int width = rect.width();
    int height = rect.height();
    int x = rect.x();
    int y = rect.y();
    qDebug()<<"2 rect === "<<rect<<"width"<<width<<"height"<<height;
    bool isOver = false;
    if((x + width >= mainRect.width())){
        int tmpWidth = x + width - mainRect.width();
        x = x - tmpWidth;
        qDebug()<<" ====== x > 0 === ";
        if (x <= 0){
            x = 0;
            isOver = true;
        }
    }

    if((y + height >= mainRect.height())){
       int tmpHeight = y + height - mainRect.height();
       y = y - tmpHeight;
       if(y <= 0){
            y = 0;
            isOver = true;
            qDebug()<<" ====== y <= 0 === ";
        }
    }

    rect.moveTo(x,y);
    rect.setWidth(width);
    rect.setHeight(height);
    return isOver;
}