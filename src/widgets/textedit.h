/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Maintainer: Peng Hui<penghui@deepin.com>
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

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <DWidget>
#include <DPlainTextEdit>
#include <QPainter>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

class TextEdit : public DPlainTextEdit
{
    Q_OBJECT
public:
    TextEdit(int index, DWidget *parent);
    ~TextEdit();

public slots:
    void setColor(QColor c);
    QColor getColor();
    int getIndex();
    void updateCursor();
    void setCursorVisible(bool visible);
    void keepReadOnlyStatus();
    void setFontSize(int fontsize);
    void updateContentSize(QString content);
    void setEditing(bool edit);
    void setSelecting(bool select);

signals:
    void repaintTextRect(TextEdit *edit,  QRectF newPositiRect);
    void backToEditing();
    void clickToEditing(int index);
    void textEditSelected(int index);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void inputMethodEvent(QInputMethodEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    int m_index;
    QColor m_textColor;
    QPainter *m_painter;
    bool m_editing = false;

    QPointF m_pressPoint;
    bool m_isPressed;
};

#endif // TEXTEDIT_H
