#include "timewidget.h"
#include "dde-dock/constants.h"
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QPixmap>
#include <QThread>
#include <QDBusInterface>
#include <QDBusPendingCall>

TimeWidget::TimeWidget(DWidget *parent):
    DWidget(parent),
    m_bRefresh(true),
    m_lightIcon(nullptr),
    m_shadeIcon(nullptr),
    m_currentIcon(nullptr)
{
    QFontMetrics fm(RECORDER_TIME_FONT);
    m_showTimeStr = QString("00:00:00");
    m_textSize = fm.boundingRect("00:00:00 ").size();
    m_timer = new QTimer(this);
    m_dockInter = new DBusDock("com.deepin.dde.daemon.Dock", "/com/deepin/dde/daemon/Dock", QDBusConnection::sessionBus(), this);
    connect(m_dockInter, &DBusDock::PositionChanged, this, &TimeWidget::onPositionChanged);
    m_position = m_dockInter->position();
    m_lightIcon = new QIcon(":/res/light.svg");
    m_shadeIcon = new QIcon(":/res/shade.svg");
    m_currentIcon = m_lightIcon;
}

TimeWidget::~TimeWidget()
{
    if(nullptr != m_lightIcon)
        delete m_lightIcon;
    if(nullptr != m_shadeIcon)
        delete m_shadeIcon;
    if(nullptr != m_timer)
        m_timer->deleteLater();
    if(nullptr != m_dockInter)
        m_dockInter->deleteLater();
}

bool TimeWidget::enabled()
{
    return isEnabled();
}

QSize TimeWidget::sizeHint() const
{
    QFontMetrics fm(RECORDER_TIME_FONT);
    int width = -1;
    int height = -1;
    if(0 == m_position || 2 == m_position){
        width = fm.boundingRect(RECORDER_TIME_LEVEL_SIZE).size().width();
        height = fm.boundingRect(RECORDER_TIME_LEVEL_SIZE).size().height()*2;
    }
    else if(1 == m_position || 3 == m_position){
        width = fm.boundingRect(RECORDER_TIME_VERTICAL_SIZE).size().width();
        height = fm.boundingRect(RECORDER_TIME_VERTICAL_SIZE).size().height()*2;
    }
    return QSize(width,height);
}

void TimeWidget::onTimeout()
{
    if(m_bRefresh){
        if(m_currentIcon == m_lightIcon)
            m_currentIcon = m_shadeIcon;
        else
            m_currentIcon = m_lightIcon;
    }
    m_bRefresh = !m_bRefresh;
    QTime showTime(0,0,0);
    int time = m_baseTime.secsTo(QTime::currentTime());
    showTime = showTime.addSecs(time);
    m_showTimeStr = showTime.toString("hh:mm:ss");
    update();
}

void TimeWidget::onPositionChanged(int value)
{
    m_position = value;
}

void TimeWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform,true);
    const auto ratio = devicePixelRatioF();
    if(0 == m_position || 2 == m_position){
        m_pixmap = QIcon::fromTheme(QString("recordertime"), *m_currentIcon).pixmap(QSize(RECORDER_TIME_LEVEL_ICON_SIZE, RECORDER_TIME_LEVEL_ICON_SIZE) * ratio);
        m_pixmap.setDevicePixelRatio(ratio);
        const QRectF &rf = QRectF(rect());
        const QRectF &prf = QRectF(m_pixmap.rect());
        QPointF pf = rf.center() - prf.center() / m_pixmap.devicePixelRatioF();
        painter.drawPixmap(0,static_cast<int>(pf.y()), m_pixmap);
        QFont font = RECORDER_TIME_FONT;
        //painter.setPen(Qt::white);
        painter.setFont(font);
        QFontMetrics fm(font);
        painter.drawText(m_pixmap.width()*static_cast<int>(devicePixelRatioF())+3,rect().y(),rect().width(),rect().height(), Qt::AlignLeft|Qt::AlignVCenter,m_showTimeStr);
    }
    else if(1 == m_position || 3 == m_position){
        m_pixmap = QIcon::fromTheme(QString("recordertime"), *m_currentIcon).pixmap(QSize(RECORDER_TIME_VERTICAL_ICON_SIZE, RECORDER_TIME_VERTICAL_ICON_SIZE) * ratio);
        m_pixmap.setDevicePixelRatio(ratio);
        painter.drawPixmap(0,0, m_pixmap);
    }
    QWidget::paintEvent(e);
}

//const QPixmap TimeWidget::loadSvg(const QString &fileName, const QSize &size) const
//{
//    const auto ratio = devicePixelRatioF();
//    QPixmap pixmap = QIcon::fromTheme(fileName, *m_currentIcon).pixmap(size * ratio);
//    pixmap.setDevicePixelRatio(ratio);
//    return pixmap;
//}

void TimeWidget::mousePressEvent(QMouseEvent *e)
{ 
    if(e->pos().x() > 0 && e->pos().x() < m_pixmap.width()){
        QDBusInterface notification(QString::fromUtf8("com.deepin.ScreenRecorder"),
                                    QString::fromUtf8("/com/deepin/ScreenRecorder"),
                                    QString::fromUtf8("com.deepin.ScreenRecorder"),
                                    QDBusConnection::sessionBus());
        notification.asyncCall("stopRecord");
    }
    QWidget::mousePressEvent(e);
}

void TimeWidget::start()
{
    m_showTimeStr = QString("00:00:00");
    connect(m_timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
    m_baseTime = QTime::currentTime();
    m_timer->start(400);
}

void TimeWidget::stop()
{
    disconnect(m_timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
}
