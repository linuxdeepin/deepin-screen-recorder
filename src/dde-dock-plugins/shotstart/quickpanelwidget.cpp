#include "quickpanelwidget.h"
#include <QVBoxLayout>

#include "../../utils/log.h"
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DToolTip>
DGUI_USE_NAMESPACE
QuickPanelWidget::QuickPanelWidget(QWidget *parent)
    :QWidget(parent)
    , m_icon(new CommonIconButton(this))
    , m_description(new DLabel(this))
{
    qCDebug(dsrApp) << "QuickPanelWidget constructor called.";
    initUI();
    m_timer = new QTimer(this);
    m_showTimeStr = tr("Screenshot");
    m_type = SHOT;
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::themeTypeChanged,this,&QuickPanelWidget::refreshIcon);
    qCDebug(dsrApp) << "QuickPanelWidget constructor finished. Initial type: SHOT";
}

QuickPanelWidget::~QuickPanelWidget()
{
    qCDebug(dsrApp) << "QuickPanelWidget destructor called.";
    if (nullptr != m_timer) {
        m_timer->deleteLater();
        qCDebug(dsrApp) << "Timer deleted.";
    } else {
        qCDebug(dsrApp) << "Timer is null, no deletion needed.";
    }
    qCDebug(dsrApp) << "QuickPanelWidget destructor finished.";
}

void QuickPanelWidget::initUI()
{
    qCDebug(dsrApp) << "initUI method called.";
    m_icon->setFixedSize(QSize(24, 24));

    m_description->setElideMode(Qt::ElideRight);
    DToolTip::setToolTipShowMode(m_description, DToolTip::ShowWhenElided);
    DFontSizeManager::instance()->bind(m_description, DFontSizeManager::T10);
    qCDebug(dsrApp) << "Icon size and description properties set.";

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(0);
    layout->addStretch(1);
    layout->addWidget(m_icon, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(m_description, 0, Qt::AlignCenter);
    layout->addStretch(1);
    setLayout(layout);
    qCDebug(dsrApp) << "UI layout initialized.";
    qCDebug(dsrApp) << "initUI method finished.";
}

void QuickPanelWidget::setIcon(const QIcon &icon)
{
    qCDebug(dsrApp) << "setIcon method called.";
    m_icon->setIcon(icon);
    qCDebug(dsrApp) << "Icon set.";
    qCDebug(dsrApp) << "setIcon method finished.";
}

void QuickPanelWidget::setDescription(const QString &description)
{
    qCDebug(dsrApp) << "setDescription method called with description:" << description;
    m_description->setText(description);
    //m_description->setToolTip(description);
    qCDebug(dsrApp) << "Description set.";
    qCDebug(dsrApp) << "setDescription method finished.";
}

void QuickPanelWidget::setWidgetState(WidgetState state)
{
    qCDebug(dsrApp) << "setWidgetState method called with state:" << state;
    if (m_icon) {
        m_icon->setActiveState(WS_ACTIVE == state);
        qCDebug(dsrApp) << "Icon active state set to:" << (WS_ACTIVE == state);
    } else {
        qCDebug(dsrApp) << "Icon is null, skipping setting active state.";
    }
    qCDebug(dsrApp) << "setWidgetState method finished.";
}

void QuickPanelWidget::changeType(int type)
{
    qCDebug(dsrApp) << "changeType method called with type:" << type;
    setDescription(m_showTimeStr);
    m_type = type;
    if(type == SHOT){
        QString shotIcon = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType? "screenshot-dark" : "screenshot";
        setIcon(QIcon::fromTheme(shotIcon, QIcon(QString(":/res/%1.svg").arg(shotIcon))));
        qCDebug(dsrApp) << "Type is SHOT, setting icon to:" << shotIcon;
    }else if(type == RECORD){
        QString recordIcon("screen-recording");
        setIcon(QIcon::fromTheme(recordIcon, QIcon(QString(":/res/%1.svg").arg(recordIcon))));
        qCDebug(dsrApp) << "Type is RECORD, setting icon to:" << recordIcon;
    }else {
        qWarning() << "Type Unkonw! Please select SHOT or RECORD";
        qCDebug(dsrApp) << "Unknown type provided:" << type;
    }
    qCDebug(dsrApp) << "changeType method finished.";
}
void QuickPanelWidget::start()
{
    qCDebug(dsrApp) << "start method called.";
    m_showTimeStr = QString("00:00:00");
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_baseTime = QTime::currentTime();
    m_timer->start(400);
    qCDebug(dsrApp) << "Timer started with interval 400ms. Base time set.";
    qCDebug(dsrApp) << "start method finished.";
}

void QuickPanelWidget::stop()
{
    qCDebug(dsrApp) << "stop method called.";
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    if(m_timer->isActive()){
        m_timer->stop();
        qCDebug(dsrApp) << "Timer was active, stopped.";
    } else {
        qCDebug(dsrApp) << "Timer was not active, no stop needed.";
    }
    m_showTimeStr = tr("Screenshot");
    qCDebug(dsrApp) << "Show time string reset to Screenshot.";
    qCDebug(dsrApp) << "stop method finished.";
}

void QuickPanelWidget::pause()
{
    qCDebug(dsrApp) << "pause method called.";
    m_timer->stop();
    setDescription(m_showTimeStr);
    qCDebug(dsrApp) << "Timer stopped and description updated.";
    qCDebug(dsrApp) << "pause method finished.";
}

void QuickPanelWidget::onTimeout()
{
    qCDebug(dsrApp) << "onTimeout method called.";
    QTime showTime(0, 0, 0);
    int time = m_baseTime.secsTo(QTime::currentTime());
    showTime = showTime.addSecs(time);
    m_showTimeStr = showTime.toString("hh:mm:ss");
    qInfo() << "Current show time: " << m_showTimeStr;
    setDescription(m_showTimeStr);
    update();
    qCDebug(dsrApp) << "Time updated to:" << m_showTimeStr;
    qCDebug(dsrApp) << "onTimeout method finished.";
}

void QuickPanelWidget::refreshIcon()
{
    qCDebug(dsrApp) << "refreshIcon method called.";
    changeType(m_type);
    qCDebug(dsrApp) << "refreshIcon method finished.";
}
void QuickPanelWidget::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mouseReleaseEvent method called.";
    Q_UNUSED(event)
    if (underMouse()) {
        Q_EMIT clicked();
        qCDebug(dsrApp) << "Mouse is under widget, clicked signal emitted.";
    } else {
        qCDebug(dsrApp) << "Mouse is not under widget, no click signal emitted.";
    }
    qCDebug(dsrApp) << "mouseReleaseEvent method finished.";
}
