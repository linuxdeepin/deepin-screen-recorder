// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "RecorderRegionShow.h"
#include "utils.h"
#include "utils/log.h"

#include <DWindowManagerHelper>

#include <QBitmap>
#include <QVector>

const int INDICATOR_WIDTH = 110;
const int CAMERA_Y_OFFSET = 0;

RecorderRegionShow::RecorderRegionShow():
    m_painter(nullptr),
    m_cameraWidget(nullptr)
{
    qCDebug(dsrApp) << "RecorderRegionShow constructor.";
    /*
    setAttribute (Qt::WA_AlwaysShowToolTips);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground, true);
    */
    installEventFilter(this);  // add event filter
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_painter =  new QPainter();
    qCDebug(dsrApp) << "RecorderRegionShow constructor end.";
}
RecorderRegionShow::~RecorderRegionShow()
{
    qCDebug(dsrApp) << "RecorderRegionShow destructor.";
    if (nullptr != m_painter) {
        qCDebug(dsrApp) << "Deleting m_painter.";
        delete m_painter;
        m_painter = nullptr;
    }
    if (m_cameraWidget) {
        qCDebug(dsrApp) << "m_cameraWidget exists.";
        if (m_cameraWidget->getCameraStatus()) {
            qCDebug(dsrApp) << "Camera is running, stopping camera.";
            m_cameraWidget->cameraStop();
        }
        delete m_cameraWidget;
        m_cameraWidget = nullptr;
    }
    for (int i = 0; i < m_keyButtonList.count(); ++i) {
        qCDebug(dsrApp) << "Deleting KeyButtonWidget at index:" << i;
        delete m_keyButtonList[i];
    }
    qCDebug(dsrApp) << "RecorderRegionShow destructor end.";
}

void RecorderRegionShow::initCameraInfo(const CameraWidget::Position position, const QSize size)
{
    qCDebug(dsrApp) << "Entry. Position:" << position << ", Size:" << size;
    m_cameraWidget = new CameraWidget();

    QRect r = this->geometry();
    m_cameraWidget->setFixedSize(size);
    m_cameraWidget->setDevcieName(m_deviceName);
    m_cameraWidget->initUI();
    m_cameraWidget->setRecordRect(r.x(), r.y(), r.width(), r.height());
    switch (position) {
    case CameraWidget::Position::rightBottom: {
        qCDebug(dsrApp) << "Position: rightBottom.";
        m_cameraWidget->showAt(QPoint(r.x() + r.width() - m_cameraWidget->width(), r.y() + r.height() - m_cameraWidget->height() + CAMERA_Y_OFFSET));
        break;
    }
    case CameraWidget::Position::rightTop: {
        qCDebug(dsrApp) << "Position: rightTop.";
        m_cameraWidget->showAt(QPoint(r.x() + r.width() - m_cameraWidget->width(), r.y() + CAMERA_Y_OFFSET));
        break;
    }
    case CameraWidget::Position::leftBottom: {
        qCDebug(dsrApp) << "Position: leftBottom.";
        m_cameraWidget->showAt(QPoint(r.x(), r.y() + r.height() - m_cameraWidget->height() + CAMERA_Y_OFFSET));
        break;
    }
    case CameraWidget::Position::leftTop: {
        qCDebug(dsrApp) << "Position: leftTop.";
        m_cameraWidget->showAt(QPoint(r.x(), r.y() + CAMERA_Y_OFFSET));
        break;
    }
    }
    m_cameraWidget->hide();
    m_cameraWidget->cameraStart();
    m_cameraWidget->setCameraWidgetImmovable(true); //固定窗口
    Utils::passInputEvent(static_cast<int>(m_cameraWidget->winId()));
    qCDebug(dsrApp) << "Exit.";
}

void RecorderRegionShow::showKeyBoardButtons(const QString &key)
{
    qCDebug(dsrApp) << "Entry. Key:" << key;
    KeyButtonWidget *t_keyWidget = new KeyButtonWidget(this);
    t_keyWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    t_keyWidget->setKeyLabelWord(key);
    m_keyButtonList.append(t_keyWidget);

    if (m_keyButtonList.count() > 5) {
        qCDebug(dsrApp) << "Key button list count is greater than 5. Removing the first element.";
        delete m_keyButtonList.first();
        m_keyButtonList.pop_front();
    }
    //更新多按钮的位置
    updateMultiKeyBoardPos();
    repaint();
    qCDebug(dsrApp) << "Exit.";
}

void RecorderRegionShow::updateKeyBoardButtonStyle()
{
    qCDebug(dsrApp) << "updateKeyBoardButtonStyle.";
    int count = m_keyButtonList.count();
    for (int j = 0; j < count; ++j) {
        qCDebug(dsrApp) << "Processing key button at index:" << j;
        if (DWindowManagerHelper::instance()->hasComposite()) {
            qCDebug(dsrApp) << "DWM has composite, setting blur radius to 15.";
            m_keyButtonList.at(j)->setBlurRectXRadius(15);
            m_keyButtonList.at(j)->setBlurRectYRadius(15);
        } else {
            qCDebug(dsrApp) << "DWM does not have composite, setting blur radius to 0.";
            m_keyButtonList.at(j)->setBlurRectXRadius(0);
            m_keyButtonList.at(j)->setBlurRectYRadius(0);
        }
    }
    qCDebug(dsrApp) << "updateKeyBoardButtonStyle end.";
}

// 设置Camera窗口显示
void RecorderRegionShow::setCameraShow(const bool isVisible)
{
    qCDebug(dsrApp) << "Entry. isVisible:" << isVisible;
    if (m_cameraWidget) {
        qCDebug(dsrApp) << "Setting camera widget visibility to:" << isVisible;
        m_cameraWidget->setVisible(isVisible);
    }
    qCDebug(dsrApp) << "setCameraShow end.";
}

void RecorderRegionShow::setDevcieName(const QString &devcieName)
{
    qCDebug(dsrApp) << "Entry. Device name:" << devcieName;
    m_deviceName = devcieName;
    qCDebug(dsrApp) << "setDevcieName end.";
}

void RecorderRegionShow::paintEvent(QPaintEvent *event)
{
    if (nullptr == m_painter)
        return;
    QPixmap pixmap(width(), height());
    pixmap.fill(Qt::transparent);
    m_painter->begin(&pixmap);
    m_painter->setRenderHints(QPainter::Antialiasing, true);

    QPen pen(Qt::white, 2.0);
    pen.setStyle(Qt::DashLine);
    QVector<qreal> dashes;
    dashes << 1 << 2;
    pen.setDashPattern(dashes);
    pen.setDashOffset(0);

    m_painter->setPen(pen);
    m_painter->setOpacity(1);
    m_painter->drawRect(0, 0, width(), height());
    m_painter->end();

    m_painter->begin(this);
    m_painter->drawPixmap(QPoint(0, 0), pixmap);
    m_painter->end();
    setMask(pixmap.mask());
    event->accept();
}
void RecorderRegionShow::updateMultiKeyBoardPos()
{
    qCDebug(dsrApp) << "updateMultiKeyBoardPos.";
    QPoint t_keyPoint[5];
    static float posfix[5][5] = {{-0.5f, 0}, {-(0.5f + 1 / 1.5f), (1 / 1.5f - 0.5f), 0}, {-1.8f, -0.5f, 0.8f, 0}, {-2.5f, -(0.5f + 1 / 1.5f), (1 / 1.5f - 0.5f), 1.5, 0}, {-3.1f, -1.8f, -0.5, 0.8f, 2.1f}};
    QRect r = this->geometry();
    int recordX = r.x();
    int recordY = r.y();
    int recordWidth = r.width();
    int recordHeight = r.height();


    int count = m_keyButtonList.count();
    qCDebug(dsrApp) << "Number of key buttons:" << count; // Log count
    for (int j = 0; j < count; ++j) {
        qCDebug(dsrApp) << "Processing key button at index:" << j; // Log loop iteration
        m_keyButtonList.at(j)->hide();
        t_keyPoint[j] = QPoint(static_cast<int>(recordX + recordWidth / 2 + m_keyButtonList.at(j)->width() * posfix[count - 1][j]), std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
        m_keyButtonList.at(j)->move(t_keyPoint[j].x(), t_keyPoint[j].y());
        m_keyButtonList.at(j)->show();
    }

    /*

    QPoint t_keyPoint1;
    QPoint t_keyPoint2;
    QPoint t_keyPoint3;
    QPoint t_keyPoint4;
    QPoint t_keyPoint5;

    QRect r = this->geometry();
    int recordX = r.x();
    int recordY = r.y();
    int recordWidth = r.width();
    int recordHeight = r.height();

    if (!m_keyButtonList.isEmpty()) {
        switch (m_keyButtonList.count()) {
        //一个按键的情况
        case 1:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();
            break;
            //两个按键的情况
        case 2:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 + m_keyButtonList.at(1)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();
            break;
            //三个按键的情况o
        case 3:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2 + m_keyButtonList.at(2)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();
            break;
            //四个按键的情况
        case 4:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 - m_keyButtonList.at(1)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2 + m_keyButtonList.at(2)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(3)->width() * 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();
            break;
            //五个按键的情况
        case 5:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 2.6),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 - m_keyButtonList.at(1)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(3)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();

            m_keyButtonList.at(4)->hide();
            t_keyPoint5 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(4)->width() * 2.6),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(4)->move(t_keyPoint5.x(), t_keyPoint5.y());
            m_keyButtonList.at(4)->show();
            break;
        default:
            break;
        }
    }
    */
    qCDebug(dsrApp) << "updateMultiKeyBoardPos end.";
}
