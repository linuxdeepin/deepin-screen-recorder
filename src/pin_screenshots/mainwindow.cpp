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
    //初始化主窗口
    initMainWindow();
    //计算屏幕大小
    CalculateScreenSize();
    this->setMinimumSize(WHEELNUM, WHEELNUM);
    initShortcut(); // 初始化快捷键
}

MainWindow::~MainWindow()
{

}

void MainWindow::initMainWindow()
{
    m_ocrInterface = nullptr;
    //去菜单栏，置顶窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
//    setAttribute(Qt::WA_TranslucentBackground, true); //设置透明
    //设置可以进行鼠标操作
    setMouseTracking(true);
    isLeftPressDown = false;
    dir = UP;
    //获取屏幕的锁房比例
    m_pixelRatio = qApp->primaryScreen()->devicePixelRatio();

    m_menuController = new MenuController();
    connect(m_menuController, &MenuController::saveAction, this, &MainWindow::onSave);
    connect(m_menuController, &MenuController::closeAction, this, &MainWindow::onExit);
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
    proportion = static_cast<double>(this->width())  / this->height();
    return true;
}

bool MainWindow::openImageAndName(const QImage &image, const QString &name, const QPoint &point)
{
    qDebug() << "func: " << __func__ ;
    m_image = image;
    if (name.isEmpty()) {
        m_imageName = QString("test.png");
    } else {
        m_imageName = name;
    }
    int width = static_cast<int>(m_image.width() / m_pixelRatio);
    int height = static_cast<int>(m_image.height() / m_pixelRatio);
    //将窗口的大小重置为图片的大小
    resize(width, height);
    //将坐标点转换为m_pixelRatio屏幕缩放比下的点
    QPoint temp(static_cast<int>(point.x() * m_pixelRatio), static_cast<int>(point.y() * m_pixelRatio));
    QRect currnetScreenRect;
    //查找当前截图贴图区域所在的屏幕
    for (int i = 0; i < m_screenInfo.size(); ++i) {
        currnetScreenRect = {
            static_cast<int>(m_screenInfo[i].x),
            static_cast<int>(m_screenInfo[i].y),
            static_cast<int>(m_screenInfo[i].width),
            static_cast<int>(m_screenInfo[i].height)
        };
        if (currnetScreenRect.contains(temp)) {
            int x = static_cast<int>((temp.x() - currnetScreenRect.x()) / m_pixelRatio + currnetScreenRect.x());
            temp.setX(x);
            int y = static_cast<int>((temp.y() - currnetScreenRect.y()) / m_pixelRatio + currnetScreenRect.y());
            temp.setY(y);
            break;
        }
    }
    //移动到指定位置
    move(temp);
    update();
    proportion = static_cast<double>(this->width())  / this->height();
    return true;
}
// 开启OCR
void MainWindow::openOCR()
{
    // 测试
    if (m_ocrInterface == nullptr)
        m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), this);
    m_ocrInterface->openImageAndName(m_image, m_imageName);
}

// 贴图保存
void MainWindow::saveImg()
{
    // 测试
    QString savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    m_image.save(QString("%1/%2.png").arg(savePath).arg(m_imageName));
    this->close();
}

void MainWindow::onSave()
{
    qDebug() << "func: " << __func__ ;
    saveImg();
}
void MainWindow::onExit()
{
    qDebug() << "func: " << __func__ ;
    this->close();
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
        handleMouseRightBtn(event);
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
            QPoint newPoint;
            switch (dir) {
            case LEFTTOP:
                newPoint = QPointF(br.x() - gloPoint.x(), static_cast<int>((br.x() - gloPoint.x()) / proportion)).toPoint();
                rMove.setX(br.x() - newPoint.x());
                rMove.setY(br.y() - newPoint.y());
                if (rMove.x() +  WHEELNUM >= br.x() || rMove.y() + WHEELNUM >= br.y()) {
                    rMove.setX(br.x() - WHEELNUM);
                    rMove.setY(br.y() - WHEELNUM);
                }
                if (br.y() - rMove.height() <= 0) {
                    int heigt = br.y();
                    int width = static_cast<int>(heigt * proportion);
                    rMove.setX(br.x() - width);
                    rMove.setY(br.y() - heigt);
                }
                break;
            case RIGHTTOP:
                newPoint = QPointF(tl.x() + static_cast<int>((bl.y() - gloPoint.y()) * proportion), gloPoint.y()).toPoint();
                if (newPoint.x() -  WHEELNUM <= bl.x() || newPoint.y() + WHEELNUM >= bl.y()) {
                    rMove.setTopRight(QPoint(bl.x() + WHEELNUM, bl.y() - WHEELNUM));
                } else {
                    rMove.setTopRight(newPoint);
                }
                break;
            case LEFTBOTTOM:
                newPoint = QPointF(gloPoint.x(), static_cast<int>(tr.y() + ((tr.x() - gloPoint.x())) / proportion)).toPoint();
                if (newPoint.x() + WHEELNUM >= tr.x() || newPoint.y() - WHEELNUM <= tr.y()) {
                    rMove.setBottomLeft(QPoint(tr.x() - WHEELNUM, tr.y() + WHEELNUM));
                } else {
                    if (newPoint.y() >= m_screenSize.height()) {
                        int height = m_screenSize.height() - tr.y();
                        int width = static_cast<int>(height * proportion);
                        newPoint.setY(m_screenSize.height());
                        newPoint.setX(tr.x() - width);
                    }
                    rMove.setBottomLeft(newPoint);

                }
                break;
            case RIGHTBOTTOM:
                newPoint = QPointF(gloPoint.x() - tl.x(), static_cast<int>((gloPoint.x() - tl.x()) / proportion)).toPoint();
                rMove.setWidth(gloPoint.x() - tl.x());
                rMove.setHeight(newPoint.y());
                if (rMove.y() + rMove.height() >= m_screenSize.height()) {
                    int heigt = m_screenSize.height() - rMove.y();
                    int width = static_cast<int>(heigt * proportion);
                    rMove.setWidth(width);
                    rMove.setHeight(heigt);
                }
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
    if (event->key() == Qt::Key_Left) {
        this->move(x - MOVENUM, y);
    } else if (event->key() == Qt::Key_Right) {
        this->move(x + MOVENUM, y);
    } else if (event->key() == Qt::Key_Up) {
        this->move(x, y - MOVENUM);
    } else if (event->key() == Qt::Key_Down) {
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
    if (event->delta() < 0 && (width < this->minimumWidth() || height < this->minimumHeight()))
        return;
    if (event->delta() > 0) {
        QPoint newPoint = QPointF(width + WHEELNUM, static_cast<int>((width + WHEELNUM) / proportion)).toPoint();
        rect.setWidth(newPoint.x());
        rect.setHeight(newPoint.y());
    } else {
        QPoint newPoint = QPointF(width - WHEELNUM, static_cast<int>((width - WHEELNUM) / proportion)).toPoint();
        rect.setWidth(newPoint.x());
        rect.setHeight(newPoint.y());
    }
    QPointF globalPos = event->globalPos(); // 鼠标在屏幕中的坐标
    //qDebug()<<"globalPos"<<globalPos;
    QPointF newPoint(rect.width() * unionPoint.x(), rect.height() * unionPoint.y()); //扩大后，鼠标在贴图中的坐标
    QPointF topLeft(globalPos.x() - newPoint.x(), globalPos.y() - newPoint.y()); // 贴图左上角的点在屏幕中的坐标
    rect = QRect(topLeft.toPoint(), rect.size());

    //qDebug()<<"=====rect : "<<rect;
    if (boundaryCalculation(rect))
        return;
    this->setGeometry(rect.x(), rect.y(), rect.width(), rect.height());
}

// 初始化快捷键
void MainWindow::initShortcut()
{
    // Esc —— 退出
    QShortcut *Keyesc = new QShortcut(QKeySequence("Escape"), this);
    connect(Keyesc, &QShortcut::activated, this, [ = ] {
        qDebug() << "shortcut : Keyesc (key: esc)";
        this->close();
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
    });
}
// 贴图窗口边界计算
bool MainWindow::boundaryCalculation(QRect &rect)
{
    //QRect mainRect = QApplication::desktop()->screen()->geometry();
    //qDebug()<<"mainRect"<<mainRect;
    int width = rect.width();
    int height = rect.height();
    int x = rect.x();
    int y = rect.y();
    //qDebug()<<"2 rect === "<<rect<<"width"<<width<<"height"<<height;
    bool isOver = false;
    if ((x + width >= m_screenSize.width())) {
        int tmpWidth = x + width - m_screenSize.width();
        x = x - tmpWidth;
        //qDebug()<<" ====== x > 0 === "<<x<<y;
        if (x <= 0) {
            x = 0;
            isOver = true;
        }
    }

    if ((y + height >= m_screenSize.height())) {
        int tmpHeight = y + height - m_screenSize.height();
        y = y - tmpHeight;
        if (y <= 0) {
            y = 0;
            isOver = true;
            //qDebug()<<" ====== y <= 0 === ";
        }
    }

    rect.moveTo(x, y);
    rect.setWidth(width);
    rect.setHeight(height);
    return isOver;
}
// 计算屏幕大小
void MainWindow::CalculateScreenSize()
{
    //qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QList<QScreen *> screenList = qApp->screens();
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        qDebug() << (*it)->name() << rect;
        ScreenInfo screenInfo;
        screenInfo.x = rect.x();
        screenInfo.y = rect.y();
        screenInfo.height =  static_cast<int>(rect.height());
        screenInfo.width = static_cast<int>(rect.width());
        screenInfo.name = (*it)->name();
        m_screenInfo.append(screenInfo);
    }

    m_screenSize.setWidth(m_screenInfo[0].x + m_screenInfo[0].width);
    m_screenSize.setHeight(m_screenInfo[0].y + m_screenInfo[0].height);

    // 通过每个屏幕， 右下角的坐标来计算屏幕总大小。
    for (int i = 1; i < m_screenInfo.size(); ++i) {
        if ((m_screenInfo[i].height + m_screenInfo[i].y) > m_screenSize.height())
            m_screenSize.setHeight(m_screenInfo[i].height + m_screenInfo[i].y);

        if ((m_screenInfo[i].width + m_screenInfo[i].x) > m_screenSize.width())
            m_screenSize.setWidth(m_screenInfo[i].width + m_screenInfo[i].x);
    }

    qDebug() << m_screenSize;
    if (m_screenInfo.size() > 1) {
        // 排序
        qSort(m_screenInfo.begin(), m_screenInfo.end(), [ = ](const ScreenInfo info1, const ScreenInfo info2) {
            return info1.x < info2.x;
        });
    }
}

//处理鼠标右键
void MainWindow::handleMouseRightBtn(QMouseEvent *mouseEvent)
{
    qDebug() << "func: " << __func__ ;
    if (m_menuController != nullptr) {
        m_menuController->showMenu(QPoint(mapToGlobal(mouseEvent->pos())));
    } else {
        qDebug() << "the m_menuController is nullptr!!!";
    }
}
