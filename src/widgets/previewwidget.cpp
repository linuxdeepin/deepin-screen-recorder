#include "previewwidget.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

PreviewWidget::PreviewWidget(const QRect &rect, QWidget *parent) : QWidget(parent), m_previewRect(rect)
{
    setGeometry(rect);
    m_previewRect = rect;
    m_maxHeight = QApplication::desktop()->screen()->geometry().height() * 7 / 10;
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    //paint
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QImage image = m_currentPix.scaledToHeight(m_previewRect.height(), Qt::SmoothTransformation);
    this->resize(image.width(), m_previewRect.height());
    painter.drawImage(rect(), m_currentPix);
}

void PreviewWidget::setRect(const QRect &rect)
{
    m_previewRect = rect;
}

