// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textedit.h"
#include "../utils/configsettings.h"
#include "../utils/baseutils.h"
#include "../utils.h"
#include "../utils/log.h"
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
    qCDebug(dsrApp) << "TextEdit constructor called with index:" << index;
    m_index = index;
    setLineWrapMode(DPlainTextEdit::NoWrap); //不自动换行
    setContextMenuPolicy(Qt::NoContextMenu);//不需要菜单
    int defaultColorIndex = ConfigSettings::instance()->getValue("text", "color_index").toInt();
    QColor defaultColor = BaseUtils::colorIndexOf(defaultColorIndex);
    setColor(defaultColor);
    QFont textFont;
    //默认字体大小
    int defaultFontSize = ConfigSettings::instance()->getValue("text", "fontsize").toInt();
    textFont.setPixelSize(defaultFontSize);
    //设置默认字体
    this->document()->setDefaultFont(textFont);
//    this->setLineWrapMode(QPlainTextEdit::WidgetWidth);

    //设置光标
    QTextCursor cursor = textCursor();
    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    textBlockFormat.setAlignment(Qt::AlignLeft);
    cursor.mergeBlockFormat(textBlockFormat);

    QFontMetricsF m_fontMetric = QFontMetricsF(this->document()->defaultFont());
    QSizeF originSize = QSizeF(m_fontMetric.boundingRect("d").width()  + TEXT_MARGIN,  m_fontMetric.boundingRect("d").height());
    //记录当前光标的高度
    m_cursorHeight = cursorRect().height();
    qCDebug(dsrApp) << "Cursor height: "  << m_cursorHeight;

    this->resize(static_cast<int>(originSize.width()), static_cast<int>(originSize.height()));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //这里有两个信号容易搞混，contentsChanged这个是更改之后才会发信号，contentsChange这个信号更改之前发信号
    connect(this->document(), &QTextDocument::contentsChanged, this,  [ = ] {
        qCDebug(dsrApp) << "Text content changed, updating size";
        updateContentSize(this->toPlainText());
    });
    m_currentCursor = QCursor().pos();
}

int TextEdit::getIndex()
{
    qCDebug(dsrApp) << "TextEdit::getIndex called, returning index:" << m_index;
    return m_index;
}

void TextEdit::setColor(QColor c)
{
    qCDebug(dsrApp) << "TextEdit::setColor called with color:" << c;
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
    qCDebug(dsrApp) << "TextEdit::setFontSize called with font size:" << fontsize;
    QFont font;
    font.setPixelSize(fontsize);
    this->document()->setDefaultFont(font);
    this->updateGeometry();
    m_cursorHeight = cursorRect().height();

    updateContentSize(this->toPlainText());
}

void TextEdit::inputMethodEvent(QInputMethodEvent *e)
{
    qCDebug(dsrApp) << "TextEdit::inputMethodEvent called.";
    DPlainTextEdit::inputMethodEvent(e);

    QString virtualStr = this->toPlainText() + e->preeditString();
    qCDebug(dsrApp) << "Input method event, virtual string length:" << virtualStr.length();
    updateContentSize(virtualStr);
}

void TextEdit::updateContentSize(QString content)
{
    qCDebug(dsrApp) << "TextEdit::updateContentSize called for text length:" << content.length();
//    qDebug() << "=============" << m_count << "===============";
//    qDebug() << "content: " << content << "content.size(): " << content.size();
//    qDebug() << "blockCount() : " << blockCount() ;
//    qDebug() << "this->document()->toPlainText(): " << this->document()->toPlainText() ;
//    qDebug() << "this->document()->size(): " << this->document()->size() ;
//    qDebug() << "this->document()->textWidth(): " << this->document()->textWidth();
//    qDebug() << "this->document()->idealWidth(): " << this->document()->idealWidth();
//    qDebug() << "this->document()->indentWidth(): " << this->document()->indentWidth();
//    qDebug() << "this->document()->lineCount(): " << this->document()->lineCount();
//    qDebug() << "this->document()->defaultFont().pixelSize(): " << this->document()->defaultFont().pixelSize();
    QFontMetricsF fontMetric = QFontMetricsF(this->document()->defaultFont());
    //当前文本的大小--当前选中的文本
    QSizeF docSize =  fontMetric.size(Qt::TextDontClip,  content);
//    qDebug() << "docSize: " << docSize ;
//    qDebug() << "docSize + TEXT_MARGIN: " << (docSize.width() + TEXT_MARGIN);
    qreal mwidth = 0;
    mwidth = this->document()->size().width() + 2;

//    qDebug() << "mwidth: " << mwidth ;
    //根据当前光标的高度及行数，调整当前编辑框的大小
    this->setMinimumSize(static_cast<int>(mwidth), static_cast<int>(m_cursorHeight *  blockCount() + TEXT_MARGIN));
    this->resize(static_cast<int>(mwidth), static_cast<int>(m_cursorHeight *  blockCount() + TEXT_MARGIN));
    emit  repaintTextRect(this,  QRectF(this->x(), this->y(),
                                        mwidth, m_cursorHeight *  blockCount() + TEXT_MARGIN));

//    qDebug() << "this->size(): " << this->size();
//    qDebug() << "=============" << m_count << "===============";
//    m_count++;
}

void TextEdit::setEditing(bool edit)
{
    qCDebug(dsrApp) << "TextEdit::setEditing called with edit:" << edit;
    m_editing = edit;
}

void TextEdit::setSelecting(bool select)
{
    qCDebug(dsrApp) << "TextEdit::setSelecting called with select:" << select;
    if (select) {
        qCDebug(dsrApp) << "Text edit selected, emitting textEditSelected signal for index:" << getIndex();
        emit textEditSelected(getIndex());
    }

//    emit clickToEditing(getIndex());
}

void TextEdit::updateCursor()
{
    qCDebug(dsrApp) << "TextEdit::updateCursor called.";
//    setTextColor(Qt::green);
}

void TextEdit::setCursorVisible(bool visible)
{
    qCDebug(dsrApp) << "TextEdit::setCursorVisible called with visible:" << visible;
    if (visible) {
        qCDebug(dsrApp) << "Setting cursor width to 1.";
        setCursorWidth(1);
    } else {
        qCDebug(dsrApp) << "Setting cursor width to 0.";
        setCursorWidth(0);
    }
}
void TextEdit::mousePressEvent(QMouseEvent *e)
{
    qCDebug(dsrApp) << "TextEdit::mousePressEvent called at pos:" << e->pos();
//    if (!this->isReadOnly()) {
//        DPlainTextEdit::mousePressEvent(e);
//        return;
//    }
    m_currentCursor = QCursor().pos();
    if (m_editing == true) {
        qCDebug(dsrApp) << "TextEdit is in editing mode, passing event to DPlainTextEdit::mousePressEvent.";
        DPlainTextEdit::mousePressEvent(e);
        return;
    }

    if (e->button() == Qt::LeftButton) {
        qCDebug(dsrApp) << "Left mouse button pressed.";
        m_isPressed = true;
        m_pressPoint = QPointF(mapToGlobal(e->pos()));

        if (this->isReadOnly()) {
            qCDebug(dsrApp) << "Text edit is read-only, emitting textEditSelected signal for index:" << getIndex();
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
    m_currentCursor = QCursor().pos();
//    if (!(e->x() > 0 && e->x() < this->geometry().width() && e->y() > 0 && e->y() < this->geometry().height())) {
//        return;
//    }
    if (m_editing == true) {
        qCDebug(dsrApp) << "TextEdit is in editing mode, passing event to DPlainTextEdit::mouseMoveEvent.";
        DPlainTextEdit::mouseMoveEvent(e);
        return;
    }
    qApp->setOverrideCursor(Qt::ClosedHandCursor);
    QPointF posOrigin = QPointF(mapToGlobal(e->pos()));
    QPointF movePos = QPointF(posOrigin.x(), posOrigin.y());

    if (m_isPressed && movePos != m_pressPoint) {
        qCDebug(dsrApp) << "Mouse is pressed and moved, moving TextEdit widget.";
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
    qCDebug(dsrApp) << "TextEdit::mouseReleaseEvent called at pos:" << e->pos();
    if (m_editing == true) {
        qCDebug(dsrApp) << "TextEdit is in editing mode, passing event to DPlainTextEdit::mouseReleaseEvent.";
        DPlainTextEdit::mouseReleaseEvent(e);
        return;
    }
    m_isPressed = false;
    if (this->isReadOnly()) {
        qCDebug(dsrApp) << "Text edit is read-only, setting mouse tracking to false.";
        setMouseTracking(false);
        DPlainTextEdit::mouseReleaseEvent(e);
        return;
    }


}

void TextEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    qCDebug(dsrApp) << "TextEdit::mouseDoubleClickEvent called at pos:" << e->pos();
    this->setReadOnly(false);
    this->setCursorVisible(true);
//    emit backToEditing();
    emit clickToEditing(getIndex());

    DPlainTextEdit::mouseDoubleClickEvent(e);
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    qCDebug(dsrApp) << "TextEdit::keyPressEvent called with key:" << e->key();
    DPlainTextEdit::keyPressEvent(e);
    if (m_isPressed) {
        qCDebug(dsrApp) << "Key pressed while mouse is pressed, moving cursor.";
        Utils::cursorMove(m_currentCursor, e);
    }
    if (e->key() == Qt::Key_Escape && !this->isReadOnly()) {
        qCDebug(dsrApp) << "Escape key pressed, setting TextEdit to read-only.";
        this->setReadOnly(true);
    }
}

void TextEdit::focusInEvent(QFocusEvent *e)
{
    qCDebug(dsrApp) << "TextEdit::focusInEvent called.";
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

TextEdit::~TextEdit() {
    qCDebug(dsrApp) << "TextEdit destructor called.";
}
