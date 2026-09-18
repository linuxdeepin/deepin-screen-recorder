// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "commoniconbutton.h"
#include "../../utils/log.h"

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE
constexpr int BASE_ICON_SIZE = 24;

CommonIconButton::CommonIconButton(QWidget *parent)
    : QWidget(parent)
    , m_refreshTimer(nullptr)
    , m_rotateAngle(0)
    , m_clickable(false)
    , m_rotatable(false)
    , m_hover(false)
    , m_state(Default)
    , m_activeState(false)
{
    qCDebug(dsrApp) << "CommonIconButton constructor called.";
    setAccessibleName("IconButton");
    int iconSize = getIconSize();
    setFixedSize(iconSize, iconSize);
    if (parent) {
        setForegroundRole(parent->foregroundRole());
        qCDebug(dsrApp) << "Parent exists, setting foreground role.";
    }

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &CommonIconButton::refreshIcon);
    qCDebug(dsrApp) << "CommonIconButton constructor finished.";
}

void CommonIconButton::setStateIconMapping(QMap<State, QPair<QString, QString>> mapping)
{
    qCDebug(dsrApp) << "setStateIconMapping method called.";
    m_fileMapping = mapping;
    qCDebug(dsrApp) << "setStateIconMapping method finished.";
}

void CommonIconButton::setState(State state)
{
    qCDebug(dsrApp) << "setState method called with state:" << state;
    m_state = state;
    if (m_fileMapping.contains(state)) {
        qCDebug(dsrApp) << "File mapping contains state, setting icon from mapping.";
        auto pair = m_fileMapping.value(state);
        setIcon(pair.first, pair.second);
    } else {
        qCDebug(dsrApp) << "File mapping does not contain state.";
    }
    if (!m_icon.isNull()) {
        qCDebug(dsrApp) << "Icon is not null, setting icon with theme colors.";
        setIcon(m_icon, m_lightThemeColor, m_darkThemeColor);
    } else {
        qCDebug(dsrApp) << "Icon is null, skipping setting icon with theme colors.";
    }
    qCDebug(dsrApp) << "setState method finished.";
}

void CommonIconButton::setIcon(const QIcon &icon, QColor lightThemeColor, QColor darkThemeColor)
{
    qCDebug(dsrApp) << "setIcon method (QIcon, QColor, QColor) called.";
    m_icon = icon;
    if (lightThemeColor.isValid() && darkThemeColor.isValid()) {
        qCDebug(dsrApp) << "Theme colors are valid, applying them.";
        m_lightThemeColor = lightThemeColor;
        m_darkThemeColor = darkThemeColor;
        QColor color = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ? m_lightThemeColor : m_darkThemeColor;
        if (m_activeState) {
            color = palette().color(QPalette::Highlight);
            qCDebug(dsrApp) << "Active state is true, setting color to highlight.";
        }
        auto pa = palette();
        pa.setColor(QPalette::WindowText, color);
        setPalette(pa);
    }

    update();
    qCDebug(dsrApp) << "setIcon method (QIcon, QColor, QColor) finished.";
}

void CommonIconButton::setActiveState(bool state)
{
    qCDebug(dsrApp) << "setActiveState method called with state:" << state;
    m_activeState = state;
    if (m_lightThemeColor.isValid() && m_darkThemeColor.isValid()) {
        qCDebug(dsrApp) << "Theme colors are valid, updating icon.";
        setIcon(m_icon, m_lightThemeColor, m_darkThemeColor);
    } else {
        qCDebug(dsrApp) << "Theme colors are not valid, setting foreground role.";
        setForegroundRole(state ? QPalette::Highlight : QPalette::NoRole);
    }
    qCDebug(dsrApp) << "setActiveState method finished.";
}

void CommonIconButton::setIcon(const QString &icon, const QString &fallback, const QString &suffix)
{
    qCDebug(dsrApp) << "setIcon method (QString, QString, QString) called.";
    if (!m_fileMapping.contains(Default)) {
        m_fileMapping.insert(Default, QPair<QString, QString>(icon, fallback));
        qCDebug(dsrApp) << "Default state not in file mapping, inserting it.";
    }

    QString tmp = icon;
    QString tmpFallback = fallback;

    static auto addDarkMark = [suffix] (QString &file) {
        qCDebug(dsrApp) << "addDarkMark lambda called for file:" << file;
        if (file.contains(suffix)) {
            file.replace(suffix, "-dark" + suffix);
            qCDebug(dsrApp) << "File contains suffix, replaced with -dark suffix.";
        } else {
            file.append("-dark");
            qCDebug(dsrApp) << "File does not contain suffix, appended -dark.";
        }
    };
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        addDarkMark(tmp);
        addDarkMark(tmpFallback);
        qCDebug(dsrApp) << "Theme is LightType, applying dark mark to icons.";
    }
    m_icon = QIcon::fromTheme(tmp, QIcon::fromTheme(tmpFallback));
    update();
    qCDebug(dsrApp) << "setIcon method (QString, QString, QString) finished.";
}

void CommonIconButton::setHoverIcon(const QIcon &icon)
{
    qCDebug(dsrApp) << "setHoverIcon method called.";
    m_hoverIcon = icon;
    qCDebug(dsrApp) << "setHoverIcon method finished.";
}

void CommonIconButton::setClickable(bool clickable)
{
    qCDebug(dsrApp) << "setClickable method called with clickable:" << clickable;
    m_clickable = clickable;
    qCDebug(dsrApp) << "setClickable method finished.";
}

void CommonIconButton::setRotatable(bool rotatable)
{
    qCDebug(dsrApp) << "setRotatable method called with rotatable:" << rotatable;
    m_rotatable = rotatable;
    if (!m_rotatable) {
        qCDebug(dsrApp) << "Rotatable is false, checking and deleting refresh timer.";
        if (m_refreshTimer) {
            delete m_refreshTimer;
            qCDebug(dsrApp) << "Refresh timer deleted.";
        }
        m_refreshTimer = nullptr;
    }
    qCDebug(dsrApp) << "setRotatable method finished.";
}

void CommonIconButton::startRotate()
{
    qCDebug(dsrApp) << "startRotate method called.";
    if (!m_refreshTimer) {
        qCDebug(dsrApp) << "Refresh timer is null, creating new timer.";
        m_refreshTimer = new QTimer(this);
        m_refreshTimer->setInterval(50);
        connect(m_refreshTimer, &QTimer::timeout, this, &CommonIconButton::startRotate);
    }
    m_refreshTimer->start();
    m_rotateAngle += 54;
    update();
    if (m_rotateAngle >= 360) {
        qCDebug(dsrApp) << "Rotation angle reached 360, stopping rotate.";
        stopRotate();
    }
    qCDebug(dsrApp) << "startRotate method finished.";
}

void CommonIconButton::stopRotate()
{
    qCDebug(dsrApp) << "stopRotate method called.";
    m_refreshTimer->stop();
    m_rotateAngle = 0;
    update();
    qCDebug(dsrApp) << "stopRotate method finished.";
}

bool CommonIconButton::event(QEvent *e)
{
    qCDebug(dsrApp) << "event method called with event type:" << e->type();
    switch (e->type()) {
    case QEvent::Leave:
    case QEvent::Enter:
        m_hover = e->type() == QEvent::Enter;
        update();
        break;
    default:
        break;
    }
    return QWidget::event(e);
}

void CommonIconButton::paintEvent(QPaintEvent *e)
{
    qCDebug(dsrApp) << "paintEvent method called.";
    QWidget::paintEvent(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (m_rotateAngle != 0) {
        qCDebug(dsrApp) << "Rotate angle is not zero, applying rotation.";
        painter.translate(this->width() / 2, this->height() / 2);
        painter.rotate(m_rotateAngle);
        painter.translate(-(this->width() / 2), -(this->height() / 2));
    }

    // 在非使能模式下显示灰度图
    QIcon::Mode showMode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
    qCDebug(dsrApp) << "Show mode set to:" << (showMode == QIcon::Normal ? "Normal" : "Disabled");

    if (m_hover && !m_hoverIcon.isNull()) {
        m_hoverIcon.paint(&painter, rect(), Qt::AlignCenter, showMode);
        qCDebug(dsrApp) << "Hover icon is valid and hovered, painting hover icon.";
    } else if (!m_icon.isNull()) {
        int iconSize = getIconSize();
        painter.drawPixmap(rect(), m_icon.pixmap(iconSize));
    }
    qCDebug(dsrApp) << "paintEvent method finished.";
}

void CommonIconButton::mousePressEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mousePressEvent method called.";
    m_pressPos = event->pos();
    QWidget::mousePressEvent(event);
    qCDebug(dsrApp) << "mousePressEvent method finished.";
}

void CommonIconButton::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(dsrApp) << "mouseReleaseEvent method called.";
    if (m_clickable && rect().contains(m_pressPos) && rect().contains(event->pos()) && (!m_refreshTimer || !m_refreshTimer->isActive())) {
        qCDebug(dsrApp) << "Button is clickable and conditions met for click action.";
        if (m_rotatable) {
            startRotate();
            qCDebug(dsrApp) << "Button is rotatable, starting rotation.";
        }
        Q_EMIT clicked();
        qCDebug(dsrApp) << "Clicked signal emitted, mouseReleaseEvent finished (early exit).";
        return;
    }
    return QWidget::mouseReleaseEvent(event);
}

void CommonIconButton::refreshIcon()
{
    qCDebug(dsrApp) << "refreshIcon method called.";
    setState(m_state);
    qCDebug(dsrApp) << "refreshIcon method finished.";
}

int CommonIconButton::getIconSize() const
{
    // 获取设备像素比
    qreal ratio = devicePixelRatio();
    // 返回适配后的大小，向上取整确保不会太小
    return qCeil(BASE_ICON_SIZE * ratio);
}

