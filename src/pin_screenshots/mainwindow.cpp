// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "putils.h"
#include "settings.h"
#include "../utils/log.h"

#include <QDebug>
#include <QDBusInterface>
#include <QScreen>
#include <QGuiApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

#define MOVENUM 1

#define WHEELNUM 10
namespace {
const int TOOLBAR_Y_SPACING = 12;
}

MainWindow::MainWindow(QWidget *parent)
    : DWidget(parent)
{
    qCDebug(dsrApp) << "MainWindow constructor called.";
    //初始化主窗口
    initMainWindow();
    qCDebug(dsrApp) << "initMainWindow finished.";
    //计算屏幕大小
    CalculateScreenSize();
    qCDebug(dsrApp) << "CalculateScreenSize finished.";
    this->setMinimumSize(WHEELNUM, WHEELNUM);
    qCDebug(dsrApp) << "Set minimum size of window.";
    initShortcut(); // 初始化快捷键
    qCDebug(dsrApp) << "initShortcut finished.";
}

MainWindow::~MainWindow()
{
    qCDebug(dsrApp) << "MainWindow destructor called.";
    if (m_toolBar != nullptr) {
        qCDebug(dsrApp) << "Deleting m_toolBar.";
        delete m_toolBar;
        m_toolBar = nullptr;
    }
}

void MainWindow::initMainWindow()
{
    qCDebug(dsrApp) << "Initializing main window components.";
    m_ocrInterface = nullptr;
    m_toolBar = nullptr;
    //去菜单栏，置顶窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);
    qCDebug(dsrApp) << "Set window flags.";

    //设置可以进行鼠标操作
    setMouseTracking(true);
    qCDebug(dsrApp) << "Mouse tracking enabled.";
    isLeftPressDown = false;
    dir = UP;
    //获取屏幕的缩放比例
    m_pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    qCDebug(dsrApp) << "Pixel ratio:" << m_pixelRatio;

    m_menuController = new MenuController();
    connect(m_menuController, &MenuController::saveAction, this, &MainWindow::onSave);
    connect(m_menuController, &MenuController::closeAction, this, &MainWindow::onExit);

    // 工具栏设置
    m_toolBar = new ToolBarWidget(this); //初始化
    connect(m_toolBar, SIGNAL(sendOcrButtonClicked()), this, SLOT(onOpenOCR()));
    connect(m_toolBar, SIGNAL(sendSaveButtonClicked()), this, SLOT(onSave()));
    connect(m_toolBar, SIGNAL(sendCloseButtonClicked()), this, SLOT(onExit()));

    m_toolBar->setFocusPolicy(Qt::StrongFocus);
    qCDebug(dsrApp) << "ToolBar focus policy set.";

    QWidget::installEventFilter(this);
    qCDebug(dsrApp) << "Event filter installed.";
}

bool MainWindow::openFile(const QString &filePaths)
{
    qCDebug(dsrApp) << "Opening file:" << filePaths;
    QImage image(filePaths);
    openImageAndName(image, "", QPoint(0, 0));
    qCDebug(dsrApp) << "openImageAndName called after opening file.";
    return true;
}

bool MainWindow::openImage(const QImage &image)
{
    qCDebug(dsrApp) << "Opening image (QImage).";
    openImageAndName(image, "", QPoint(0, 0));
    qCDebug(dsrApp) << "openImageAndName called after opening QImage.";
    return true;
}

bool MainWindow::openImageAndName(const QImage &image, const QString &name, const QPoint &point)
{
    qCDebug(dsrApp) << "Opening image with name and point. Name:" << name << ", Point:" << point;
    m_image = image;
    m_lastImagePath = name;
    m_imageName = QString(tr("Screenshot") + "_%1").arg(QDateTime::currentDateTime().
                                                        toString("yyyyMMddHHmmss"));
    int width = static_cast<int>(m_image.width() / m_pixelRatio);
    int height = static_cast<int>(m_image.height() / m_pixelRatio);
    qCDebug(dsrApp) << "Image dimensions (scaled):" << width << "x" << height;
    //将窗口的大小重置为图片的大小
    resize(width, height);
    qCDebug(dsrApp) << "Window resized to image dimensions.";
    //将坐标点转换为m_pixelRatio屏幕缩放比下的点
    m_showPosition = QPoint(static_cast<int>(point.x() * m_pixelRatio), static_cast<int>(point.y() * m_pixelRatio));
    qCDebug(dsrApp) << "Calculated show position:" << m_showPosition;
    QRect currnetScreenRect;
    //查找当前截图贴图区域所在的屏幕(m_screenInfo中存储已缩放的数据）
    for (int i = 0; i < m_screenInfo.size(); ++i) {
        qCDebug(dsrApp) << "Checking screen info for screen:" << m_screenInfo[i].name;
        currnetScreenRect = {
            static_cast<int>(m_screenInfo[i].x),
            static_cast<int>(m_screenInfo[i].y),
            static_cast<int>(m_screenInfo[i].width *  m_pixelRatio),
            static_cast<int>(m_screenInfo[i].height * m_pixelRatio)
        };
        if (currnetScreenRect.contains(m_showPosition)) {
            qCDebug(dsrApp) << "Current screen rect contains show position.";
            int x = static_cast<int>((m_showPosition.x() - currnetScreenRect.x()) / m_pixelRatio + currnetScreenRect.x());
            m_showPosition.setX(x);
            int y = static_cast<int>((m_showPosition.y() - currnetScreenRect.y()) / m_pixelRatio + currnetScreenRect.y());
            m_showPosition.setY(y);
            qCDebug(dsrApp) << "Adjusted show position based on screen:" << m_showPosition;
            break;
        }
    }
    //移动到指定位置
    move(m_showPosition);
    qCDebug(dsrApp) << "Window moved to show position.";
    update();
    proportion = static_cast<double>(this->width())  / this->height();
    qCDebug(dsrApp) << "Calculated proportion:" << proportion;
    m_isfirstTime = true; //解决119157Bug采用的标识
    updateToolBarPosition();// 更新位置
    qCDebug(dsrApp) << "Tool bar position updated.";
    return true;
}
// 开启OCR
void MainWindow::onOpenOCR()
{
    qCDebug(dsrApp) << "onOpenOCR called.";
    saveImg();
    qCDebug(dsrApp) << "Image saved before OCR.";
    // 测试
    if (m_ocrInterface == nullptr) {
        m_ocrInterface = new OcrInterface("com.deepin.Ocr", "/com/deepin/Ocr", QDBusConnection::sessionBus(), this);
        qCDebug(dsrApp) << "OCR interface created.";
    }
    m_ocrInterface->openImageAndName(m_image, m_imageName);
    qCDebug(dsrApp) << "Opened image with OCR interface.";
}

// 贴图保存
void MainWindow::saveImg()
{
    qCDebug(dsrApp) << "saveImg called.";
    m_saveInfo = m_toolBar->getSaveInfo(); // 获取保存信息
    qCDebug(dsrApp) << "Save info retrieved. Type:" << m_saveInfo.first << ", Format:" << m_saveInfo.second;

    if (m_saveInfo.first == SubToolWidget::DESKTOP) {
        qCDebug(dsrApp) << "保存到桌面";
        QString savePath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
        QString formatStr;
        if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::PNG)) {
            formatStr = QString("png");
            qCDebug(dsrApp) << "Save format set to PNG.";
        } else if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::JPG)) {
            formatStr = QString("jpg");
            qCDebug(dsrApp) << "Save format set to JPG.";
        } else if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::BMP)) {
            formatStr = QString("bmp");
            qCDebug(dsrApp) << "Save format set to BMP.";
        }
        m_lastImagePath = QString("%1/%2.%3").arg(savePath).arg(m_imageName).arg(formatStr);
        qCDebug(dsrApp) << "Image will be saved to:" << m_lastImagePath;
    } else if (m_saveInfo.first == SubToolWidget::PICTURES) {
        qCDebug(dsrApp) << "保存到图片";
        QString savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first() + QDir::separator() + "Screenshots";
        if ((!QDir(savePath).exists() && QDir().mkdir(savePath) == false) ||  // 文件不存在，且创建失败
                (QDir(savePath).exists() && !QFileInfo(savePath).isWritable())) {  // 文件存在，且不能写
            qCDebug(dsrApp) << "Screenshots directory does not exist or is not writable, using default Pictures location.";
            savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
        }
        QString formatStr;
        if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::PNG)) {
            formatStr = QString("png");
            qCDebug(dsrApp) << "Save format set to PNG.";
        } else if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::JPG)) {
            formatStr = QString("jpg");
            qCDebug(dsrApp) << "Save format set to JPG.";
        } else if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::BMP)) {
            formatStr = QString("bmp");
            qCDebug(dsrApp) << "Save format set to BMP.";
        }
        m_lastImagePath = QString("%1/%2.%3").arg(savePath).arg(m_imageName).arg(formatStr);
        qCDebug(dsrApp) << "Image will be saved to:" << m_lastImagePath;
    } else if (m_saveInfo.first == SubToolWidget::FOLDER_CHANGE) {
        m_toolBar->hide();
        this->hide();
        qCDebug(dsrApp) << "保存到指定位置";
        QString saveFileName;
        QString imgName = Settings::instance()->getSavePath();
        if (!imgName.isEmpty()) {
            qCDebug(dsrApp) << "Custom save path is not empty.";
            imgName += "/";
        }
        if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::PNG)) {
            imgName += QString("%1.png").arg(m_imageName);
            saveFileName = QFileDialog::getSaveFileName(this, tr("Save"),  imgName,
                                                        tr("PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)"));
            qCDebug(dsrApp) << "PNG save dialog opened.";
        } else if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::JPG)) {
            imgName += QString("%1.jpg").arg(m_imageName);
            saveFileName = QFileDialog::getSaveFileName(this, tr("Save"),  imgName,
                                                        tr("JPEG (*.jpg *.jpeg);;PNG (*.png);;BMP (*.bmp)"));
            qCDebug(dsrApp) << "JPG save dialog opened.";
        } else if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::BMP)) {
            imgName += QString("%1.bmp").arg(m_imageName);
            saveFileName = QFileDialog::getSaveFileName(this, tr("Save"),  imgName,
                                                        tr("BMP (*.bmp);;JPEG (*.jpg *.jpeg);;PNG (*.png)"));
            qCDebug(dsrApp) << "BMP save dialog opened.";
        }
        if (saveFileName.isEmpty()) {
            qCDebug(dsrApp) << "User cancelled save dialog.";
            qWarning() << "指定路径为空！";
            onExit();
            return;
        }
        qCDebug(dsrApp) << "saveFileName" << saveFileName;
        m_lastImagePath = saveFileName;
        Settings::instance()->setSavePath(QFileInfo(saveFileName).path());
        qCDebug(dsrApp) << "Save path updated in settings.";
    } else if (m_saveInfo.first == SubToolWidget::FOLDER) {
        QString savePath = Settings::instance()->getSavePath();
        qCDebug(dsrApp) << "保存到历史路径" << savePath;
        QString formatStr;
        if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::PNG)) {
            formatStr = QString("png");
            qCDebug(dsrApp) << "Save format set to PNG.";
        } else if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::JPG)) {
            formatStr = QString("jpg");
            qCDebug(dsrApp) << "Save format set to JPG.";
        } else if (m_saveInfo.second == static_cast<SubToolWidget::SAVEFORMAT>(SubToolWidget::BMP)) {
            formatStr = QString("bmp");
            qCDebug(dsrApp) << "Save format set to BMP.";
        }
        m_lastImagePath = QString("%1/%2.%3").arg(savePath).arg(m_imageName).arg(formatStr);
    } else if (m_saveInfo.first == SubToolWidget::CLIPBOARD) {
        qCDebug(dsrApp) << "保存到剪切板";
        m_lastImagePath = "";
    }
    //每次保存时重设isChangeSavePath
    Settings::instance()->setIsChangeSavePath(false);

    bool isSaveState = true;
    if (m_saveInfo.first != SubToolWidget::CLIPBOARD) {
        isSaveState = m_image.save(m_lastImagePath);
    }

    // 保存到剪贴板
    QMimeData *t_imageData = new QMimeData;
    // 图片数据过大时，可能影响后端剪贴板处理，调整为保存 PNG 图片
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    m_image.save(&buffer, "PNG");
    t_imageData->setData("image/png", bytes);
    QClipboard *cb = qApp->clipboard();
    cb->setMimeData(t_imageData, QClipboard::Clipboard);

    if (PUtils::isWaylandMode) {
        QEventLoop eventloop;
        connect(cb, SIGNAL(dataChanged()), &eventloop, SLOT(quit()));
        eventloop.exec();
    }

    // 发送通知
    sendNotify(m_lastImagePath, isSaveState);
}
// 获取贴图窗口的显示位置
QPoint MainWindow::getShowPosition()
{
    qCDebug(dsrApp) << "获取贴图窗口的显示位置";
    return m_showPosition;
}

// 保存
void MainWindow::onSave()
{
    qCDebug(dsrApp) << "onSave called, calling saveImg.";
    saveImg();
}
// 退出
void MainWindow::onExit()
{
    //每次关闭贴图时重设isChangeSavePath
    Settings::instance()->setIsChangeSavePath(false);
    qCDebug(dsrApp) << "Exiting application.";
    m_toolBar->close();
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
    //qCDebug(dsrApp) << this << __FUNCTION__ << __LINE__ ;
    switch (event->button()) {
    case Qt::LeftButton:
        isLeftPressDown = true;
        if (dir != NONE) {
            this->mouseGrabber();
        } else {
            dragPosition = (event->globalPosition() - this->frameGeometry().topLeft()).toPoint();
        }
        if (!m_toolBar->isHidden())
            m_toolBar->hide(); //隐藏工具栏
        //qCDebug(dsrApp) << this << __FUNCTION__ << __LINE__ ;
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
    QPoint gloPoint = event->globalPosition().toPoint();
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint br = mapToGlobal(rect.bottomRight());
    QPoint bl = mapToGlobal(rect.bottomLeft());
    QPoint tr = mapToGlobal(rect.topRight());

    if (!isLeftPressDown) {
        this->region(gloPoint);
    } else {
        // 确保在拖动过程中隐藏工具栏
        if (!m_toolBar->isHidden()) {
            m_toolBar->hide();
        }
        
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
            //qCDebug(dsrApp) << "=============event->globalPos()" << event->globalPos() << "dragPosition" << dragPosition;
            QPoint globalPoint = (event->globalPosition() - QPointF(dragPosition)).toPoint();
            if (PUtils::isWaylandMode && globalPoint.y() < 0) {
                globalPoint.setY(0);
            }
            move(globalPoint);
            event->accept();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
//    qCDebug(dsrApp) << this << __FUNCTION__ << __LINE__ ;
    if (event->button() == Qt::LeftButton) {
        isLeftPressDown = false;
        if (dir != NONE) {
            this->releaseMouse();
        }
        this->setCursor(QCursor(Qt::ArrowCursor));
        
        // 在拖拽结束后显示工具栏
        // 短暂延迟确保所有状态都已更新
        QTimer::singleShot(50, this, [this]() {
            // 检查鼠标是否仍在窗口内
            QPoint globalPos = QCursor::pos();
            QPoint localPos = this->mapFromGlobal(globalPos);
            if (this->rect().contains(localPos)) {
                updateToolBarPosition();
                qCDebug(dsrApp) << "Showing toolbar after drag operation";
            }
        });
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // 如果不是由于鼠标左键按下引起的大小变化，则不处理
    if (!isLeftPressDown) {
        return;
    }
    
    // 在窗口大小调整结束后短暂延迟显示工具栏
    // 使用一次性定时器，避免多次触发
    static QTimer *resizeTimer = nullptr;
    if (resizeTimer) {
        resizeTimer->stop();
        delete resizeTimer;
    }
    
    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true);
    connect(resizeTimer, &QTimer::timeout, this, [this]() {
        // 检查鼠标是否在窗口内
        QPoint globalPos = QCursor::pos();
        QPoint localPos = this->mapFromGlobal(globalPos);
        if (this->rect().contains(localPos)) {
            updateToolBarPosition();
            qCDebug(dsrApp) << "Showing toolbar after resize operation";
        }
    });
    resizeTimer->start(100);
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (!m_image.isNull()) {
        QPainter pp(this);
        QRect Temp(0, 0, this->width(), this->height());
        // 用rect取到的图片不容易失帧
        //QImage tempImage = m_image.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        pp.drawImage(Temp, m_image);
    }


}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int x = this->pos().x();
    int y = this->pos().y();
    bool isNeedUpdateToolBar = false;
    if (event->key() == Qt::Key_Left) {
        this->move(x - MOVENUM, y);
        isNeedUpdateToolBar = true;
    } else if (event->key() == Qt::Key_Right) {
        this->move(x + MOVENUM, y);
        isNeedUpdateToolBar = true;
    } else if (event->key() == Qt::Key_Up) {
        // 适配wayland
        if (y - MOVENUM < 0) {
            this->move(x, 0);
        } else {
            this->move(x, y - MOVENUM);
        }

        isNeedUpdateToolBar = true;
    } else if (event->key() == Qt::Key_Down) {
        this->move(x, y + MOVENUM);
        isNeedUpdateToolBar = true;
    }

    if (isNeedUpdateToolBar) {
        updateToolBarPosition();
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    QRect rect;
    int width = this->rect().width();
    int height = this->rect().height();
    QPointF unionPoint(event->position().x() / width, event->position().y() / height);
    //qCDebug(dsrApp) << unionPoint;
    if (event->angleDelta().y() < 0 && (width < this->minimumWidth() || height < this->minimumHeight()))
        return;
    if (event->angleDelta().y() > 0) {
        QPoint newPoint = QPointF(width + WHEELNUM, static_cast<int>((width + WHEELNUM) / proportion)).toPoint();
        rect.setWidth(newPoint.x());
        rect.setHeight(newPoint.y());
    } else {
        QPoint newPoint = QPointF(width - WHEELNUM, static_cast<int>((width - WHEELNUM) / proportion)).toPoint();
        rect.setWidth(newPoint.x());
        rect.setHeight(newPoint.y());
    }
    QPointF globalPos = event->globalPosition(); // 鼠标在屏幕中的坐标
    //qCDebug(dsrApp)<<"globalPos"<<globalPos;
    QPointF newPoint(rect.width() * unionPoint.x(), rect.height() * unionPoint.y()); //扩大后，鼠标在贴图中的坐标
    QPointF topLeft(globalPos.x() - newPoint.x(), globalPos.y() - newPoint.y()); // 贴图左上角的点在屏幕中的坐标
    rect = QRect(topLeft.toPoint(), rect.size());

    this->setGeometry(rect.x(), rect.y(), rect.width(), rect.height());
    updateToolBarPosition(); //更新位置
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        //QEvent::WindowActivate:窗口被激活;QEvent::WindowDeactivate:窗口已停用
        if (QEvent::WindowActivate == event->type()) {
            //qCDebug(dsrApp) <<this<< m_toolBar << __FUNCTION__ << __LINE__ << event->type();
            //规避wayland下打开贴图1，并在贴图1上重新生成贴图2时，贴图2激活状态不对(工具栏已经设置显示，但实际上未显示。)。
            if (PUtils::isWaylandMode && !isLeftPressDown) {
                this->m_toolBar->hide();
            }
            updateToolBarPosition();
            //规避wayland下打开多个贴图，通过鼠标连续切换贴图时，贴图激活状态不对(工具栏已经设置显示，但实际上未显示。)。
            if (PUtils::isWaylandMode && isLeftPressDown) {
                this->m_toolBar->hide();
            }
            //qCDebug(dsrApp) <<this<< m_toolBar << __FUNCTION__ << __LINE__ << event->type();
            return false;
        } else if (QEvent::WindowDeactivate == event->type()) {
            //qCDebug(dsrApp) << this << m_toolBar <<__FUNCTION__ << __LINE__ << event->type();
            // 检查鼠标是否在工具栏上，如果是则不隐藏工具栏
            QPoint globalPos = QCursor::pos();
            if (m_toolBar->isActiveWindow() || m_toolBar->geometry().contains(globalPos))
                return false;
            m_toolBar->hide();
            //qCDebug(dsrApp) << this << m_toolBar <<__FUNCTION__ << __LINE__ << event->type();
            return false;
        }
    }
    return false;
}

// 初始化快捷键
void MainWindow::initShortcut()
{
    // Esc —— 退出
    QShortcut *Keyesc = new QShortcut(QKeySequence("Escape"), this);
    connect(Keyesc, &QShortcut::activated, this, [ = ] {
        qCDebug(dsrApp) << "shortcut : Keyesc (key: esc)";
        onExit();
    });

    // Ctrl+S —— 保存
    QShortcut *Keysave = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(Keysave, &QShortcut::activated, this, [ = ] {
        qCDebug(dsrApp) << "shortcut : Keysave (key: ctrl+s)";
        onSave();
    });

    // Alt+O —— 文字识别
    QShortcut *Keyocr = new QShortcut(QKeySequence("Alt+O"), this);
    connect(Keyocr, &QShortcut::activated, this, [ = ] {
        qCDebug(dsrApp) << "shortcut : Keyocr (key: alt+o)";
        onOpenOCR();
    });


    // F1 —— 显示帮助 测试
    QShortcut *Keyhelp = new QShortcut(QKeySequence("F1"), this);
    connect(Keyhelp, &QShortcut::activated, this, [ = ] {
        qCDebug(dsrApp) << "shortcut : Keyhelp (key: F1)";
        QDBusInterface interFace("com.deepin.Manual.Open",
                                 "/com/deepin/Manual/Open",
                                 "com.deepin.Manual.Open",
                                 QDBusConnection::sessionBus());
        // 帮助手册跳转到子标题，传入子标题标签(标签文档内唯一，固定为英文，与翻译无关)
        QList<QVariant> arg;
        arg << (QString("deepin-screen-recorder"))                  // 应用名称
            << QString("pinscreenshots");                         // 帮助文案中的标题名称
        interFace.callWithArgumentList(QDBus::AutoDetect, "OpenTitle", arg);
    });

    QShortcut *KeyF3 = new QShortcut(QKeySequence("F3"), this);
    connect(KeyF3, &QShortcut::activated, this, [ = ] {
        qCDebug(dsrApp) << "shortcut : Keyocr (key: F3)";
        m_toolBar->onOptionButtonClicked();
    });
}

// 计算屏幕大小
void MainWindow::CalculateScreenSize()
{
    //qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QList<QScreen *> screenList = qApp->screens();
    for (auto it = screenList.constBegin(); it != screenList.constEnd(); ++it) {
        QRect rect = (*it)->geometry();
        qCDebug(dsrApp) << (*it)->name() << rect;
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

    qCDebug(dsrApp) << m_screenSize;
    if (m_screenInfo.size() > 1) {
        // 排序
        std::sort(m_screenInfo.begin(), m_screenInfo.end(), [](const ScreenInfo& info1, const ScreenInfo& info2) {
            return info1.x < info2.x;
        });
    }
}

//处理鼠标右键
void MainWindow::handleMouseRightBtn(QMouseEvent *mouseEvent)
{
    qCDebug(dsrApp) << "func: " << __func__ ;
    if (m_menuController != nullptr) {
        m_menuController->showMenu(QPoint(mapToGlobal(mouseEvent->pos())));
    } else {
        qCDebug(dsrApp) << "the m_menuController is nullptr!!!";
    }
}

// 发送通知
void MainWindow::sendNotify(QString savePath, bool bSaveState)
{
    QDBusInterface notification("org.freedesktop.Notifications",
                                "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications",
                                QDBusConnection::sessionBus());

    if (!bSaveState) {
        qCDebug(dsrApp) << __FUNCTION__;
        QString tips = QString(tr("Save failed. Please save it in your home directory."));
        Settings::instance()->setSavePath("");
        QList<QVariant> arg;
        unsigned int id = 0;
        int timeout = -1;
        arg << tr("Pin Screenshots") << id << "deepin-screen-recorder" << QString() << tips << QStringList() << QVariantMap() << timeout;
        notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
        onExit();
        return;
    }

    QStringList actions;
    QVariantMap hints;
    QString body;
    // 如果保存路径为剪切板，不做打开view操作
    if (!savePath.isEmpty()) {
        actions << "_open" << tr("View");
        if (!QStandardPaths::findExecutable("dde-file-manager").isEmpty()) {
            qCDebug(dsrApp) << QFileInfo(savePath).path();
            hints["x-deepin-action-_open"] = QStringList{"dde-file-manager", "--show-item", savePath};
        } else {
            hints["x-deepin-action-_open"] = QStringList{"xdg-open", savePath};
        }
        body = QString(tr("Saved to %1")).arg(savePath);
    }
    int timeout = -1;
    unsigned int id = 0;
    QList<QVariant> arg;
    arg << tr("Pin Screenshots")                 // appname
        << id                                                    // id
        << QString("deepin-screen-recorder")                     // icon
        << tr("Screenshot finished")                              // summary
        << body              // body
        << actions                                               // actions
        << hints                                                 // hints
        << timeout;                                              // timeout
    notification.callWithArgumentList(QDBus::AutoDetect, "Notify", arg);
    onExit();
}

// 更新工具栏显示位置
void MainWindow::updateToolBarPosition()
{
    //获取贴图界面右下角的点
    QPoint brPoint = mapToGlobal(this->rect().bottomRight());
    int x = 0, y = 0;
//    qCDebug(dsrApp) << "updateToolBarPosition brPoint is" << brPoint;
    //判断工具栏放贴图界面下方时是否会超出屏幕下方
    if ((brPoint.y() + TOOLBAR_Y_SPACING + m_toolBar->height()) >= m_screenSize.height()) {
        //此时工具栏放贴图界面下方会超出屏幕，只能放贴图界面上方
        //获取贴图界面右上角的点
        QPoint trPoint = mapToGlobal(this->rect().topRight());
        //判断工具栏是否超出屏幕上方
        if (trPoint.y() - TOOLBAR_Y_SPACING - m_toolBar->height() <= 0) {
            //工具栏放贴图界面上下方都会超出屏幕，此时就工具栏放屏幕上方
            x = trPoint.x() - m_toolBar->width();
            y = 0;
        } else {
            x = trPoint.x() - m_toolBar->width();
            y = trPoint.y() - TOOLBAR_Y_SPACING - m_toolBar->height();
        }
    } else {
        //此时工具栏放贴图界面下方不会超出屏幕可以放界面下方
        x = brPoint.x() - m_toolBar->width();
        y = brPoint.y() + TOOLBAR_Y_SPACING;
        //qCDebug(dsrApp) << "m_toolBar->width()" << m_toolBar->toolBarWidth();
    }
    if (x < 0) {
        x = 0;
    } else if (x + m_toolBar->width() > m_screenSize.width()) {
        x = m_screenSize.width() - m_toolBar->width();
    }
    m_toolBar->showAt(QPoint(x, y), m_isfirstTime);
    m_toolBar->activateWindow();
    m_isfirstTime = false;
}

void MainWindow::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    if (!isLeftPressDown && !m_toolBar->isVisible()) {
        updateToolBarPosition();
    }
    QWidget::enterEvent(event);
}

void MainWindow::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    
    // 获取当前鼠标全局位置
    QPoint globalPos = QCursor::pos();
    
    // 获取工具栏几何信息
    QRect toolBarGeom = m_toolBar->geometry();
    
    // 获取主窗口底部边缘位置
    QPoint brPoint = mapToGlobal(this->rect().bottomRight());
    QPoint blPoint = mapToGlobal(this->rect().bottomLeft());
    
    // 创建一个连接主窗口和工具栏的区域
    QRect connectionArea;
    
    // 如果工具栏在窗口下方
    if (toolBarGeom.top() > brPoint.y()) {
        connectionArea = QRect(
            QPoint(qMin(blPoint.x(), toolBarGeom.left()), brPoint.y()),
            QPoint(qMax(brPoint.x(), toolBarGeom.right()), toolBarGeom.top())
        );
    }
    // 如果工具栏在窗口上方
    else if (toolBarGeom.bottom() < mapToGlobal(this->rect().topLeft()).y()) {
        QPoint tlPoint = mapToGlobal(this->rect().topLeft());
        QPoint trPoint = mapToGlobal(this->rect().topRight());
        connectionArea = QRect(
            QPoint(qMin(tlPoint.x(), toolBarGeom.left()), toolBarGeom.bottom()),
            QPoint(qMax(trPoint.x(), toolBarGeom.right()), tlPoint.y())
        );
    }
    
    // 只有当鼠标既不在工具栏上，也不在连接区域内时才隐藏工具栏
    // 但是如果鼠标在工具栏上，让工具栏自己的leaveEvent处理
    if (!toolBarGeom.contains(globalPos) && !connectionArea.contains(globalPos)) {
        // 使用短暂延迟，避免在快速移动时错误隐藏
        QTimer::singleShot(100, this, [this, toolBarGeom]() {
            QPoint currentPos = QCursor::pos();
            if (!this->geometry().contains(this->mapFromGlobal(currentPos)) && 
                !toolBarGeom.contains(currentPos)) {
                m_toolBar->hide();
                qCDebug(dsrApp) << "Hiding toolbar after delay check in MainWindow::leaveEvent";
            }
        });
    }
    
    QWidget::leaveEvent(event);
}


