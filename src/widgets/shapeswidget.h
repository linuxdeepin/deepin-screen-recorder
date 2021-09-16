/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     He MingYang <hemingyang@uniontech.com>
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

#ifndef SHAPESWIDGET_H
#define SHAPESWIDGET_H

#include "../utils/shapesutils.h"
#include "../utils/baseutils.h"
#include "../widgets/textedit.h"
#include "../widgets/sidebar.h"
#include "../menucontroller/menucontroller.h"

#include <DFrame>
#include <QGestureEvent>
#include <QMouseEvent>

class ShapesWidget : public DFrame
{
    Q_OBJECT
public:
    explicit ShapesWidget(DWidget *parent = 0);
    ~ShapesWidget();

    enum ShapeBlurStatus {
        Drawing,
        Selected,
        Hovered,
        Normal,
    };
    enum ClickedKey {
        First,
        Second,
        Third,
        Fourth,
        Fifth,
        Sixth,
        Seventh,
        Eighth,
    };


signals:
    void reloadEffectImg(QString effect);
    void requestScreenshot();
    void shapePressed(QString shape);
    void saveBtnPressed(SaveAction action);
    void requestExit();
    void menuNoFocus();
    void saveFromMenu();
    void closeFromMenu();
    //选中某个形状后对应工具栏切换
    void shapeClicked(QString shape);
    void setShapesUndo(bool status);

public slots:
    /**
     * @brief updateSelectedShape: 更新选中的形状
     * @param group:
     * @param key:
     * @param index:
     */
    void updateSelectedShape(const QString &group, const QString &key, int index);
    void setCurrentShape(QString shapeType);
    //void updatePenColor();
    //void setPenColor(QColor color);
    void clearSelected();
    void setAllTextEditReadOnly();
    void setNoChangedTextEditRemove();
    void saveActionTriggered();

    void handleDrag(QPointF oldPoint, QPointF newPoint);
    void handleRotate(QPointF pos);
    void handleResize(QPointF pos, int key);

    /**
     * @brief clickedOnShapes 图形是否被点击，发射点击信号
     * @param pos
     * @return 被点击：true 未被点击：false
     */
    bool clickedOnShapes(QPointF pos);

    /**
     * @brief clickedOnRect: 矩形是否被点击
     * @param rectPoints: 矩形的四个点
     * @param pos: 当前鼠标的位置
     * @param isBlurMosaic: 是否有模糊效果或者马赛克效果
     * @return
     */
    bool clickedOnRect(FourPoints rectPoints, QPointF pos, bool isBlurMosaic = false);

    /**
     * @brief clickedOnEllipse: 圆形是否被点击
     * @param mainPoints: 圆形的主要点
     * @param pos: 当前鼠标的位置
     * @param isBlurMosaic: 是否有模糊效果或者马赛克效果
     * @return
     */
    bool clickedOnEllipse(FourPoints mainPoints, QPointF pos, bool isBlurMosaic = false);

    /**
     * @brief clickedOnArrow: 箭头是否被点击
     * @param points: 箭头的主要点
     * @param pos: 当前鼠标的位置
     * @return
     */
    bool clickedOnArrow(QList<QPointF> points, QPointF pos);

    /**
     * @brief clickedOnLine: 画出的线是否被点击
     * @param mainPoints: 画笔线的主要点
     * @param points:
     * @param pos: 当前鼠标的位置
     * @return
     */
    bool clickedOnLine(FourPoints mainPoints, QList<QPointF> points, QPointF pos);

    /**
     * @brief clickedOnText: 文本框是否被点击
     * @param mainPoints: 文本框的主要点
     * @param pos: 当前鼠标的位置
     * @return
     */
    bool clickedOnText(FourPoints mainPoints, QPointF pos);
    bool rotateOnPoint(FourPoints mainPoints, QPointF pos);

    bool hoverOnShapes(Toolshape toolShape, QPointF pos);
    bool hoverOnRect(FourPoints rectPoints, QPointF pos, bool isTextBorder = false);
    bool hoverOnEllipse(FourPoints mainPoints, QPointF pos);
    bool hoverOnArrow(QList<QPointF> points, QPointF pos);
    bool hoverOnLine(FourPoints mainPoints, QList<QPointF> points, QPointF pos);
    bool hoverOnText(FourPoints mainPoints, QPointF pos);

    bool hoverOnRotatePoint(FourPoints mainPoints, QPointF pos);
    bool textEditIsReadOnly();

    void undoDrawShapes();
    void undoAllDrawShapes();
    void deleteCurrentShape();
    //QString  getCurrentType();
    void microAdjust(QString direction);
    void setShiftKeyPressed(bool isShift);
    void updateCursorShape();
    void menuSaveSlot();
    void menuCloseSlot();
    //void updateSideBarPosition();
    void setGlobalRect(QRect rect);

protected:
    bool event(QEvent *event);
    /**
     * @brief mousePressEvent: 重写鼠标按压事件
     * @param e
     */
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *e);

    /**
     * @brief clickeShapes:只是用来判断是否选中图形,不是真实鼠标事件会触发（触摸屏）
     * @param pos:坐标
     * @return
     */
    bool clickedShapes(QPointF pos);
    /**
     * @brief pinchTriggered:捏合手势事件处理
     * @param pinch:捏合手势
     */
    void pinchTriggered(QPinchGesture *pinch);
    /**
     * @brief tapTriggered:单击手势事件处理
     * @param tap:单击手势
     */
    void tapTriggered(QTapGesture *tap);

private:
    QPointF m_pos1 = QPointF(0, 0);
    QPointF m_pos2 = QPointF(0, 0);
    QPointF m_pos3, m_pos4;

    /**
     * @brief m_pressedPoint: 当前鼠标按下的位置
     */
    QPointF m_pressedPoint;
    QPointF m_movingPoint;
    qreal   m_lastAngle;

    bool m_isRecording;
    bool m_isMoving;
    bool m_isSelected;
    bool m_isPressed;
    bool m_isHovered;
    bool m_isRotated;
    bool m_isArrowRotated;
    bool m_isResize;
    bool m_isShiftPressed;
    bool m_editing;

    /**
     * @brief m_isSelectedText: 是否选中文本框
     */
    bool m_isSelectedText;
    ResizeDirection m_resizeDirection;
    ClickedKey m_clickedKey;

    int m_shapesIndex;
    /**
     * @brief m_selectedIndex 被选中图形的索引号
     */
    int m_selectedIndex;
    int m_currentIndex;
    int m_hoveredIndex;
    int m_selectedOrder;
    bool m_blurEffectExist = false;
    bool m_mosaicEffectExist = false;
    /**
     * @brief m_currentType: 当前的图形类型
     * 现阶段存在5种图形：
     * line: 通过画笔画出的线条
     * arrow: 箭头
     * rectangle: 矩形
     * oval: 圆形
     * text: 文本
     */
    QString m_currentType = "rectangle";
    QColor m_penColor;
    bool m_clearAllTextBorder = false;
    bool m_sideBarInit = false;

    /**
     * @brief m_currentShape:当前正在绘制的图形
     */
    Toolshape m_currentShape;
    Toolshape m_selectedShape;
    Toolshape m_hoveredShape;

    QMap<int, TextEdit *> m_editMap;
    void updateTextRect(TextEdit *edit, QRectF newRect);
    Toolshapes m_shapes;
    MenuController *m_menuController;
    //SideBar *m_sideBar;

    QRect m_globalRect;

    void paintImgPoint(QPainter &painter, QPointF pos, QPixmap img, bool isResize = true);
    //void paintImgPointArrow(QPainter &painter, QPointF pos, QPixmap img);
    void paintRect(QPainter &painter, FourPoints rectFPoints, int index,
                   ShapeBlurStatus  rectStatus = Normal, bool isBlur = false, bool isMosaic = false);
    void paintEllipse(QPainter &painter, FourPoints ellipseFPoints, int index,
                      ShapeBlurStatus  ovalStatus = Normal, bool isBlur = false, bool isMosaic = false);
    void paintArrow(QPainter &painter, QList<QPointF> lineFPoints,
                    int lineWidth, bool isStraight = false);
    void paintLine(QPainter &painter, QList<QPointF> lineFPoints);
    void paintText(QPainter &painter, FourPoints rectFPoints);
};
#endif // SHAPESWIDGET_H
