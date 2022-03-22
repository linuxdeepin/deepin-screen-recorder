/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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

#include "textedit.h"
#include "../utils/configsettings.h"
#include "../utils/baseutils.h"

#include <DPalette>

#include <QDebug>
#include <QPen>
#include <QScrollBar>
#include <QPixmap>
#include <QFontMetricsF>
#include <QApplication>


DGUI_USE_NAMESPACE

//const QSize CURSOR_SIZE = QSize(5, 20);
const int TEXT_MARGIN = 12;

TextEdit::TextEdit(int index, DWidget *parent)
    : DPlainTextEdit(parent),
      m_textColor(Qt::red)
{
    m_index = index;
    setLineWrapMode(DPlainTextEdit::NoWrap);
    setContextMenuPolicy(Qt::NoContextMenu);

    int defaultColorIndex = ConfigSettings::instance()->value(
                                "text", "color_index").toInt();
    QColor defaultColor = BaseUtils::colorIndexOf(defaultColorIndex);
    setColor(defaultColor);
    QFont textFont;
    int defaultFontSize = ConfigSettings::instance()->value("text", "fontsize").toInt();
    textFont.setPixelSize(defaultFontSize);
    this->document()->setDefaultFont(textFont);
    this->setLineWrapMode(QPlainTextEdit::WidgetWidth);

    QTextCursor cursor = textCursor();
    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    textBlockFormat.setAlignment(Qt::AlignLeft);
    cursor.mergeBlockFormat(textBlockFormat);

    QFontMetricsF m_fontMetric = QFontMetricsF(this->document()->defaultFont());
    QSizeF originSize = QSizeF(m_fontMetric.boundingRect(
                                   "d").width()  + TEXT_MARGIN,  m_fontMetric.boundingRect(
                                   "d").height() + TEXT_MARGIN);
    this->resize(static_cast<int>(originSize.width()), static_cast<int>(originSize.height()));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this->document(), &QTextDocument::contentsChange, this,  [ = ] {
        updateContentSize(this->toPlainText());
    });
}

int TextEdit::getIndex()
{
    return m_index;
}

void TextEdit::setColor(QColor c)
{
    qDebug() << "set color";
    m_textColor = c;
//    setStyleSheet(QString("TextEdit {background-color:  transparent;"
//                          " color: %1; border: none;}").arg(m_textColor.name()));

    DPalette pa = this->palette();
    pa.setColor(DPalette::Base, QColor(255, 255, 255, 0));
    pa.setColor(DPalette::Dark, QColor(255, 255, 255, 0));
    pa.setColor(DPalette::Light, QColor(255, 255, 255, 0));
    pa.setColor(DPalette::Text, c);
    this->setPalette(pa);
    this->updateGeometry();
}
/*
 * never used
QColor TextEdit::getColor()
{
    return m_textColor;
}
*/
void TextEdit::setFontSize(int fontsize)
{
    QFont font;
    font.setPixelSize(fontsize);
    this->document()->setDefaultFont(font);
    this->updateGeometry();

    updateContentSize(this->toPlainText());
}

void TextEdit::inputMethodEvent(QInputMethodEvent *e)
{
    DPlainTextEdit::inputMethodEvent(e);

    QString virtualStr = this->toPlainText() + e->preeditString();
    updateContentSize(virtualStr);
}

void TextEdit::updateContentSize(QString content)
{
    QFontMetricsF fontMetric = QFontMetricsF(this->document()->defaultFont());
    QSizeF docSize =  fontMetric.size(0,  content);
    this->setMinimumSize(static_cast<int>(docSize.width() + TEXT_MARGIN), static_cast<int>(docSize.height() + TEXT_MARGIN));
    this->resize(static_cast<int>(docSize.width() + TEXT_MARGIN), static_cast<int>(docSize.height() + TEXT_MARGIN));
    emit  repaintTextRect(this,  QRectF(this->x(), this->y(),
                                        docSize.width() + TEXT_MARGIN, docSize.height() + TEXT_MARGIN));
}

void TextEdit::setEditing(bool edit)
{
    m_editing = edit;
}

void TextEdit::setSelecting(bool select)
{
    if (select) {
        emit textEditSelected(getIndex());
    }

//    emit clickToEditing(getIndex());
}

void TextEdit::updateCursor()
{
//    setTextColor(Qt::green);
}

void TextEdit::setCursorVisible(bool visible)
{
    if (visible) {
        setCursorWidth(1);
    } else {
        setCursorWidth(0);
    }
}
void TextEdit::mousePressEvent(QMouseEvent *e)
{
    qDebug() << "TextEdit mousePressEvent" << e->pos();
//    if (!this->isReadOnly()) {
//        DPlainTextEdit::mousePressEvent(e);
//        return;
//    }
    if (m_editing == true) {
        DPlainTextEdit::mousePressEvent(e);
        return;
    }

    if (e->button() == Qt::LeftButton) {
        m_isPressed = true;
        m_pressPoint = QPointF(mapToGlobal(e->pos()));

        if (this->isReadOnly()) {
            qDebug() << "text select:" << getIndex();
            emit textEditSelected(getIndex());
        }
    }

    DPlainTextEdit::mousePressEvent(e);
}
/*
void TextEdit::wheelEvent(QWheelEvent *e)
{
    Q_UNUSED(e);
    return;
}
*/
void TextEdit::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->x() > 0 && e->x() < this->geometry().width() && e->y() > 0 && e->y() < this->geometry().height())) {
        return;
    }
    if (m_editing == true) {
        DPlainTextEdit::mouseMoveEvent(e);
        return;
    }
    qApp->setOverrideCursor(Qt::ClosedHandCursor);
    QPointF posOrigin = QPointF(mapToGlobal(e->pos()));
    QPointF movePos = QPointF(posOrigin.x(), posOrigin.y());

    if (m_isPressed && movePos != m_pressPoint) {
        this->move(static_cast<int>(this->x() + movePos.x() - m_pressPoint.x()),
                   static_cast<int>(this->y() + movePos.y() - m_pressPoint.y()));

        emit  repaintTextRect(this,  QRectF(qreal(this->x()), qreal(this->y()),
                                            this->width(),  this->height()));
        m_pressPoint = movePos;
    }


    DPlainTextEdit::mouseMoveEvent(e);
}

void TextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_editing == true) {
        DPlainTextEdit::mouseReleaseEvent(e);
        return;
    }
    m_isPressed = false;
    if (this->isReadOnly()) {
        setMouseTracking(false);
        DPlainTextEdit::mouseReleaseEvent(e);
        return;
    }


}

void TextEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    this->setReadOnly(false);
    this->setCursorVisible(true);
//    emit backToEditing();
    emit clickToEditing(getIndex());

    DPlainTextEdit::mouseDoubleClickEvent(e);
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    DPlainTextEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_Escape && !this->isReadOnly()) {
        this->setReadOnly(true);
    }
}

void TextEdit::focusInEvent(QFocusEvent *e)
{
    setAttribute(Qt::WA_InputMethodEnabled);
    DPlainTextEdit::focusInEvent(e);
}

//void TextEdit::focusOutEvent(QFocusEvent *e)
//{
//    emit textEditFinish(getIndex());
//    setReadOnly(true);
//    releaseKeyboard();
//    setEditing(false);
//    DPlainTextEdit::focusOutEvent(e);
//}

TextEdit::~TextEdit() {}
