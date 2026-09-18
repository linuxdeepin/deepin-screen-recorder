// SPDX-FileCopyrightText: 2021-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quickpanelwidget.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DToolTip>
#include <QVBoxLayout>
#include "../../utils/log.h"

DGUI_USE_NAMESPACE

QuickPanelWidget::QuickPanelWidget(QWidget *parent)
    : QWidget(parent)
    , m_icon(new CommonIconButton(this))
    , m_description(new DLabel(this))
{
    qCDebug(dsrApp) << "QuickPanelWidget constructor called.";
    initUI();
    m_timer = new QTimer(this);
    m_showTimeStr = tr("Record");
    m_type = RECORD;
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &QuickPanelWidget::refreshIcon);
    qCDebug(dsrApp) << "QuickPanelWidget constructor finished. Initial type: RECORD.";
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

/**
   @brief 设置当前显示的图标为 \a icon ，默认状态和录制模式显示不同图标
 */
void QuickPanelWidget::setIcon(const QIcon &icon)
{
    qCDebug(dsrApp) << "setIcon method called.";
    m_icon->setIcon(icon);
    qCDebug(dsrApp) << "Icon set.";
    qCDebug(dsrApp) << "setIcon method finished.";
}

/**
   @brief 设置显示的文案为 \a description ，录制时将显示"录制中"文案
 */
void QuickPanelWidget::setDescription(const QString &description)
{
    qCDebug(dsrApp) << "setDescription method called with description:" << description;
    m_description->setText(description);
    // 同样更新提示文案
    m_description->setToolTip(description);
    qCDebug(dsrApp) << "Description and tooltip set.";
    qCDebug(dsrApp) << "setDescription method finished.";
}

/**
   @brief 切换截图或录屏模式为 \a type
   @sa FuctionType
 */
void QuickPanelWidget::changeType(FuctionType type)
{
    qCDebug(dsrApp) << "changeType method called with type:" << type;
    qCDebug(dsrApp) << "Changing function type to:" << type;
    setDescription(m_showTimeStr);
    m_type = type;
    if (type == RECORD) {
        QString shotIcon = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ?
                               "status-screen-record-dark" :
                               "status-screen-record";
        qCDebug(dsrApp) << "Type is RECORD, setting icon to:" << shotIcon;
        setIcon(QIcon::fromTheme(shotIcon, QIcon(QString(":/res/%1.svg").arg(shotIcon))));
    } else if (type == RECORDING) {
        QString recordIcon("screen-recording");
        qCDebug(dsrApp) << "Type is RECORDING, setting icon to:" << recordIcon;
        setIcon(QIcon::fromTheme(recordIcon, QIcon(QString(":/res/%1.svg").arg(recordIcon))));
    } else {
        qWarning() << "Type Unkonw! Please select RECORD or RECORDING";
        qCDebug(dsrApp) << "Unknown type provided:" << type;
    }
    qCDebug(dsrApp) << "changeType method finished.";
}

/**
   @brief start:开始计时
 */
void QuickPanelWidget::start()
{
    qCDebug(dsrApp) << "start method called.";
    qCInfo(dsrApp) << "Starting timer";
    m_showTimeStr = QString("00:00:00");
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_baseTime = QTime::currentTime();
    m_timer->start(400);
    qCDebug(dsrApp) << "Timer started with interval 400ms. Base time set.";
    qCDebug(dsrApp) << "start method finished.";
}

/**
   @brief stop:停止计时
 */
void QuickPanelWidget::stop()
{
    qCDebug(dsrApp) << "stop method called.";
    qCInfo(dsrApp) << "Stopping timer";
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    if (m_timer->isActive()) {
        qCDebug(dsrApp) << "Timer was active, stopping it";
        m_timer->stop();
    } else {
        qCDebug(dsrApp) << "Timer was not active, no stop needed.";
    }
    m_showTimeStr = tr("Record");
    qCDebug(dsrApp) << "Show time string reset to Record.";
    qCDebug(dsrApp) << "stop method finished.";
}

/**
   @brief pause:暂停
 */
void QuickPanelWidget::pause()
{
    qCDebug(dsrApp) << "pause method called.";
    qCInfo(dsrApp) << "Pausing timer";
    m_timer->stop();
    setDescription(m_showTimeStr);
    qCDebug(dsrApp) << "Timer stopped and description updated.";
    qCDebug(dsrApp) << "pause method finished.";
}

/**
   @brief 超时时更新显示的时间范围
 */
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

/**
   @brief 刷新图标，重设当前的显示图标
 */
void QuickPanelWidget::refreshIcon()
{
    qCDebug(dsrApp) << "refreshIcon method called.";
    changeType(m_type);
    qCDebug(dsrApp) << "refreshIcon method finished.";
}

/**
   @brief 鼠标点击抛出点击信号
 */
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
