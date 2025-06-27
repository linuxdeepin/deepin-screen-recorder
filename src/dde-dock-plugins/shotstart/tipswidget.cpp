// Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tipswidget.h"
#include "../../utils/log.h"

#include <QPainter>
#include <QAccessible>
#include <QTextDocument>

TipsWidget::TipsWidget(QWidget *parent)
    : QFrame(parent)
    , m_type(SingleLine)
{
    qCDebug(dsrApp) << "TipsWidget constructor called. Initial type: SingleLine.";
}

void TipsWidget::setText(const QString &text)
{
    qCDebug(dsrApp) << "setText method called with text:" << text;
    m_type = TipsWidget::SingleLine;
    // 如果传递的是富文本，获取富文本中的纯文本内容进行显示
    QTextDocument document;
    document.setHtml(text);
    // 同时去掉两边的空白信息，例如qBittorrent的提示
    m_text = document.toPlainText().simplified();
    qCDebug(dsrApp) << "Processed text for display:" << m_text;

#if 0   //测试时可以使用下面的语句
    // FIXME:藏语字体绘制会有异常，设置高度时需要使用fontMetrics().boundingRect()去获取整体的边界矩形的高度，
    // 使用fontMetrics().height()去获取时，针对藏语这种字体，其高度和实际显示区域并不等同
    m_text = "བོད་སྐད་ཡིག་གཟུགས་ཚད་ལེན་ཚོད་ལྟའི་སྐོར་གྱི་རྗོད་ཚིག";
    qCDebug(dsrApp) << "Test text for Tibetan font applied.";
#endif

    setFixedSize(fontMetrics().horizontalAdvance(m_text), fontMetrics().boundingRect(m_text).height());
    qCDebug(dsrApp) << "Fixed size set based on text metrics. Width:" << fontMetrics().horizontalAdvance(m_text) << ", Height:" << fontMetrics().boundingRect(m_text).height();

    update();

#ifndef QT_NO_ACCESSIBILITY
    if (accessibleName().isEmpty()) {
        QAccessibleEvent event(this, QAccessible::NameChanged);
        QAccessible::updateAccessibility(&event);
        qCDebug(dsrApp) << "Accessible name is empty, updating accessibility.";
    } else {
        qCDebug(dsrApp) << "Accessible name is not empty.";
    }
#endif
    qCDebug(dsrApp) << "setText method finished.";
}

void TipsWidget::setTextList(const QStringList &textList)
{
    qCDebug(dsrApp) << "setTextList method called with textList size:" << textList.size();
    m_type = TipsWidget::MultiLine;
    m_textList = textList;

    int width = 0;
    int height = 0;
    for (QString text : m_textList) {
        width = qMax(width, fontMetrics().horizontalAdvance(text));
        height += fontMetrics().boundingRect(text).height();
        qCDebug(dsrApp) << "Processing text in list:" << text << ", current max width:" << width << ", current total height:" << height;
    }

    setFixedSize(width, height);
    qCDebug(dsrApp) << "Fixed size set for multi-line text. Width:" << width << ", Height:" << height;

    update();
    qCDebug(dsrApp) << "setTextList method finished.";
}

/**
 * @brief TipsWidget::paintEvent 任务栏插件提示信息绘制
 * @param event
 */
void TipsWidget::paintEvent(QPaintEvent *event)
{
    qCDebug(dsrApp) << "paintEvent method called.";
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setPen(QPen(palette().brightText(), 1));

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);
    qCDebug(dsrApp) << "Painter and text option initialized.";

    switch (m_type) {
    case SingleLine:
        option.setWrapMode(QTextOption::NoWrap);
        painter.drawText(rect(), m_text, option);
        qCDebug(dsrApp) << "Drawing single line text:" << m_text;
        break;
    case MultiLine:
        int y= 0;
        if (m_textList.size() != 1) {
            option.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            qCDebug(dsrApp) << "Multi-line text, setting alignment to AlignLeft | AlignVCenter.";
        }
        for (QString text : m_textList) {
            int lineHeight = fontMetrics().boundingRect(text).height();
            painter.drawText(QRect(0, y, rect().width(), lineHeight), text, option);
            y += lineHeight;
        }
        break;
    }
    qCDebug(dsrApp) << "paintEvent method finished.";
}

bool TipsWidget::event(QEvent *event)
{
    qCDebug(dsrApp) << "event method called with event type:" << event->type();
    if (event->type() == QEvent::FontChange) {
        qCDebug(dsrApp) << "Event type is QEvent::FontChange.";
        switch (m_type) {
        case SingleLine:
            setText(m_text);
            qCDebug(dsrApp) << "Font change for SingleLine, re-setting text.";
            break;
        case MultiLine:
            setTextList(m_textList);
            qCDebug(dsrApp) << "Font change for MultiLine, re-setting text list.";
            break;
        }
    }
    return QFrame::event(event);
}
