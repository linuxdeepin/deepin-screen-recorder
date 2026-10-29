// Copyright (C) 2020-2026 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../utils/calculaterect.h"
#include "../utils/configsettings.h"
#include "../utils/tempfile.h"
#include "../utils/shapesutils.h"
#include "shapeswidget.h"
#include "../utils.h"
#include "../utils/log.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QGestureEvent>
#include <QFontMetricsF>
#include <QIntValidator>
#include <QLineEdit>
#include <QTimer>
#include "tooltips.h"

#include <cmath>

#define LINEWIDTH(index) (index*2+3)
#define SEQUENCE_DIAMETER(idx) (24 + (idx)*8)

namespace {

// 下一个编号超过 99 时的禁用角标：跟随鼠标显示，不显示下一个编号预览。
const QSize SEQUENCE_PREVIEW_SIZE(17, 17);
// 光标热点到禁用角标左上角的偏移（设计稿 Group 3 相对鼠标的位置）。
const int SEQUENCE_PREVIEW_ANCHOR = 5;

// 最大编号：设计稿只示意两位编号，达到后不再允许新增
const int SEQUENCE_MAX_NUMBER = 99;

// 超限态角标（设计稿 序列号2 的 Group 3）：17x17 白色底圆 + 14x14 红圆 + 30° 白色横条 10x1
const qreal SEQUENCE_LIMIT_BADGE_SIZE = 17.0;
const qreal SEQUENCE_LIMIT_BADGE_INNER_SIZE = 14.0;
const qreal SEQUENCE_LIMIT_BADGE_BAR_WIDTH = 10.0;
const qreal SEQUENCE_LIMIT_BADGE_BAR_HEIGHT = 1.0;
const qreal SEQUENCE_LIMIT_BADGE_BAR_ANGLE = 30.0;
const QColor SEQUENCE_LIMIT_BADGE_BASE_COLOR(Qt::white);
const QColor SEQUENCE_LIMIT_BADGE_COLOR(0xFE, 0x00, 0x00);
// 提示左上角对齐角标右下角：相对光标再偏移一个角标边长
const int SEQUENCE_LIMIT_TIPS_ANCHOR = SEQUENCE_PREVIEW_ANCHOR + int(SEQUENCE_LIMIT_BADGE_SIZE);

bool pointInEllipse(const FourPoints &points, const QPointF &pos)
{
    if (points.size() < 4)
        return false;

    const QRectF rect(points[0], points[3]);
    const QRectF normalized = rect.normalized();
    if (normalized.width() <= 0.0 || normalized.height() <= 0.0)
        return false;

    const qreal radiusX = normalized.width() / 2.0;
    const qreal radiusY = normalized.height() / 2.0;
    const qreal normalizedX = (pos.x() - normalized.center().x()) / radiusX;
    const qreal normalizedY = (pos.y() - normalized.center().y()) / radiusY;
    return normalizedX * normalizedX + normalizedY * normalizedY <= 1.0;
}

int configuredLineWidthIndex(const QString &group)
{
    // Persisted settings can be corrupted or written by older versions. The
    // toolbar exposes exactly three line-width tiers, so never feed an
    // out-of-range value into geometry calculations.
    return qBound(0, ConfigSettings::instance()->getValue(group, "line_width").toInt(), 2);
}

int configuredLineWidth(const QString &group)
{
    return LINEWIDTH(configuredLineWidthIndex(group));
}

/**
 * @brief 序号超限禁用角标
 *
 * 只在下一个编号超过 99 时跟随鼠标显示；正常序号模式下不显示下一个编号预览。
 */
class SequenceLimitBadgeWidget : public QWidget
{
public:
    explicit SequenceLimitBadgeWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(SEQUENCE_PREVIEW_SIZE);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        paintLimitBadge(painter);
    }

private:
    // 超限角标：白底圆 + 红圆 + 白色横条（自绘，不使用 Qt 样式表）
    void paintLimitBadge(QPainter &painter)
    {
        const qreal size = SEQUENCE_LIMIT_BADGE_SIZE;
        const qreal inset = (size - SEQUENCE_LIMIT_BADGE_INNER_SIZE) / 2.0;
        painter.setPen(Qt::NoPen);
        painter.setBrush(SEQUENCE_LIMIT_BADGE_BASE_COLOR);
        painter.drawEllipse(QRectF(0, 0, size, size));
        painter.setBrush(SEQUENCE_LIMIT_BADGE_COLOR);
        painter.drawEllipse(QRectF(inset, inset, SEQUENCE_LIMIT_BADGE_INNER_SIZE,
                                   SEQUENCE_LIMIT_BADGE_INNER_SIZE));
        painter.save();
        painter.translate(size / 2.0, size / 2.0);
        painter.rotate(SEQUENCE_LIMIT_BADGE_BAR_ANGLE);
        painter.setBrush(SEQUENCE_LIMIT_BADGE_BASE_COLOR);
        painter.drawRect(QRectF(-SEQUENCE_LIMIT_BADGE_BAR_WIDTH / 2.0,
                                -SEQUENCE_LIMIT_BADGE_BAR_HEIGHT / 2.0,
                                SEQUENCE_LIMIT_BADGE_BAR_WIDTH,
                                SEQUENCE_LIMIT_BADGE_BAR_HEIGHT));
        painter.restore();
    }
};

} // namespace

const int DRAG_BOUND_RADIUS = 8;
const int SPACING = 12;
const int RESIZEPOINT_WIDTH = 15;
const QSize ROTATE_ICON_SIZE = QSize(30, 30);

ShapesWidget::ShapesWidget(DWidget *parent)
    : DFrame(parent),
      m_lastAngle(0),
      m_isMoving(false),
      m_isSelected(false),
      m_isShiftPressed(false),
      m_editing(false),
      m_shapesIndex(-1),
      m_selectedIndex(-1),
      m_selectedOrder(-1),
      m_menuController(new MenuController)
{
    //订阅手势事件
    QList<Qt::GestureType> gestures;
    //    gestures << Qt::PanGesture;
    gestures << Qt::PinchGesture;
    //    gestures << Qt::SwipeGesture;
    gestures << Qt::TapGesture;
    //    gestures << Qt::TapGesture;
    //    gestures << Qt::TapAndHoldGesture;
    //    gestures << Qt::CustomGesture;
    //    gestures << Qt::LastGestureType;
    foreach (Qt::GestureType gesture, gestures)
        grabGesture(gesture);


    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);

    m_penColor = BaseUtils::colorIndexOf(ConfigSettings::instance()->getValue("pen", "color_index").toInt());

    connect(m_menuController, &MenuController::shapePressed,
            this, &ShapesWidget::shapePressed);
    //    connect(m_menuController, &MenuController::saveBtnPressed,
    //            this, &ShapesWidget::saveBtnPressed);
    connect(m_menuController, &MenuController::saveAction,
            this, &ShapesWidget::menuSaveSlot);
    connect(m_menuController, &MenuController::closeAction,
            this, &ShapesWidget::menuCloseSlot);
    connect(m_menuController, &MenuController::unDoAction,
            this, &ShapesWidget::undoDrawShapes);
    connect(m_menuController, &MenuController::menuNoFocus,
            this, &ShapesWidget::menuNoFocus);
    connect(this, &ShapesWidget::setShapesUndo,
            m_menuController, &MenuController::setUndoEnable);
    connect(ConfigSettings::instance(), &ConfigSettings::shapeConfigChanged, this, &ShapesWidget::updateSelectedShape);
    //m_sideBar = new SideBar(this);
    //m_sideBar->hide();
    m_currentCursor = QCursor().pos();

    // 序号超限禁用角标：仅在下一编号超过 99 时跟随光标显示（自绘，不使用 Qt 样式表）
    m_sequencePreview = new SequenceLimitBadgeWidget(this);
    m_sequencePreview->hide();

    // 超限气泡限时显示：点击弹出，2 秒后气泡消失，角标继续跟随鼠标
    m_sequenceLimitTipsTimer = new QTimer(this);
    m_sequenceLimitTipsTimer->setSingleShot(true);
    m_sequenceLimitTipsTimer->setInterval(2000);
    connect(m_sequenceLimitTipsTimer, &QTimer::timeout, this, [this] {
        if (m_sequenceLimitTips) {
            m_sequenceLimitTips->hide();
        }
    });
}

ShapesWidget::~ShapesWidget()
{
    if (m_menuController) {
        delete m_menuController;
        m_menuController = nullptr;
    }
    if (m_sequenceEdit) {
        delete m_sequenceEdit;
        m_sequenceEdit = nullptr;
    }
    if (m_sequenceLimitTipsTimer) {
        m_sequenceLimitTipsTimer->stop();
    }
    if (m_sequenceLimitTips) {
        m_sequenceLimitTips->hide();
    }
}
//更新选中的形状
void ShapesWidget::updateSelectedShape(const QString &group,
                                       const QString &key, int index)
{
    qCDebug(dsrApp) << "Updating selected shape - group:" << group << "key:" << key << "index:" << index;
    
    if (m_isSelectedText) {
        qCDebug(dsrApp) << "Skipping update for selected text";
        m_isSelectedText = false;
        return;
    }
    
    if (group == m_currentShape.type && key == "color_index") {
        m_penColor = BaseUtils::colorIndexOf(index);
        qCDebug(dsrApp) << "Updated pen color to:" << m_penColor;
    }

    if (m_selectedIndex != -1 && m_selectedOrder != -1 && m_selectedOrder < m_shapes.length()) {
        qCDebug(dsrApp) << "Updating shape properties for selected shape at order:" << m_selectedOrder;
        
        if (m_selectedShape.type == "sequence" && key == "line_width") {
            // 序号圆圈的直径由大小档位换算，实时调整选中项而不是仅记忆到下次新建
            resizeSelectedSequence(index);
        } else if ((m_selectedShape.type == "arrow" || m_selectedShape.type == "line") && key != "color_index") {
            m_selectedShape.lineWidth = LINEWIDTH(index);
        } else if (m_selectedShape.type == group && key == "line_width") {
            m_selectedShape.lineWidth = LINEWIDTH(index);
        } else if (group == "text" && m_selectedShape.type == group && key == "color_index") {
            int tmpIndex = m_shapes[m_selectedOrder].index;
            if (m_editMap.contains(tmpIndex)) {
                m_selectedShape.colorIndex = index;
                m_editMap.value(tmpIndex)->setColor(BaseUtils::colorIndexOf(index));
                m_editMap.value(tmpIndex)->update();
            }

        } else if (group == "text" && m_selectedShape.type == group && key == "fontsize")  {
            const int tmpIndex = m_shapes[m_selectedOrder].index;
            if (TextEdit *edit = m_editMap.value(tmpIndex, nullptr)) {
                edit->setFontSize(index);
                edit->update();
            } else if (isTreeLandMode()
                       && !m_selectedShape.text.isEmpty()
                       && m_selectedShape.mainPoints.size() >= 4) {
                const int fontSize = qMax(1, index);
                m_selectedShape.fontSize = fontSize;
                m_shapes[m_selectedOrder].fontSize = fontSize;
                // TreeLand deletes the temporary inline editor after commit.
                // Recompute the committed text box here so the selection border,
                // content bounds and exported image all follow the new font size.
                QFont font;
                font.setPixelSize(fontSize);
                const QFontMetricsF metrics(font);
                const QStringList lines = m_selectedShape.text.split(QLatin1Char('\n'));
                qreal textWidth = 0.0;
                for (const QString &line : lines)
                    textWidth = qMax(textWidth, metrics.horizontalAdvance(line));
                const qreal width = std::ceil(textWidth + 12.0);
                const qreal height = std::ceil(metrics.lineSpacing()
                                               * qMax(1, lines.size()) + 12.0);
                const QPointF topLeft = m_selectedShape.mainPoints[0];
                m_selectedShape.mainPoints[1] = topLeft + QPointF(0.0, height);
                m_selectedShape.mainPoints[2] = topLeft + QPointF(width, 0.0);
                m_selectedShape.mainPoints[3] = topLeft + QPointF(width, height);
            }
        } else if (group != "text" && m_selectedShape.type == group && key == "color_index") {
            m_selectedShape.colorIndex = index;
            if (m_selectedShape.type == "sequence") {
                // 序号颜色与大小一样需要记忆，作用于下一个新建序号
                ConfigSettings::instance()->setValue("sequence", "color_index", index);
            }
        } else if (group == "effect" && m_selectedShape.type == group &&
                   key == "radius" && (m_selectedShape.isOval == 0 || m_selectedShape.isOval == 1)) {

            m_selectedShape.radius = index * 3 + 10;
            if (m_selectedShape.isBlur) {
                emit reloadEffectImg("blur", m_selectedShape.radius);
            } else {
                emit reloadEffectImg("mosaic", m_selectedShape.radius);
            }
        }

        if (m_selectedOrder < m_shapes.length()) {
            m_shapes[m_selectedOrder] = m_selectedShape;
            qCDebug(dsrApp) << "Shape updated at order:" << m_selectedOrder;
        }
        update();
        emit shapesChanged();
    }
    //qDebug() << ">>>>> function: " << __func__ << ", line: " << __LINE__ << ", m_selectedShape.type: " << m_selectedShape.type;
}
/*
 * never used
void ShapesWidget::updatePenColor()
{
    setPenColor(colorIndexOf(ConfigSettings::instance()->value(
                                 "common", "color_index").toInt()));
}
*/
void ShapesWidget::setCurrentShape(QString shapeType)
{
    qCDebug(dsrApp, ) << __FUNCTION__ << __LINE__ << "type: " << shapeType;
    m_currentType = shapeType;
    if (shapeType.isEmpty()) {
        // 收起二级工具栏后进入普通模式：不创建新图形，但仍允许选中和编辑已有标注。
        m_isRecording = false;
        m_currentShape = Toolshape();
        m_pos1 = QPointF(0, 0);
        m_pos2 = QPointF(0, 0);
        clearSelected();
        setAllTextEditReadOnly();
        m_editing = false;
        if (m_sequencePreview) {
            m_sequencePreview->hide();
            hideSequenceLimitTips();
        }
        update();
        return;
    }

    if (shapeType == "text") {
        resetForTextToolSwitch();
    } else {
        setAllTextEditReadOnly();
    }

    if (shapeType == "sequence") {
        // 进入序号工具时工具栏会恢复上次的粗细档位，此时不应缩放之前选中的序号；
        // 本函数在工具栏恢复完成之后才被调用，延后到下一轮事件循环即可覆盖恢复过程。
        m_sequenceToolEntry = true;
        QTimer::singleShot(0, this, [this] { m_sequenceToolEntry = false; });
        if (m_sequencePreview) {
            updateSequencePreview(mapFromGlobal(QCursor::pos()));
        }
        updateCursorShape();
    } else if (m_sequencePreview) {
        m_sequencePreview->hide();
        hideSequenceLimitTips();
    }
}

void ShapesWidget::setEffectSourceImage(const QImage &image)
{
    m_effectSourceImage = image;
    m_blurEffectPixmaps.clear();
    m_mosaicEffectPixmaps.clear();
    update();
}

void ShapesWidget::resetForTextToolSwitch()
{
    // 切换到 text 时仍保留上一个图形的选中态，
    // 导致第一次点击先“清选中”，第二次点击才进入文本创建。
    // 这里在工具切换时清理选中态，保证首次点击直接进入文本输入。
    // 工具栏同步“已选中文字”也会切到 text；此时必须保留文字选中态，
    // 否则紧接着的拖拽会被中断。
    if (!isTreeLandMode() || m_selectedShape.type != "text")
        clearSelected();
    m_isRotated = false;
    m_isResize = false;
    m_clickedKey = Unknow;
}

void ShapesWidget::clearSelected()
{
    qCDebug(dsrApp) << "Clearing selected shape";

    for (int j = 0; j < m_selectedShape.mainPoints.length(); j++) {
        m_selectedShape.mainPoints[j] = QPointF(0, 0);
    }
    for (int j = 0; j < m_hoveredShape.mainPoints.length(); j++) {
        m_hoveredShape.mainPoints[j] = QPointF(0, 0);
    }

    //qDebug() << "clear selected!!!";
    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;
    m_isArrowRotated = false;
    m_resizeDirection = Outting;
    m_clickedKey = Unknow;
    m_isMoving = false;
    m_selectedIndex = -1;
    m_hoveredIndex = -1;
    m_selectedOrder = -1;
    m_selectedShape.points.clear();
    m_hoveredShape.points.clear();
    updateCursorShape();
}

void ShapesWidget::setAllTextEditReadOnly()
{
    qCDebug(dsrApp) << "Setting all text edits to read-only";
    
    QMap<int, TextEdit *>::iterator i = m_editMap.begin();
    while (i != m_editMap.end()) {
        i.value()->setReadOnly(true);
        i.value()->releaseKeyboard();
        i.value()->setEditing(false);

        QTextCursor textCursor =  i.value()->textCursor();
        textCursor.clearSelection();
        i.value()->setTextCursor(textCursor);
        ++i;
    }

    setNoChangedTextEditRemove();
    update();
    emit shapesChanged();
}

void ShapesWidget::setNoChangedTextEditRemove()
{
    qCDebug(dsrApp) << "Removing unchanged text edits";
    
    if (m_shapes.length() == 0) {
        return;
    }
    for (int i = 0; i < m_shapes.length(); i++) {
        if (m_shapes[i].type == "text") {
            int t_tempIndex = m_shapes[i].index;
            // m_editMap.value() 在 key 不存在时返回 nullptr（QMap 的默认构造指针），
            // 后续 ->document() 会段错误，需提前判空。
            QPlainTextEdit *edit = m_editMap.value(t_tempIndex);
            if (!edit) {
                qCWarning(dsrApp) << "setNoChangedTextEditRemove: edit for index" << t_tempIndex << "is null, skip.";
                continue;
            }
            if (edit->document()->toPlainText() == QString(tr("Input text here"))
                    || edit->document()->toPlainText().isEmpty()) {
                qCDebug(dsrApp) << "Removing empty text edit at index:" << t_tempIndex;
                m_shapes.removeAt(i);
                edit->clear();
                m_editMap.remove(t_tempIndex);

                break;
            }

        }

    }
    if (m_shapes.length() == 0) {
        qCDebug(dsrApp) << "No shapes remaining, disabling undo";
        emit setShapesUndo(false);
    }

    update();
    emit shapesChanged();
}

void ShapesWidget::saveActionTriggered()
{
    qCInfo(dsrApp, ) << __FUNCTION__ << __LINE__ << "正在执行清除图形编辑界面...";
    setAllTextEditReadOnly();
    clearSelected();
    m_clearAllTextBorder = true;
    qCInfo(dsrApp, ) << __FUNCTION__ << __LINE__ << "已清楚图形编辑界面";
}

//点击某个形状
bool ShapesWidget::clickedOnShapes(QPointF pos, bool includeClosedShapeInterior)
{
    qCDebug(dsrApp) << "Checking for shape click at position:" << pos;
    bool onShapes = false;
    m_selectedOrder = -1;
    // 上一次选择可能停留在缩放/旋转状态；命中检测会按当前图形重新设置。
    // 序号只参与整体移动，不复用其它图形遗留的缩放/旋转语义。
    m_isResize = false;
    m_isRotated = false;
    m_isArrowRotated = false;
    m_resizeDirection = Outting;
    m_clickedKey = Unknow;
    //    qDebug() << ">>>>> function: " << __func__ << ", line: " << __LINE__
    //             << ", pos: " << pos
    //             << ", m_shapes.length(): " << m_shapes.length();
    for (int i = 0; i < m_shapes.length(); i++) {
        //当前是否有形状被选中
        bool currentOnShape = false;
        if (m_shapes[i].type == "rectangle") {
            if (clickedOnRect(m_shapes[i].mainPoints, pos,
                              includeClosedShapeInterior)) {
                qCDebug(dsrApp) << "Clicked on rectangle at index:" << i;
                currentOnShape = true;
                emit shapeClicked("rect");
            }
        }
        if (m_shapes[i].type == "oval") {
            const bool hitBoundary = clickedOnEllipse(m_shapes[i].mainPoints,
                                                       pos, false);
            const bool hitInterior = includeClosedShapeInterior
                    && pointInEllipse(m_shapes[i].mainPoints, pos);
            if (hitBoundary || hitInterior) {
                qCDebug(dsrApp) << "Clicked on oval at index:" << i;
                if (hitInterior && !hitBoundary) {
                    // TreeLand moves a closed shape when its interior is
                    // dragged. clickedOnEllipse only initializes that state
                    // for an edge hit, so do it explicitly for the interior.
                    m_isSelected = true;
                    m_isResize = false;
                    m_isRotated = false;
                    m_resizeDirection = Moving;
                    m_pressedPoint = pos;
                }
                currentOnShape = true;
                emit shapeClicked("circ");
            }
        }
        if (m_shapes[i].type == "effect") {
            if (m_shapes[i].isOval == 0) {
                if (clickedOnEllipse(m_shapes[i].mainPoints, pos, true)) {
                    currentOnShape = true;
                    emit shapeClicked("effect");
                }
            } else if (m_shapes[i].isOval == 1) {
                if (clickedOnRect(m_shapes[i].mainPoints, pos, true)) {
                    currentOnShape = true;
                    emit shapeClicked("effect");
                }
            } else {
                if (clickedOnLine(m_shapes[i].mainPoints, m_shapes[i].points, pos)) {
                    continue;
                }
            }

        }
        if (m_shapes[i].type == "arrow") {
            if (clickedOnArrow(m_shapes[i].points, pos)) {
                currentOnShape = true;
                emit shapeClicked("arrow");
            }
        }
        if (m_shapes[i].type == "line") {
            if (clickedOnArrow(m_shapes[i].points, pos)) {
                currentOnShape = true;
                emit shapeClicked("line");
            }
        }
        if (m_shapes[i].type == "pen") {
            if (clickedOnLine(m_shapes[i].mainPoints, m_shapes[i].points, pos)) {
                currentOnShape = true;
                emit shapeClicked("pen");
            }
        }

        if (m_shapes[i].type == "text") {
            if (clickedOnText(m_shapes[i].mainPoints, pos)) {
                currentOnShape = true;
                emit shapeClicked("text");
            }
        }

        if (m_shapes[i].type == "sequence") {
            // 序号标注：仅内部命中（包围矩形 contains），不启用四角缩放
            if (m_shapes[i].mainPoints.length() == 4) {
                QRectF seqRect(m_shapes[i].mainPoints[0], m_shapes[i].mainPoints[3]);
                if (seqRect.contains(pos)) {
                    currentOnShape = true;
                    m_isSelected = true;
                    m_isResize = false;
                    m_isRotated = false;
                    m_resizeDirection = Moving;
                    emit shapeClicked("sequence");
                }
            }
        }

        if (currentOnShape) {
            m_selectedShape = m_shapes[i];
            m_selectedIndex = m_shapes[i].index;
            m_selectedOrder = i;
            onShapes = true;
            break;
        } else {
            m_selectedIndex = -1;
            m_selectedOrder = -1;
            update();
            continue;
        }
    }
    return onShapes;
}

//判断是否选中图形,不是真实鼠标事件会触发
bool ShapesWidget::clickedShapes(QPointF pos)
{
    for (int i = 0; i < m_shapes.length(); i++) {
        if (m_shapes[i].type == "rectangle") {
            if (clickedOnRect(m_shapes[i].mainPoints, pos, false)) {
                return true;
            }
        }
        if (m_shapes[i].type == "oval") {
            if (clickedOnEllipse(m_shapes[i].mainPoints, pos, false)) {
                return true;
            }
        }

        if (m_shapes[i].type == "effect") {
            if (m_shapes[i].isOval == 0) {
                if (clickedOnEllipse(m_shapes[i].mainPoints, pos, true)) {
                    return true;
                }
            } else if (m_shapes[i].isOval == 1) {
                if (clickedOnRect(m_shapes[i].mainPoints, pos, true)) {
                    return true;
                }
            } else {
                if (clickedOnLine(m_shapes[i].mainPoints, m_shapes[i].points, pos)) {
                    return true;
                }
            }
        }

        if (m_shapes[i].type == "arrow" || m_shapes[i].type == "line") {
            if (clickedOnArrow(m_shapes[i].points, pos)) {
                return true;
            }
        }
        if (m_shapes[i].type == "pen") {
            if (clickedOnLine(m_shapes[i].mainPoints, m_shapes[i].points, pos)) {
                return true;
            }
        }

        if (m_shapes[i].type == "text") {
            if (clickedOnText(m_shapes[i].mainPoints, pos)) {
                return true;
            }
        }
    }
    return false;
}

//TODO: selectUnique
//矩形是否被点击
bool ShapesWidget::clickedOnRect(FourPoints rectPoints, QPointF pos, bool isBlurMosaic)
{
    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;

    QPointF point1 = rectPoints[0];
    QPointF point2 = rectPoints[1];
    QPointF point3 = rectPoints[2];
    QPointF point4 = rectPoints[3];

    FourPoints otherFPoints = getAnotherFPoints(rectPoints);
    if (pointClickIn(point1, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = First;
        m_resizeDirection = TopLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point2, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Second;
        m_resizeDirection = BottomLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point3, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Third;
        m_resizeDirection = TopRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point4, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fourth;
        m_resizeDirection = BottomRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[0], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fifth;
        m_resizeDirection = Left;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[1], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Sixth;
        m_resizeDirection = Top;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[2], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Seventh;
        m_resizeDirection = Right;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[3], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Eighth;
        m_resizeDirection = Bottom;
        m_pressedPoint = pos;
        return true;
    } else if (rotateOnPoint(rectPoints, pos)) {
        m_isSelected = true;
        m_isRotated = true;
        m_isResize = false;
        m_resizeDirection = Rotate;
        m_pressedPoint = pos;
        return true;
    } else if (pointOnLine(rectPoints[0], rectPoints[1], pos) ||
               pointOnLine(rectPoints[1], rectPoints[3], pos) ||
               pointOnLine(rectPoints[3], rectPoints[2], pos) ||
               pointOnLine(rectPoints[2], rectPoints[0], pos)) {
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else if (isBlurMosaic && pointInRect(rectPoints, pos)) {
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    }  else {
        m_isSelected = false;
        m_isResize = false;
        m_isRotated = false;
    }

    return false;
}

//圆形是否被点击
bool ShapesWidget::clickedOnEllipse(FourPoints mainPoints, QPointF pos, bool isBlurMosaic)
{
    qCDebug(dsrApp) << "clickedOnEllipse called with pos:" << pos << ", isBlurMosaic:" << isBlurMosaic;
    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;

    m_pressedPoint = pos;
    FourPoints otherFPoints = getAnotherFPoints(mainPoints);
    if (pointClickIn(mainPoints[0], pos)) {
        qCDebug(dsrApp) << "Clicked on mainPoints[0] (TopLeft).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = First;
        m_resizeDirection = TopLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[1], pos)) {
        qCDebug(dsrApp) << "Clicked on mainPoints[1] (BottomLeft).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Second;
        m_resizeDirection = BottomLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[2], pos)) {
        qCDebug(dsrApp) << "Clicked on mainPoints[2] (TopRight).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Third;
        m_resizeDirection = TopRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[3], pos)) {
        qCDebug(dsrApp) << "Clicked on mainPoints[3] (BottomRight).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fourth;
        m_resizeDirection = BottomRight;
        m_pressedPoint = pos;
        return true;
    }  else if (pointClickIn(otherFPoints[0], pos)) {
        qCDebug(dsrApp) << "Clicked on otherFPoints[0] (Left).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fifth;
        m_resizeDirection = Left;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[1], pos)) {
        qCDebug(dsrApp) << "Clicked on otherFPoints[1] (Top).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Sixth;
        m_resizeDirection = Top;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[2], pos)) {
        qCDebug(dsrApp) << "Clicked on otherFPoints[2] (Right).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Seventh;
        m_resizeDirection = Right;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[3], pos)) {
        qCDebug(dsrApp) << "Clicked on otherFPoints[3] (Bottom).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Eighth;
        m_resizeDirection = Bottom;
        m_pressedPoint = pos;
        return true;
    } else if (rotateOnPoint(mainPoints, pos)) {
        qCDebug(dsrApp) << "Clicked on rotate point.";
        m_isSelected = true;
        m_isRotated = true;
        m_isResize = false;
        m_resizeDirection = Rotate;
        m_pressedPoint = pos;
        return true;
    }  else if (pointOnEllipse(mainPoints, pos)) {
        qCDebug(dsrApp) << "Clicked on ellipse boundary.";
        m_isSelected = true;
        m_isResize = false;

        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else if (isBlurMosaic && pointInRect(mainPoints, pos)) {
        qCDebug(dsrApp) << "Clicked inside blur/mosaic ellipse area.";
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        qCDebug(dsrApp) << "No click detected on ellipse.";
        m_isSelected = false;
        m_isResize = false;
        m_isRotated = false;
    }

    return false;
}

//箭头是否被点击
bool ShapesWidget::clickedOnArrow(QList<QPointF> points, QPointF pos)
{
    qCDebug(dsrApp) << "clickedOnArrow called with pos:" << pos;
    if (points.length() != 2) {
        qCDebug(dsrApp) << "Arrow points length is not 2, returning false.";
        return false;
    }

    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;
    m_isArrowRotated = false;


    if (pointClickIn(points[0], pos)) {
        qCDebug(dsrApp) << "Clicked on arrow point 0 (start point).";
        m_isSelected = true;
        m_isRotated = true;
        m_isArrowRotated = false;
        if (isTreeLandMode())
            m_clickedKey = First;
        m_resizeDirection = Left;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(points[1], pos)) {
        qCDebug(dsrApp) << "Clicked on arrow point 1 (end point).";
        m_isSelected = true;
        m_isRotated = true;
        m_isArrowRotated = false;
        if (isTreeLandMode())
            m_clickedKey = Second;
        m_resizeDirection = Right;
        m_pressedPoint = pos;
        return true;
    } else if (pointOnLine(points[0], points[1], pos)) {
        qCDebug(dsrApp) << "Clicked on arrow line.";
        m_isSelected = true;
        m_isRotated = false;
        if (isTreeLandMode())
            m_clickedKey = Unknow;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        qCDebug(dsrApp) << "No click detected on arrow.";
        m_isSelected = false;
        m_isRotated = false;
        m_isResize = false;
        m_isArrowRotated = false;
        if (isTreeLandMode())
            m_clickedKey = Unknow;
        m_resizeDirection = Outting;
        m_pressedPoint = pos;
        return false;
    }

}

//画出的线是否被点击
bool ShapesWidget::clickedOnLine(FourPoints mainPoints,
                                 QList<QPointF> points,
                                 QPointF pos)
{
    qCDebug(dsrApp) << "clickedOnLine called with pos:" << pos;
    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;

    m_pressedPoint = QPoint(0, 0);
    FourPoints otherFPoints = getAnotherFPoints(mainPoints);
    if (pointClickIn(mainPoints[0], pos)) {
        qCDebug(dsrApp) << "Clicked on mainPoints[0] (TopLeft).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = First;
        m_resizeDirection = TopLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[1], pos)) {
        qCDebug(dsrApp) << "Clicked on mainPoints[1] (BottomLeft).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Second;
        m_resizeDirection = BottomLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[2], pos)) {
        qCDebug(dsrApp) << "Clicked on mainPoints[2] (TopRight).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Third;
        m_resizeDirection = TopRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[3], pos)) {
        qCDebug(dsrApp) << "Clicked on mainPoints[3] (BottomRight).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fourth;
        m_resizeDirection = BottomRight;
        m_pressedPoint = pos;
        return true;
    }  else if (pointClickIn(otherFPoints[0], pos)) {
        qCDebug(dsrApp) << "Clicked on otherFPoints[0] (Left).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fifth;
        m_resizeDirection = Left;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[1], pos)) {
        qCDebug(dsrApp) << "Clicked on otherFPoints[1] (Top).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Sixth;
        m_resizeDirection = Top;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[2], pos)) {
        qCDebug(dsrApp) << "Clicked on otherFPoints[2] (Right).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Seventh;
        m_resizeDirection = Right;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[3], pos)) {
        qCDebug(dsrApp) << "Clicked on otherFPoints[3] (Bottom).";
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Eighth;
        m_resizeDirection = Bottom;
        m_pressedPoint = pos;
        return true;
    } else if (rotateOnPoint(mainPoints, pos)) {
        qCDebug(dsrApp) << "Clicked on rotate point.";
        m_isSelected = true;
        m_isRotated = true;
        m_isResize = false;
        m_resizeDirection = Rotate;
        m_pressedPoint = pos;
        return true;
    }  else if (pointOnArLine(points, pos)) {
        qCDebug(dsrApp) << "Clicked on arrow line.";
        m_isSelected = true;
        m_isResize = false;

        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        qCDebug(dsrApp) << "No click detected on line.";
        m_isSelected = false;
        m_isResize = false;
        m_isRotated = false;
    }

    return false;
}

//文本框是否被点击
bool ShapesWidget::clickedOnText(FourPoints mainPoints, QPointF pos)
{
    qCDebug(dsrApp) << "clickedOnText called with pos:" << pos;
    if (pointInRect(mainPoints, pos)) {
        qCDebug(dsrApp) << "Click detected inside text rectangle.";
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;

        return true;
    } else {
        qCDebug(dsrApp) << "Click not detected inside text rectangle.";
        m_isSelected = false;
        m_isResize = false;

        return false;
    }
}

bool ShapesWidget::hoverOnRect(FourPoints rectPoints, QPointF pos, bool isTextBorder)
{
    qCDebug(dsrApp) << "hoverOnRect called with pos:" << pos << ", isTextBorder:" << isTextBorder;
    FourPoints tmpFPoints = getAnotherFPoints(rectPoints);
    if (pointClickIn(rectPoints[0], pos)) {
        qCDebug(dsrApp) << "Hovered on TopLeft resize point.";
        m_resizeDirection = TopLeft;
        return true;
    } else if (pointClickIn(rectPoints[1], pos)) {
        qCDebug(dsrApp) << "Hovered on BottomLeft resize point.";
        m_resizeDirection = BottomLeft;
        return true;
    } else if (pointClickIn(rectPoints[2], pos)) {
        qCDebug(dsrApp) << "Hovered on TopRight resize point.";
        m_resizeDirection = TopRight;
        return true;
    } else if (pointClickIn(rectPoints[3], pos)) {
        qCDebug(dsrApp) << "Hovered on BottomRight resize point.";
        m_resizeDirection = BottomRight;
        return true;
    } else if (rotateOnPoint(rectPoints, pos) && m_selectedIndex != -1
               && m_selectedIndex == m_hoveredIndex && !isTextBorder) {
        qCDebug(dsrApp) << "Hovered on rotate point.";
        m_resizeDirection = Rotate;
        return true;
    }  else if (pointClickIn(tmpFPoints[0], pos)) {
        qCDebug(dsrApp) << "Hovered on Left resize point.";
        m_resizeDirection = Left;
        return true;
    } else if (pointClickIn(tmpFPoints[1], pos)) {
        qCDebug(dsrApp) << "Hovered on Top resize point.";
        m_resizeDirection = Top;
        return true;
    }  else if (pointClickIn(tmpFPoints[2], pos)) {
        qCDebug(dsrApp) << "Hovered on Right resize point.";
        m_resizeDirection = Right;
        return true;
    } else if (pointClickIn(tmpFPoints[3], pos)) {
        qCDebug(dsrApp) << "Hovered on Bottom resize point.";
        m_resizeDirection = Bottom;
        return true;
    } else if (pointOnLine(rectPoints[0],  rectPoints[1], pos) || pointOnLine(rectPoints[1],
                                                                              rectPoints[3], pos) || pointOnLine(rectPoints[3], rectPoints[2], pos) ||
               pointOnLine(rectPoints[2], rectPoints[0], pos)) {
        qCDebug(dsrApp) << "Hovered on rectangle border.";
        m_resizeDirection = Moving;
        return true;
    } else {
        qCDebug(dsrApp) << "No hover detected on rectangle.";
        m_resizeDirection = Outting;
    }
    return false;
}

bool ShapesWidget::hoverOnEllipse(FourPoints mainPoints, QPointF pos)
{
    qCDebug(dsrApp) << "hoverOnEllipse called with pos:" << pos;
    FourPoints tmpFPoints = getAnotherFPoints(mainPoints);

    if (pointClickIn(mainPoints[0], pos)) {
        qCDebug(dsrApp) << "Hovered on TopLeft resize point.";
        m_resizeDirection = TopLeft;
        return true;
    } else if (pointClickIn(mainPoints[1], pos)) {
        qCDebug(dsrApp) << "Hovered on BottomLeft resize point.";
        m_resizeDirection = BottomLeft;
        return true;
    } else if (pointClickIn(mainPoints[2], pos)) {
        qCDebug(dsrApp) << "Hovered on TopRight resize point.";
        m_resizeDirection = TopRight;
        return true;
    } else if (pointClickIn(mainPoints[3], pos)) {
        qCDebug(dsrApp) << "Hovered on BottomRight resize point.";
        m_resizeDirection = BottomRight;
        return true;
    } else if (rotateOnPoint(mainPoints, pos) && m_selectedIndex != -1
               && m_selectedIndex == m_hoveredIndex) {
        qCDebug(dsrApp) << "Hovered on rotate point.";
        m_resizeDirection = Rotate;
        return true;
    }  else if (pointClickIn(tmpFPoints[0], pos)) {
        qCDebug(dsrApp) << "Hovered on Left resize point.";
        m_resizeDirection = Left;
        return true;
    } else if (pointClickIn(tmpFPoints[1], pos)) {
        qCDebug(dsrApp) << "Hovered on Top resize point.";
        m_resizeDirection = Top;
        return true;
    }  else if (pointClickIn(tmpFPoints[2], pos)) {
        qCDebug(dsrApp) << "Hovered on Right resize point.";
        m_resizeDirection = Right;
        return true;
    } else if (pointClickIn(tmpFPoints[3], pos)) {
        qCDebug(dsrApp) << "Hovered on Bottom resize point.";
        m_resizeDirection = Bottom;
        return true;
    }  else if (pointOnEllipse(mainPoints, pos)) {
        qCDebug(dsrApp) << "Hovered on ellipse boundary.";
        m_isSelected = true;
        m_isResize = false;

        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        qCDebug(dsrApp) << "No hover detected on ellipse.";
        m_resizeDirection = Outting;
    }
    return false;
}

bool ShapesWidget::hoverOnArrow(QList<QPointF> points, QPointF pos)
{
    qCDebug(dsrApp) << "hoverOnArrow called with pos:" << pos;
    if (points.length() != 2) {
        qCDebug(dsrApp) << "Arrow points length is not 2, returning false.";
        return false;
    }
    bool result = false;
//    QPointF t_rotatepos;

//    qreal t_minx = qMin(points[1].x(), points[0].x());
//    qreal t_miny = qMin(points[1].y(), points[0].y());
//    int t_height = qAbs(points[1].y() - points[0].y());
//    int t_width = qAbs(points[1].x() - points[0].x());

//    t_rotatepos.setX(t_minx + t_width / 2 - 5);
//    t_rotatepos.setY(t_miny + t_height / 2 - 35);

    //第一个点的偏移量
    qreal firstPointPadX = std::abs(pos.x() - points[0].x());
    qreal firstPointPadY = std::abs(pos.y() - points[0].y());
    //第二个点的偏移量
    qreal secondPointPadX = std::abs(pos.x() - points[1].x());
    qreal secondPointPadY = std::abs(pos.y() - points[1].y());
    //端点的偏移最大值
    int pointPadding = 4;
    if (pointOnLine(points[0], points[1], pos) &&
            ((firstPointPadX > pointPadding || firstPointPadY > pointPadding)  &&
             (secondPointPadX > pointPadding  || secondPointPadY > pointPadding))) {
        qCDebug(dsrApp) << "Hovered on arrow line, not near endpoints.";
        m_resizeDirection = Moving;
        m_clickedKey = Unknow;
        result = true;
    } else if (m_selectedIndex != -1 && m_selectedIndex == m_hoveredIndex
               && pointClickIn(points[0], pos) && firstPointPadX <= pointPadding && firstPointPadY <= pointPadding) {
        qCDebug(dsrApp) << "Hovered on first arrow endpoint.";
        m_clickedKey = First;
        m_resizeDirection = Rotate;
        result = true;
    } else if (m_selectedIndex != -1 && m_selectedIndex == m_hoveredIndex
               && pointClickIn(points[1], pos) && secondPointPadX <= pointPadding && secondPointPadY <= pointPadding) {
        qCDebug(dsrApp) << "Hovered on second arrow endpoint.";
        m_clickedKey =   Second;
        m_resizeDirection = Rotate;
        result = true;
    } /*else if ( m_selectedIndex != -1 && m_selectedIndex == m_hoveredIndex
                && pointClickIn(t_rotatepos, pos)) {
        m_clickedKey =  Third;
        m_resizeDirection = Rotate;
        return true;*/
//    }
    else {
        qCDebug(dsrApp) << "No hover detected on arrow.";
        m_resizeDirection = Outting;
        result = false;
    }
    return result;
}

bool ShapesWidget::hoverOnLine(FourPoints mainPoints, QList<QPointF> points,
                               QPointF pos)
{
    qCDebug(dsrApp) << "hoverOnLine called with pos:" << pos;
    FourPoints tmpFPoints = getAnotherFPoints(mainPoints);

    if (pointClickIn(mainPoints[0], pos)) {
        qCDebug(dsrApp) << "Hovered on TopLeft resize point.";
        m_resizeDirection = TopLeft;
        return true;
    } else if (pointClickIn(mainPoints[1], pos)) {
        qCDebug(dsrApp) << "Hovered on BottomLeft resize point.";
        m_resizeDirection = BottomLeft;
        return true;
    } else if (pointClickIn(mainPoints[2], pos)) {
        qCDebug(dsrApp) << "Hovered on TopRight resize point.";
        m_resizeDirection = TopRight;
        return true;
    } else if (pointClickIn(mainPoints[3], pos)) {
        qCDebug(dsrApp) << "Hovered on BottomRight resize point.";
        m_resizeDirection = BottomRight;
        return true;
    } else if (rotateOnPoint(mainPoints, pos) && m_selectedIndex != -1
               && m_selectedIndex == m_hoveredIndex) {
        qCDebug(dsrApp) << "Hovered on rotate point.";
        m_resizeDirection = Rotate;
        return true;
    }  else if (pointClickIn(tmpFPoints[0], pos)) {
        qCDebug(dsrApp) << "Hovered on Left resize point.";
        m_resizeDirection = Left;
        return true;
    } else if (pointClickIn(tmpFPoints[1], pos)) {
        qCDebug(dsrApp) << "Hovered on Top resize point.";
        m_resizeDirection = Top;
        return true;
    }  else if (pointClickIn(tmpFPoints[2], pos)) {
        qCDebug(dsrApp) << "Hovered on Right resize point.";
        m_resizeDirection = Right;
        return true;
    } else if (pointClickIn(tmpFPoints[3], pos)) {
        qCDebug(dsrApp) << "Hovered on Bottom resize point.";
        m_resizeDirection = Bottom;
        return true;
    }  else if (pointOnArLine(points, pos)) {
        qCDebug(dsrApp) << "Hovered on line segment.";
        m_isSelected = true;
        m_isResize = false;

        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        qCDebug(dsrApp) << "No hover detected on line.";
        m_resizeDirection = Outting;
    }
    return false;
}

bool ShapesWidget::hoverOnText(int textIndex, FourPoints mainPoints, QPointF pos)
{
    qCDebug(dsrApp) << "hoverOnText called with textIndex:" << textIndex << "pos:" << pos;
    //qDebug() << "hoverOnText:" <<  mainPoints << pos;
    if (hoverOnRect(mainPoints, pos, true) ||
            (pos.x() >= mainPoints[0].x() - 5
             && pos.x() <= mainPoints[2].x() + 5
             && pos.y() >= mainPoints[0].y() - 5
             && pos.y() <= mainPoints[2].y() + 5)) {

        if (m_editMap.contains(textIndex) && m_editMap[textIndex]->isReadOnly()) {
            m_resizeDirection = Moving;
            return true;
        }
    }

    m_resizeDirection = Outting;
    return false;
}

bool ShapesWidget::hoverOnShapes(Toolshape toolShape, QPointF pos)
{
    if (toolShape.type == "rectangle") {
        return hoverOnRect(toolShape.mainPoints, pos);
    } else if (toolShape.type == "oval") {
        return hoverOnEllipse(toolShape.mainPoints, pos);
    } else if (toolShape.type == "arrow" || toolShape.type == "line") {
        return hoverOnArrow(toolShape.points, pos);
    } else if (toolShape.type == "pen") {
        return hoverOnLine(toolShape.mainPoints, toolShape.points, pos);
    } else if (toolShape.type == "text") {
        return hoverOnText(toolShape.index, toolShape.mainPoints, pos);
    } else if (toolShape.type == "effect" && toolShape.isOval == 0) {
        return hoverOnEllipse(toolShape.mainPoints, pos);
    } else if (toolShape.type == "effect" && toolShape.isOval == 1) {
        return hoverOnRect(toolShape.mainPoints, pos);
    }

    m_hoveredShape.type = "";
    return false;
}

bool ShapesWidget::rotateOnPoint(FourPoints mainPoints,
                                 QPointF pos)
{
    bool result = hoverOnRotatePoint(mainPoints, pos);
    return result;
}

bool ShapesWidget::hoverOnRotatePoint(FourPoints mainPoints, QPointF pos)
{
    QPointF rotatePoint = getRotatePoint(mainPoints[0], mainPoints[1],
                                         mainPoints[2], mainPoints[3]);
    rotatePoint = QPointF(rotatePoint.x() - 5, rotatePoint.y() - 5);
    bool result = false;
    if (pos.x() >= rotatePoint.x() - SPACING && pos.x() <= rotatePoint.x() + SPACING
            && pos.y() >= rotatePoint.y() - SPACING && pos.y() <= rotatePoint.y() + SPACING) {
        result = true;
    } else {
        result = false;
    }

    m_pressedPoint = rotatePoint;
    return result;
}

bool ShapesWidget::textEditIsReadOnly()
{
    qCDebug(dsrApp, ) << "textEditIsReadOnly:" << m_editMap.count();

    QMap<int, TextEdit *>::iterator i = m_editMap.begin();
    while (i != m_editMap.end()) {
        if (m_editing || !i.value()->isReadOnly()) {
            setAllTextEditReadOnly();
            m_editing = false;
            m_currentShape.type = "";
            update();
            return true;
        }
        ++i;
    }

    return false;
}

ShapesWidget::InteractionHit ShapesWidget::interactionAt(const QPointF &pos) const
{
    auto nearPoint = [&pos](const QPointF &point) {
        return pointClickIn(point, pos);
    };
    auto closedShapeHit = [&pos, this](const Toolshape &shape) {
        if (shape.mainPoints.size() < 4)
            return false;
        if (shape.type == "oval") {
            if (pointOnEllipse(shape.mainPoints, pos))
                return true;
            if (isTreeLandMode() && pointInEllipse(shape.mainPoints, pos))
                return true;
        } else if (shape.type == "effect" && shape.isOval == 0) {
            if (pointOnEllipse(shape.mainPoints, pos) || pointInRect(shape.mainPoints, pos))
                return true;
        } else if (pointOnLine(shape.mainPoints[0], shape.mainPoints[1], pos)
                   || pointOnLine(shape.mainPoints[1], shape.mainPoints[3], pos)
                   || pointOnLine(shape.mainPoints[3], shape.mainPoints[2], pos)
                   || pointOnLine(shape.mainPoints[2], shape.mainPoints[0], pos)) {
            return true;
        }
        return (shape.type == "effect" && shape.isOval == 1
                && pointInRect(shape.mainPoints, pos))
                || (shape.type == "rectangle" && isTreeLandMode()
                    && pointInRect(shape.mainPoints, pos));
    };
    auto handleHit = [&nearPoint](const Toolshape &shape) {
        InteractionHit hit;
        if (shape.mainPoints.size() < 4)
            return hit;
        const FourPoints middle = getAnotherFPoints(shape.mainPoints);
        const QList<QPair<QPointF, InteractionHandle>> handles = {
            {shape.mainPoints[0], InteractionHandle::TopLeft},
            {shape.mainPoints[1], InteractionHandle::BottomLeft},
            {shape.mainPoints[2], InteractionHandle::TopRight},
            {shape.mainPoints[3], InteractionHandle::BottomRight},
            {middle[0], InteractionHandle::Left},
            {middle[1], InteractionHandle::Top},
            {middle[2], InteractionHandle::Right},
            {middle[3], InteractionHandle::Bottom}
        };
        for (const auto &entry : handles) {
            if (nearPoint(entry.first)) {
                hit.kind = InteractionKind::Resize;
                hit.handle = entry.second;
                return hit;
            }
        }
        return hit;
    };

    for (const Toolshape &shape : m_shapes) {
        if (shape.type == "arrow" || shape.type == "line") {
            if (shape.points.size() != 2)
                continue;
            if (nearPoint(shape.points[0])) {
                return {InteractionKind::Rotate, InteractionHandle::Left};
            }
            if (nearPoint(shape.points[1])) {
                return {InteractionKind::Rotate, InteractionHandle::Right};
            }
            if (pointOnLine(shape.points[0], shape.points[1], pos))
                return {InteractionKind::Move, InteractionHandle::None};
            continue;
        }

        if (shape.type == "sequence") {
            if (shape.mainPoints.size() >= 4
                    && QRectF(shape.mainPoints[0], shape.mainPoints[3]).contains(pos)) {
                return {InteractionKind::Move, InteractionHandle::None};
            }
            continue;
        }

        InteractionHit handle = handleHit(shape);
        if (handle.isValid())
            return handle;

        if (shape.mainPoints.size() >= 4 && shape.type != "text"
                && shape.type != "pen") {
            const QPointF rotatePoint = getRotatePoint(shape.mainPoints[0],
                                                       shape.mainPoints[1],
                                                       shape.mainPoints[2],
                                                       shape.mainPoints[3]);
            if (QRectF(rotatePoint.x() - 17, rotatePoint.y() - 17,
                       34, 34).contains(pos)) {
                return {InteractionKind::Rotate, InteractionHandle::None};
            }
        }

        if (shape.type == "pen" || (shape.type == "effect" && shape.isOval == 2)) {
            for (int i = 1; i < shape.points.size(); ++i) {
                if (pointOnLine(shape.points[i - 1], shape.points[i], pos))
                    return {InteractionKind::Move, InteractionHandle::None};
            }
            if (shape.points.size() == 1 && pointClickIn(shape.points[0], pos))
                return {InteractionKind::Move, InteractionHandle::None};
            continue;
        }

        if (shape.type == "text") {
            if (shape.mainPoints.size() >= 4
                    && QRectF(shape.mainPoints[0], shape.mainPoints[3]).contains(pos)) {
                // TreeLand hosts inline editing in a separate widget, so a
                // committed text annotation is moved as a whole there.
                // X11 keeps its legacy explicit TextEditor hit.
                return {isTreeLandMode()
                        ? InteractionKind::Move
                        : InteractionKind::TextEditor,
                        InteractionHandle::None};
            }
            continue;
        }

        if (closedShapeHit(shape))
            return {InteractionKind::Move, InteractionHandle::None};
    }

    return {};
}

bool ShapesWidget::beginInteraction(const QPointF &pos)
{
    if (!clickedOnShapes(pos, isTreeLandMode())) {
        const bool canCreate = m_currentType == "rectangle"
                || m_currentType == "oval"
                || m_currentType == "arrow"
                || m_currentType == "line"
                || m_currentType == "pen"
                || m_currentType == "effect"
                || (isTreeLandMode()
                    && (m_currentType == "sequence" || m_currentType == "text"));
        if (!canCreate)
            return false;

        m_currentShape = Toolshape();
        m_selectedShape = Toolshape();
        m_hoveredShape = Toolshape();
        m_currentShape.type = m_currentType;
        m_currentShape.colorIndex = ConfigSettings::instance()
                ->getValue(m_currentType, "color_index").toInt();
        m_currentShape.lineWidth = configuredLineWidth(m_currentType);
        if (m_currentType == "effect") {
            m_currentShape.isBlur = ConfigSettings::instance()
                    ->getValue("effect", "isBlur").toBool();
            m_currentShape.isOval = ConfigSettings::instance()
                    ->getValue("effect", "isOval").toInt();
            m_currentShape.radius = ConfigSettings::instance()
                    ->getValue("effect", "radius").toInt() * 3 + 10;
        }

        m_currentIndex = ++m_shapesIndex;
        m_currentShape.index = m_currentIndex;
        if (isTreeLandMode() && m_currentType == "sequence") {
            // Sequence is a single-click tool, so it is committed before the
            // release event instead of waiting for a non-empty drag rectangle.
            placeSequenceAt(pos);
            m_currentShape = Toolshape();
            m_isRecording = false;
            update();
            emit shapesChanged();
            // Even at the 99-item limit the click belongs to the annotation
            // tool; returning false would hand the same press to selection.
            return true;
        }
        if (isTreeLandMode() && m_currentType == "text") {
            const QString placeholder = tr("Input text here");
            const int fontSize = qMax(1, ConfigSettings::instance()
                                             ->getValue("text", "fontsize").toInt());
            QFont font;
            font.setPixelSize(fontSize);
            const QFontMetricsF metrics(font);
            const QSizeF initialSize(metrics.horizontalAdvance(placeholder) + 14.0,
                                     metrics.height() + 12.0);
            m_currentShape.text = placeholder;
            m_currentShape.fontSize = fontSize;
            m_currentShape.mainPoints = getMainPoints(pos, pos, false);
            m_currentShape.mainPoints[0] = pos;
            m_currentShape.mainPoints[1] = pos + QPointF(0, initialSize.height());
            m_currentShape.mainPoints[2] = pos + QPointF(initialSize.width(), 0);
            m_currentShape.mainPoints[3] = pos + QPointF(initialSize.width(), initialSize.height());
            m_shapes.append(m_currentShape);
            m_selectedIndex = m_currentShape.index;
            m_selectedOrder = m_shapes.size() - 1;
            m_selectedShape = m_currentShape;
            m_isSelected = true;
            m_isRecording = false;

            m_pendingInlineEdit = InlineEditRequest();
            m_pendingInlineEdit.kind = InlineEditKind::Text;
            m_pendingInlineEdit.shapeIndex = m_currentShape.index;
            m_pendingInlineEdit.localRect = QRectF(m_currentShape.mainPoints[0],
                                                    m_currentShape.mainPoints[3]).normalized();
            m_pendingInlineEdit.text = placeholder;
            m_pendingInlineEdit.foregroundColor =
                    BaseUtils::colorIndexOf(m_currentShape.colorIndex);
            m_pendingInlineEdit.fontSize = fontSize;
            m_pendingInlineEdit.selectAll = true;
            m_pendingInlineEdit.isNew = true;
            m_hasPendingInlineEdit = true;
            update();
            emit shapesChanged();
            return true;
        }
        m_pos1 = pos;
        m_pos2 = pos;
        if (m_currentType == "arrow" || m_currentType == "line"
                || m_currentType == "pen"
                || (m_currentType == "effect" && m_currentShape.isOval == 2)) {
            m_currentShape.points.append(pos);
        }
        m_currentShape.mainPoints = getMainPoints(pos, pos, false);
        m_isRecording = true;
        m_interactionActive = true;
        m_interactionOrder = -1;
        m_interactionStartPos = pos;
        m_interactionLastPos = pos;
        m_interactionBackup = Toolshape();
        m_isPressed = true;
        m_pressedPoint = pos;
        m_movingPoint = pos;
        return true;
    }

    m_interactionActive = true;
    m_interactionOrder = m_selectedOrder;
    m_interactionStartPos = pos;
    m_interactionLastPos = pos;
    m_interactionBackup = m_selectedOrder >= 0 && m_selectedOrder < m_shapes.size()
            ? m_shapes[m_selectedOrder]
            : Toolshape();
    m_isPressed = true;
    m_pressedPoint = pos;
    m_movingPoint = pos;
    return true;
}

void ShapesWidget::updateInteraction(const QPointF &pos)
{
    if (!m_interactionActive)
        return;

    m_isPressed = true;
    m_movingPoint = pos;
    if (m_isRecording) {
        m_pos2 = pos;
        if (m_currentShape.type == "arrow" || m_currentShape.type == "line") {
            if (m_currentShape.points.size() == 1)
                m_currentShape.points.append(pos);
            else if (m_currentShape.points.size() >= 2)
                m_currentShape.points[1] = pos;
            m_currentShape.mainPoints = getMainPoints(m_currentShape.points[0],
                                                      m_currentShape.points[1],
                                                      m_isShiftPressed);
        } else if (m_currentShape.type == "pen"
                   || (m_currentShape.type == "effect" && m_currentShape.isOval == 2)) {
            if (!m_currentShape.points.isEmpty()
                    && getDistance(m_currentShape.points.last(), pos) > 2.0) {
                m_currentShape.points.append(pos);
            }
        } else {
            m_currentShape.mainPoints = getMainPoints(m_pos1, pos, m_isShiftPressed);
        }
        m_interactionLastPos = pos;
        m_pressedPoint = pos;
        update();
        emit shapesChanged();
        return;
    }

    if (m_selectedOrder < 0 || m_selectedOrder >= m_shapes.size())
        return;
    if (m_isRotated) {
        handleRotate(pos);
    } else if (m_isResize) {
        handleResize(pos, m_clickedKey);
    } else if (m_isSelected) {
        handleDrag(m_interactionLastPos, pos);
        m_selectedShape = m_shapes[m_selectedOrder];
        m_hoveredShape = m_shapes[m_selectedOrder];
        if (m_selectedShape.type == "text") {
            TextEdit *edit = m_editMap.value(m_selectedIndex);
            if (edit) {
                edit->move(static_cast<int>(m_selectedShape.mainPoints[0].x()),
                           static_cast<int>(m_selectedShape.mainPoints[0].y()));
            }
        }
    }
    m_interactionLastPos = pos;
    m_pressedPoint = pos;
    clampSelectedShapeToCanvas();
    update();
    emit shapesChanged();
}

void ShapesWidget::endInteraction()
{
    if (!m_interactionActive)
        return;
    if (m_isRecording) {
        const QRectF creationRect = QRectF(m_pos1, m_pos2).normalized();
        bool hasContent = creationRect.width() > 1.0 && creationRect.height() > 1.0;
        if (m_currentShape.type == "arrow" || m_currentShape.type == "line")
            hasContent = m_currentShape.points.size() == 2
                    && QLineF(m_currentShape.points[0], m_currentShape.points[1]).length() > 1.0;
        else if (m_currentShape.type == "pen"
                 || (m_currentShape.type == "effect" && m_currentShape.isOval == 2))
            hasContent = m_currentShape.points.size() > 1;

        if (hasContent) {
            m_shapes.append(m_currentShape);
            m_selectedIndex = m_currentShape.index;
            m_selectedOrder = m_shapes.size() - 1;
            m_selectedShape = m_currentShape;
            m_isSelected = true;
        }
        m_currentShape = Toolshape();
        m_isRecording = false;
        m_pos1 = QPointF();
        m_pos2 = QPointF();
    }
    m_interactionActive = false;
    m_interactionOrder = -1;
    m_interactionBackup = Toolshape();
    m_isPressed = false;
    update();
    emit shapesChanged();
}

void ShapesWidget::cancelInteraction()
{
    if (!m_interactionActive)
        return;
    if (m_isRecording) {
        m_currentShape = Toolshape();
        m_isRecording = false;
        m_pos1 = QPointF();
        m_pos2 = QPointF();
    }
    if (m_interactionOrder >= 0 && m_interactionOrder < m_shapes.size()) {
        m_shapes[m_interactionOrder] = m_interactionBackup;
        if (m_selectedOrder == m_interactionOrder)
            m_selectedShape = m_interactionBackup;
    }
    m_interactionActive = false;
    m_interactionOrder = -1;
    m_interactionBackup = Toolshape();
    m_isPressed = false;
    update();
    emit shapesChanged();
}

bool ShapesWidget::takePendingInlineEdit(InlineEditRequest *request)
{
    if (!m_hasPendingInlineEdit) {
        return false;
    }
    if (request) {
        *request = m_pendingInlineEdit;
    }
    m_pendingInlineEdit = InlineEditRequest();
    m_hasPendingInlineEdit = false;
    return true;
}

bool ShapesWidget::beginInlineEditAt(const QPointF &pos, InlineEditRequest *request)
{
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        const Toolshape &shape = m_shapes[i];
        if (shape.mainPoints.size() < 4) {
            continue;
        }
        const QRectF shapeRect(shape.mainPoints[0], shape.mainPoints[3]);
        if (!shapeRect.contains(pos)) {
            continue;
        }

        clickedOnShapes(pos);
        InlineEditRequest result;
        result.shapeIndex = shape.index;
        result.localRect = shapeRect.normalized();
        result.selectAll = true;
        result.isNew = false;
        if (shape.type == "text") {
            result.kind = InlineEditKind::Text;
            result.text = shape.text;
            result.foregroundColor = BaseUtils::colorIndexOf(shape.colorIndex);
            result.fontSize = qMax(1, shape.fontSize);
        } else if (shape.type == "sequence") {
            int sequenceCount = 0;
            for (const Toolshape &item : m_shapes) {
                if (item.type == "sequence") {
                    ++sequenceCount;
                }
            }
            const int sequenceOrder = sequenceOrderForIndex(shape.index);
            if (sequenceOrder < 0)
                continue;
            result.kind = InlineEditKind::Sequence;
            result.text = QString::number(shape.sequenceNumber);
            result.foregroundColor = isLightSequenceColor(shape.colorIndex)
                    ? QColor(Qt::black) : QColor(Qt::white);
            result.fontSize = qMax(8, qRound(shapeRect.height() * 0.54));
            result.minValue = sequenceNumberBeforeOrder(sequenceOrder) + 1;
            result.maxValue = SEQUENCE_MAX_NUMBER - (sequenceCount - 1 - sequenceOrder);
        } else {
            continue;
        }
        if (request) {
            *request = result;
        }
        return true;
    }
    return false;
}

void ShapesWidget::updateInlineText(int shapeIndex, const QString &text, const QRectF &localRect)
{
    const QRectF rect = localRect.normalized();
    for (int i = 0; i < m_shapes.size(); ++i) {
        Toolshape &shape = m_shapes[i];
        if (shape.type != "text" || shape.index != shapeIndex) {
            continue;
        }
        shape.text = text;
        shape.mainPoints[0] = rect.topLeft();
        shape.mainPoints[1] = rect.bottomLeft();
        shape.mainPoints[2] = rect.topRight();
        shape.mainPoints[3] = rect.bottomRight();
        if (m_selectedIndex == shapeIndex) {
            m_selectedShape = shape;
            m_selectedOrder = i;
        }
        update();
        emit shapesChanged();
        return;
    }
}

void ShapesWidget::finishInlineText(int shapeIndex)
{
    for (int i = 0; i < m_shapes.size(); ++i) {
        Toolshape &shape = m_shapes[i];
        if (shape.type != "text" || shape.index != shapeIndex) {
            continue;
        }
        const bool removeShape = shape.text.isEmpty()
                || shape.text == tr("Input text here");
        if (removeShape) {
            m_shapes.removeAt(i);
            if (m_selectedIndex == shapeIndex) {
                clearSelected();
                m_selectedIndex = -1;
                m_selectedOrder = -1;
                m_selectedShape = Toolshape();
            }
        }
        if (m_editMap.contains(shapeIndex)) {
            delete m_editMap.take(shapeIndex);
        }
        update();
        emit shapesChanged();
        if (m_shapes.isEmpty()) {
            emit setShapesUndo(false);
        }
        return;
    }
}

bool ShapesWidget::updateInlineSequence(int shapeIndex, int sequenceNumber)
{
    int sequenceCount = 0;
    for (const Toolshape &shape : m_shapes) {
        if (shape.type != "sequence") {
            continue;
        }
        ++sequenceCount;
    }
    const int sequenceOrder = sequenceOrderForIndex(shapeIndex);
    if (sequenceOrder < 0)
        return false;
    const int minValue = sequenceNumberBeforeOrder(sequenceOrder) + 1;
    const int maxValue = SEQUENCE_MAX_NUMBER - (sequenceCount - 1 - sequenceOrder);
    if (sequenceNumber < minValue || sequenceNumber > maxValue) {
        return false;
    }
    renumberSequenceFrom(sequenceOrder, sequenceNumber);
    update();
    emit shapesChanged();
    return true;
}

void ShapesWidget::setInlineEditingShape(int shapeIndex)
{
    if (m_inlineEditingShape == shapeIndex)
        return;
    m_inlineEditingShape = shapeIndex;
    update();
    emit shapesChanged();
}

QRectF ShapesWidget::contentBounds() const
{
    QRectF bounds;
    for (const Toolshape &shape : m_shapes) {
        const QRectF shapeBounds = toolshapeContentBounds(shape);
        if (shapeBounds.isEmpty())
            continue;
        bounds = bounds.isEmpty() ? shapeBounds : bounds.united(shapeBounds);
    }

    if (m_isRecording) {
        bool hasContent = false;
        if (m_currentShape.type == "arrow" || m_currentShape.type == "line") {
            hasContent = m_currentShape.points.size() == 2
                    && QLineF(m_currentShape.points[0], m_currentShape.points[1]).length() > 1.0;
        } else if (m_currentShape.type == "pen"
                   || (m_currentShape.type == "effect" && m_currentShape.isOval == 2)) {
            hasContent = m_currentShape.points.size() > 1;
        } else {
            hasContent = qAbs(m_pos1.x() - m_pos2.x()) > 1.0
                    || qAbs(m_pos1.y() - m_pos2.y()) > 1.0;
        }

        // A press alone is not committed content. Including the synthetic
        // minimum-size geometry here used to expand the selection merely by
        // clicking a drawing tool, even before the user dragged.
        if (hasContent) {
            const QRectF currentBounds = toolshapeContentBounds(m_currentShape);
            if (!currentBounds.isEmpty())
                bounds = bounds.isEmpty() ? currentBounds : bounds.united(currentBounds);
        }
    }
    return bounds.normalized();
}

void ShapesWidget::clampSelectedShapeToCanvas()
{
    if (m_selectedOrder < 0 || m_selectedOrder >= m_shapes.size())
        return;

    Toolshape &shape = m_shapes[m_selectedOrder];
    const QRectF canvas = rect();
    QRectF bounds = toolshapeContentBounds(shape);
    if (canvas.isEmpty() || bounds.isEmpty() || canvas.contains(bounds))
        return;

    QPointF delta;
    if (bounds.left() < canvas.left())
        delta.setX(canvas.left() - bounds.left());
    else if (bounds.right() > canvas.right())
        delta.setX(canvas.right() - bounds.right());
    if (bounds.top() < canvas.top())
        delta.setY(canvas.top() - bounds.top());
    else if (bounds.bottom() > canvas.bottom())
        delta.setY(canvas.bottom() - bounds.bottom());

    for (QPointF &point : shape.mainPoints)
        point += delta;
    for (QPointF &point : shape.points)
        point += delta;
    if (!shape.arrowRotatePos.isNull())
        shape.arrowRotatePos += delta;
    m_selectedShape = shape;
    m_hoveredShape = shape;
}

void ShapesWidget::handleDrag(QPointF oldPoint, QPointF newPoint)
{
    //qDebug() << "handleDrag:" << m_selectedIndex << m_shapes.length();

    if (m_selectedIndex == -1) {
        return;
    }

    if (m_shapes[m_selectedOrder].type == "arrow" || m_shapes[m_selectedOrder].type == "line") {
        for (int i = 0; i < m_shapes[m_selectedOrder].points.length(); i++) {
            m_shapes[m_selectedOrder].points[i] = QPointF(
                                                      m_shapes[m_selectedOrder].points[i].x() + (newPoint.x() - oldPoint.x()),
                                                      m_shapes[m_selectedOrder].points[i].y() + (newPoint.y() - oldPoint.y())
                                                  );
        }
        return;
    }

    if (m_shapes[m_selectedOrder].mainPoints.length() == 4) {
        for (int i = 0; i < m_shapes[m_selectedOrder].mainPoints.length(); i++) {
            m_shapes[m_selectedOrder].mainPoints[i] = QPointF(
                                                          m_shapes[m_selectedOrder].mainPoints[i].x() + (newPoint.x() - oldPoint.x()),
                                                          m_shapes[m_selectedOrder].mainPoints[i].y() + (newPoint.y() - oldPoint.y())
                                                      );
        }
    }
    for (int i = 0; i < m_shapes[m_selectedOrder].points.length(); i++) {
        m_shapes[m_selectedOrder].points[i] = QPointF(
                                                  m_shapes[m_selectedOrder].points[i].x() + (newPoint.x() - oldPoint.x()),
                                                  m_shapes[m_selectedOrder].points[i].y() + (newPoint.y() - oldPoint.y())
                                              );
    }
}

////////////////////TODO: perfect handleRotate..
void ShapesWidget::handleRotate(QPointF pos)
{
    qCDebug(dsrApp) << "handleRotate called with pos:" << pos;
    //qDebug() << "handleRotate:" << m_selectedIndex << m_shapes.length();

    if (m_selectedIndex == -1 || m_selectedShape.type == "text") {
        qCDebug(dsrApp) << "Rotation skipped: No shape selected or selected shape is text.";
        return;
    }

    if (m_selectedShape.type == "arrow" || m_selectedShape.type == "line") {
        qCDebug(dsrApp) << "Rotating arrow or line type shape.";
        if (m_isArrowRotated == false) {
            qCDebug(dsrApp) << "Arrow not rotated, handling point adjustment.";
            if (m_shapes[m_selectedOrder].isShiftPressed) {
                qCDebug(dsrApp) << "Shift key pressed, adjusting arrow/line to snap to axis.";
                if (static_cast<int>(m_shapes[m_selectedOrder].points[0].x()) == static_cast<int>(m_shapes[m_selectedOrder].points[1].x())) {
                    qCDebug(dsrApp) << "Arrow/line is vertical.";
                    if (m_clickedKey == First) {
                        qCDebug(dsrApp) << "Adjusting first point (vertical).";
                        m_shapes[m_selectedOrder].points[0] = QPointF(m_shapes[m_selectedOrder].points[1].x(),
                                                                      pos.y());
                    } else if (m_clickedKey == Second) {
                        qCDebug(dsrApp) << "Adjusting second point (vertical).";
                        m_shapes[m_selectedOrder].points[1] = QPointF(m_shapes[m_selectedOrder].points[0].x(),
                                                                      pos.y());
                    }
                } else {
                    qCDebug(dsrApp) << "Arrow/line is horizontal.";
                    if (m_clickedKey == First) {
                        qCDebug(dsrApp) << "Adjusting first point (horizontal).";
                        m_shapes[m_selectedOrder].points[0] = QPointF(pos.x(), m_shapes[m_selectedOrder].points[1].y());
                    } else if (m_clickedKey == Second) {
                        m_shapes[m_selectedOrder].points[1] = QPointF(pos.x(), m_shapes[m_selectedOrder].points[0].y());
                    }
                }
            } else {
                if (m_clickedKey == First) {
                    m_shapes[m_selectedOrder].points[0] = m_pressedPoint;
                } else if (m_clickedKey == Second) {
                    m_shapes[m_selectedOrder].points[1] = m_pressedPoint;
                }
            }
        } else {
            QPointF t_rotatepos;

            qreal t_minx = qMin(m_selectedShape.points[1].x(), m_selectedShape.points[0].x());
            qreal t_miny = qMin(m_selectedShape.points[1].y(), m_selectedShape.points[0].y());
            int t_height = static_cast<int>(qAbs(m_selectedShape.points[1].y() - m_selectedShape.points[0].y()));
            int t_width = static_cast<int>(qAbs(m_selectedShape.points[1].x() - m_selectedShape.points[0].x()));

            t_rotatepos.setX(t_minx + t_width / 2);
            t_rotatepos.setY(t_miny + t_height / 2);

            qreal angle = calculateAngle(m_pressedPoint, pos, t_rotatepos) / 35;
            for (int k = 0; k < m_selectedShape.points.length(); k++) {
                m_shapes[m_selectedOrder].points[k] = pointRotate(t_rotatepos,
                                                                  m_selectedShape.points[k], angle);
            }
        }

        m_selectedShape.points  =  m_shapes[m_selectedOrder].points;
        m_hoveredShape.points = m_shapes[m_selectedOrder].points;
        m_pressedPoint = pos;
        return;
    }

    QPointF centerInPoint = QPointF((m_selectedShape.mainPoints[0].x() +
                                     m_selectedShape.mainPoints[3].x()) / 2,
                                    (m_selectedShape.mainPoints[0].y() +
                                     m_selectedShape.mainPoints[3].y()) / 2);
    QLineF linePressed(centerInPoint, m_movingPoint);
    qreal pressedAngle = 360 - linePressed.angle();
    qreal angle;
    if (0 == static_cast<int>(m_lastAngle)) {
        m_lastAngle = pressedAngle;
        angle = 0;
    } else {
        angle = pressedAngle - m_lastAngle;
    }
    if (angle < -100) {
        m_lastAngle = pressedAngle - m_lastAngle + 360;
        angle = m_lastAngle;
    } else if (angle > 100) {
        m_lastAngle = pressedAngle - m_lastAngle - 360;
        angle = m_lastAngle;
    }
    qreal rotationDelta = (angle * M_PI) / 180;
    qreal centerX = (m_shapes[m_selectedOrder].mainPoints[0].x() + m_shapes[m_selectedOrder].mainPoints[3].x()) / 2;
    qreal centerY = (m_shapes[m_selectedOrder].mainPoints[0].y() + m_shapes[m_selectedOrder].mainPoints[3].y()) / 2;
    for (int i = 0; i < m_shapes[m_selectedOrder].mainPoints.size(); ++i) {
        qreal x = centerX + (m_shapes[m_selectedOrder].mainPoints[i].x() - centerX) * cos(rotationDelta) - (m_shapes[m_selectedOrder].mainPoints[i].y() - centerY) * sin(rotationDelta);
        qreal y = centerY + (m_shapes[m_selectedOrder].mainPoints[i].x() - centerX) * sin(rotationDelta) + (m_shapes[m_selectedOrder].mainPoints[i].y() - centerY) * cos(rotationDelta);
        //图形
        m_shapes[m_selectedOrder].mainPoints[i].setX(x);
        m_shapes[m_selectedOrder].mainPoints[i].setY(y);
    }
    qreal centerMainX = (m_selectedShape.mainPoints[0].x() + m_selectedShape.mainPoints[3].x()) / 2;
    qreal centerMainY = (m_selectedShape.mainPoints[0].y() + m_selectedShape.mainPoints[3].y()) / 2;
    for (int k = 0; k < m_selectedShape.points.length(); k++) {
        qreal x = centerMainX + (m_shapes[m_selectedOrder].points[k].x() - centerMainX) * cos(rotationDelta) - (m_shapes[m_selectedOrder].points[k].y() - centerMainY) * sin(rotationDelta);
        qreal y = centerMainY + (m_shapes[m_selectedOrder].points[k].x() - centerMainX) * sin(rotationDelta) + (m_shapes[m_selectedOrder].points[k].y() - centerMainY) * cos(rotationDelta);
        //线条
        m_shapes[m_selectedOrder].points[k].setX(x);
        m_shapes[m_selectedOrder].points[k].setY(y);
    }
    m_lastAngle = pressedAngle;

    m_selectedShape.mainPoints = m_shapes[m_selectedOrder].mainPoints;
    m_hoveredShape.mainPoints =  m_shapes[m_selectedOrder].mainPoints;
    m_pressedPoint = pos;
}

void ShapesWidget::handleResize(QPointF pos, int key)
{
    if (m_isResize && m_selectedIndex != -1) {
        if (m_shapes[m_selectedOrder].portion.isEmpty()) {
            for (int k = 0; k < m_shapes[m_selectedOrder].points.length(); k++) {
                m_shapes[m_selectedOrder].portion.append(relativePosition(
                                                             m_selectedShape.mainPoints, m_selectedShape.points[k]));
            }
        }

        FourPoints newResizeFPoints = resizePointPosition(
                                          m_shapes[m_selectedOrder].mainPoints[0],
                                          m_shapes[m_selectedOrder].mainPoints[1],
                                          m_shapes[m_selectedOrder].mainPoints[2],
                                          m_shapes[m_selectedOrder].mainPoints[3], pos, key,
                                          m_isShiftPressed);

        m_shapes[m_selectedOrder].mainPoints = newResizeFPoints;
        m_selectedShape.mainPoints = newResizeFPoints;
        m_hoveredShape.mainPoints = newResizeFPoints;

        for (int j = 0; j <  m_shapes[m_selectedOrder].portion.length(); j++) {
            m_shapes[m_selectedOrder].points[j] =
                getNewPosition(m_shapes[m_selectedOrder].mainPoints,
                               m_shapes[m_selectedOrder].portion[j]);
        }

        m_selectedShape.points = m_shapes[m_selectedOrder].points;
        m_hoveredShape.points = m_shapes[m_selectedOrder].points;
    }
    m_pressedPoint = pos;
}

bool ShapesWidget::event(QEvent *event)
{
    qCDebug(dsrApp) << "ShapesWidget::event called. Event type:" << event->type();
    if (QEvent::Gesture == event->type()) {
        qCDebug(dsrApp) << "Gesture event detected.";
        if (QGesture *tap = static_cast<QGestureEvent *>(event)->gesture(Qt::TapGesture)) {
            qCDebug(dsrApp) << "TapGesture detected.";
            tapTriggered(static_cast<QTapGesture *>(tap));
        }
        if (QGesture *pinch = static_cast<QGestureEvent *>(event)->gesture(Qt::PinchGesture)) {
            qCDebug(dsrApp) << "PinchGesture detected.";
            pinchTriggered(static_cast<QPinchGesture *>(pinch));
        }
        update();
    }
    qCDebug(dsrApp) << "Calling base QWidget::event.";
    return QWidget::event(event);
}

//重写鼠标按压事件
void ShapesWidget::mousePressEvent(QMouseEvent *e)
{
    qCDebug(dsrApp) << "mousePressEvent called. Mouse position:" << e->pos() << "Button:" << e->button() << "Source:" << e->source();
    m_lastAngle = 0;
    m_currentCursor = QCursor().pos();
    //选中图形后，重新按下真实鼠标左键
    if (Qt::MouseEventSource::MouseEventSynthesizedByQt != e->source()
            && m_selectedIndex != -1) {
        qCDebug(dsrApp) << "Real mouse event detected and a shape is selected.";
        // 判空
        if (nullptr != m_editMap.value(m_lastEditMapKey)) {
            qCDebug(dsrApp) << "Text edit exists at lastEditMapKey.";
            // 点击鼠标左键时，去掉未更改的textEdit文本框
            if (m_editMap.value(m_lastEditMapKey)->toPlainText() == QString(tr("Input text here")) ||
                    m_editMap.value(m_lastEditMapKey)->toPlainText().isEmpty()) {
                qCDebug(dsrApp) << "Text edit is empty or default. Clearing selection and setting read-only.";
                clearSelected();
                setAllTextEditReadOnly();
                m_editing = false;
                m_selectedIndex = -1;
                m_selectedOrder = -1;
                m_selectedShape.type = "";
                update();
                DFrame::mousePressEvent(e);
            }
        }
    }

    //选中图形后，重新按下触摸屏左键
    if (Qt::MouseEventSource::MouseEventSynthesizedByQt == e->source()
            && m_selectedIndex != -1
            && !clickedShapes(e->pos())
            && "text" != m_currentType) {
        clearSelected();
        setAllTextEditReadOnly();
        m_editing = false;
        m_selectedIndex = -1;
        m_selectedOrder = -1;
        m_selectedShape.type = "";
        update();
        DFrame::mousePressEvent(e);

        //return;

    }

    if (m_selectedIndex != -1) {
        if (!(clickedOnShapes(e->pos()) && m_isRotated) && m_selectedIndex == -1 && "text" == m_currentType) {
            clearSelected();
            setAllTextEditReadOnly();
            m_editing = false;
            m_selectedIndex = -1;
            m_selectedOrder = -1;
            m_selectedShape.type = "";
            update();
            DFrame::mousePressEvent(e);
            return;
        }
    }

    //鼠标右键点击
    if (Qt::MouseEventSource::MouseEventSynthesizedByQt != e->source() && e->button() == Qt::RightButton) {
        m_pos1 = QPointF(0, 0); // 修复触控屏绘制矩形后，长按屏幕会出现多余矩形的问题
        qCDebug(dsrApp, ) << "RightButton clicked!" << e->source();
        m_menuController->showMenu(QPoint(mapToGlobal(e->pos())));
        DFrame::mousePressEvent(e);
        return;
    }

    m_pressedPoint = e->pos();
    m_isPressed = true;
    if (!clickedOnShapes(m_pressedPoint)) {
        // 普通模式下只处理已有标注的选择/编辑，不允许继续创建图形。
        if (m_currentType.isEmpty()) {
            clearSelected();
            setAllTextEditReadOnly();
            m_editing = false;
            m_isRecording = false;
            update();
            DFrame::mousePressEvent(e);
            return;
        }

        // AI助手不是绘图工具，不进入绘制逻辑
        if (m_currentType == "aiassistant") {
            DFrame::mousePressEvent(e);
            return;
        }

        // 开始新的绘制前必须先清掉旧的选中态，否则 mouseReleaseEvent 会因
        // m_isSelected 仍为 true 而跳过落盘，并且旧标注的拖动锚点会污染新图形。
        clearSelected();
        setAllTextEditReadOnly();
        m_editing = false;
        m_selectedShape = Toolshape();
        m_hoveredShape = Toolshape();
        m_currentShape = Toolshape();

        m_isRecording = true;
        //qDebug() << "no one shape be clicked!" << m_selectedIndex << m_shapes.length();

        m_currentShape.type = m_currentType;
        m_currentShape.colorIndex = ConfigSettings::instance()->getValue(m_currentType, "color_index").toInt();
        m_currentShape.lineWidth = configuredLineWidth(m_currentType);

        m_selectedIndex = -1;
        m_shapesIndex += 1;
        m_currentIndex = m_shapesIndex;

        if (m_pos1 == QPointF(0, 0)) {
            m_pos1 = e->pos();
            if (m_currentType == "pen") {
                m_currentShape.index = m_currentIndex;
                m_currentShape.points.append(m_pos1);
            } else if (m_currentType == "arrow") {
                m_currentShape.index = m_currentIndex;
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.points.append(m_pos1);
                m_currentShape.lineWidth = configuredLineWidth("arrow");
            } else if (m_currentType == "line") {
                m_currentShape.index = m_currentIndex;
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.points.append(m_pos1);
                m_currentShape.lineWidth = configuredLineWidth("line");
            } else if (m_currentType == "effect") {
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.index = m_currentIndex;
                m_currentShape.isBlur = ConfigSettings::instance()->getValue("effect", "isBlur").toBool();
                m_currentShape.isOval = ConfigSettings::instance()->getValue("effect", "isOval").toInt();
                m_currentShape.radius = ConfigSettings::instance()->getValue("effect", "radius").toInt() * 3 + 10;
                m_currentShape.lineWidth = configuredLineWidth("effect");
                if (m_currentShape.isBlur) {
                    emit reloadEffectImg("blur", m_currentShape.radius);
                } else {
                    emit reloadEffectImg("mosaic", m_currentShape.radius);
                }
                if (m_currentShape.isOval == 2) {
                    m_currentShape.points.append(m_pos1);
                }
            } else if (m_currentType == "rectangle") {
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.index = m_currentIndex;
            } else if (m_currentType == "oval") {
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.index = m_currentIndex;
            } else if (m_currentType == "sequence") {
                // 序号标注：点击即放置，不进入拖拽绘制
                int seqNum = sequenceNextNumber();
                if (seqNum > SEQUENCE_MAX_NUMBER) {
                    // 已达最大编号：角标跟随光标，气泡提示弹出 2 秒后自动消失，忽略本次放置
                    updateSequencePreview(e->pos());
                    showSequenceLimitTips(e->pos());
                    m_isRecording = false;
                    m_pos1 = QPointF(0, 0);
                    DFrame::mousePressEvent(e);
                    return;
                }
                int tier = qBound(0, ConfigSettings::instance()->getValue("sequence", "line_width").toInt(), 2);
                // 序号圆圈直径由档位经 SEQUENCE_DIAMETER 编码进 mainPoints；
                // m_currentShape.lineWidth（由公共放置路径写入）仅供配置记忆，渲染不使用。
                qreal diameter = SEQUENCE_DIAMETER(tier);
                qreal half = diameter / 2.0;
                QPointF center = e->pos();
                m_currentShape.index = m_currentIndex;
                m_currentShape.sequenceNumber = seqNum;
                m_currentShape.mainPoints[0] = QPointF(center.x() - half, center.y() - half);
                m_currentShape.mainPoints[1] = QPointF(center.x() - half, center.y() + half);
                m_currentShape.mainPoints[2] = QPointF(center.x() + half, center.y() - half);
                m_currentShape.mainPoints[3] = QPointF(center.x() + half, center.y() + half);
                m_shapes.append(m_currentShape);
                // 点击放置后不进入拖拽，重置绘制状态
                m_isRecording = false;
                m_pos1 = QPointF(0, 0);
                for (int k = 0; k < m_currentShape.mainPoints.length(); k++) {
                    m_currentShape.mainPoints[k] = QPointF(0, 0);
                }
                m_currentShape.type = "";
                updateSequencePreview(e->pos());
            } else if (m_currentType == "text") {
                if (!m_editing) {
                    setAllTextEditReadOnly();
                    setNoChangedTextEditRemove();
                    m_currentShape.mainPoints[0] = m_pos1;
                    m_currentShape.index = m_currentIndex;
                    qCDebug(dsrApp, ) << "new textedit:" << m_currentIndex;
                    TextEdit *edit = new TextEdit(m_currentIndex, this);
                    QString t_editText = QString(tr("Input text here"));
                    edit->appendPlainText(t_editText);
//                    edit->setPlainText(t_editText);
                    m_editing = true;
                    edit->setEditing(true);
                    int defaultFontSize = ConfigSettings::instance()->getValue("text", "fontsize").toInt();
                    m_currentShape.fontSize = defaultFontSize;


                    m_currentShape.mainPoints[0] = m_pos1;
                    m_currentShape.mainPoints[1] = QPointF(m_pos1.x(), m_pos1.y() + edit->height());
                    m_currentShape.mainPoints[2] = QPointF(m_pos1.x() + edit->width(), m_pos1.y());
                    m_currentShape.mainPoints[3] = QPointF(m_pos1.x() + edit->width(),
                                                           m_pos1.y() + edit->height());
                    m_editMap.insert(m_currentIndex, edit);
                    m_selectedIndex = m_currentIndex;
                    m_selectedShape = m_currentShape;
                    m_lastEditMapKey = m_currentIndex;


                    connect(edit, &TextEdit::repaintTextRect, this, &ShapesWidget::updateTextRect);
                    connect(edit, &TextEdit::clickToEditing, this, [ = ](int index) {
                        //                        setAllTextEditReadOnly();
                        for (int k = 0; k < m_shapes.length(); k++) {
                            if (m_shapes[k].type == "text" && m_shapes[k].index == index) {
                                m_selectedIndex = index;
                                m_selectedShape = m_shapes[k];
                                m_selectedOrder = k;
                                m_currentShape = m_selectedShape;
                                break;
                            }
                        }
                        QMap<int, TextEdit *>::iterator i = m_editMap.begin();
                        while (i != m_editMap.end()) {
                            if (i.key() != index) {
                                i.value()->setReadOnly(true);
                                i.value()->setEditing(false);
                            } else {
                                i.value()->setEditing(true);
                            }
                            QTextCursor textCursor =  i.value()->textCursor();
                            textCursor.clearSelection();
                            i.value()->setTextCursor(textCursor);
                            ++i;
                        }
                        m_editing = true;
                        m_isSelectedText = false;
                        emit shapeClicked("text");

                    });

                    connect(edit, &TextEdit::textEditSelected, this, [ = ](int index) {
                        //                        setAllTextEditReadOnly();
                        if (m_selectedIndex != index) {
                            m_editing = false;
                            m_currentShape.type = "";
                            for (int i = 0; i < m_currentShape.mainPoints.length(); i++) {
                                m_currentShape.mainPoints[i] = QPointF(0, 0);
                            }
                        }
                        for (int k = 0; k < m_shapes.length(); k++) {
                            if (m_shapes[k].type == "text" && m_shapes[k].index == index) {
                                m_selectedIndex = index;
                                m_selectedShape = m_shapes[k];
                                m_selectedOrder = k;
                                break;
                            }
                        }
                        QMap<int, TextEdit *>::iterator i = m_editMap.begin();
                        while (i != m_editMap.end()) {
                            if (i.key() != index) {
                                i.value()->setReadOnly(true);
                                i.value()->setEditing(false);
                            }

                            QTextCursor textCursor =  i.value()->textCursor();
                            textCursor.clearSelection();
                            i.value()->setTextCursor(textCursor);
                            ++i;
                        }
                        m_isSelectedText = true;
                        emit shapeClicked("text");
                    });

                    connect(edit, &TextEdit::textEditFinish, this, [ = ](int index) {
                        //                        setAllTextEditReadOnly();
                        Q_UNUSED(index);
                        setAllTextEditReadOnly();
                    });

                    edit->setSelecting(true);
                    edit->setFocus();
                    edit->move(static_cast<int>(m_pos1.x()), static_cast<int>(m_pos1.y()));
                    edit->show();
                    QTextCursor cs = edit->textCursor();
                    edit->moveCursor(QTextCursor::Start);
                    cs.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, t_editText.length());
                    edit->setTextCursor(cs);
                    edit->selectAll();
                    m_shapes.append(m_currentShape);


                    for (int k = 0; k < m_shapes.length(); k++) {
                        if (m_shapes[k].type == "text" && m_shapes[k].index == m_currentIndex) {
                            m_selectedOrder = k;
                            break;
                        }
                    }

                    qCDebug(dsrApp, ) << "Insert text shape:" << m_shapes.size() << m_currentShape.type << m_currentShape.index;
                } else {
                    m_editing = false;
                    setAllTextEditReadOnly();

                }
            }
            update();
        }
    } else {
        m_isRecording = false;
        //qDebug() << "some on shape be clicked!";
        if (m_editing && m_editMap.contains(m_shapes[m_selectedOrder].index)) {
            m_editMap.value(m_shapes[m_selectedOrder].index)->setReadOnly(true);
            m_editMap.value(m_shapes[m_selectedOrder].index)->setCursorVisible(false);
            m_editMap.value(m_shapes[m_selectedOrder].index)->setFocusPolicy(Qt::NoFocus);
            qCDebug(dsrApp) << "Editing and editMap contains selected shape index. Setting text edit read-only.";
        }
        update();
    }
    DFrame::mousePressEvent(e);

}

void ShapesWidget::mouseReleaseEvent(QMouseEvent *e)
{
    qCDebug(dsrApp) << "mouseReleaseEvent called. Mouse position:" << e->pos();
    m_isPressed = false;
    m_isMoving = false;

    if (Qt::MouseEventSource::MouseEventSynthesizedByQt == e->source()
            && -1 != m_selectedIndex
            && "text" != m_currentType) {
        qCDebug(dsrApp) << "Synthesized mouse event, selected shape exists, and not text type. Returning.";
        return;
    }

    if (!m_isRotated && m_selectedIndex == -1 && "text" != m_currentType) {
        qCDebug(dsrApp) << "Not rotated, no shape selected, and not text type. Clearing selection.";
        clearSelected();
        setAllTextEditReadOnly();
        m_editing = false;
        m_selectedIndex = -1;
        m_selectedOrder = -1;
        m_selectedShape.type = "";
        update();
        DFrame::mouseReleaseEvent(e);
    }

    //qDebug() << m_isRecording << m_isSelected << m_pos2;
    if (m_isRecording && !m_isSelected && m_pos2 != QPointF(0, 0)) {
        qCDebug(dsrApp) << "Recording active, no shape selected, and pos2 is not (0,0). Finalizing shape.";
        if (m_currentType == "arrow" || m_currentType == "line") {
            qCDebug(dsrApp) << "Finalizing arrow or line.";
            if (m_currentShape.points.length() == 2) {
                qCDebug(dsrApp) << "Arrow/line has 2 points.";
                if (m_isShiftPressed) {
                    qCDebug(dsrApp) << "Shift pressed, snapping arrow/line to axis.";
                    if (std::atan2(std::abs(m_pos2.y() - m_pos1.y()), std::abs(m_pos2.x() - m_pos1.x()))
                            * 180 / M_PI < 45) {
                        m_pos2 = QPointF(m_pos2.x(), m_pos1.y());
                        qCDebug(dsrApp) << "Snapped to horizontal.";
                    } else {
                        m_pos2 = QPointF(m_pos1.x(), m_pos2.y());
                        qCDebug(dsrApp) << "Snapped to vertical.";
                    }
                }

                m_currentShape.points[1] = m_pos2;
                m_currentShape.mainPoints = getMainPoints(m_currentShape.points[0], m_currentShape.points[1]);
                qCDebug(dsrApp) << "Set second point and main points for arrow/line.";

                m_shapes.append(m_currentShape);
            }
        } else if (m_currentType == "pen") {
            qCDebug(dsrApp) << "Finalizing pen shape.";
            FourPoints lineFPoints = fourPointsOfLine(m_currentShape.points);
            m_currentShape.mainPoints = lineFPoints;
            m_shapes.append(m_currentShape);
            qCDebug(dsrApp) << "Appended current pen shape to shapes list.";
        } else if (m_currentType != "text") {
            qCDebug(dsrApp) << "Finalizing non-text shape (rectangle/oval/effect).";
            FourPoints rectFPoints = getMainPoints(m_pos1, m_pos2, m_isShiftPressed);
            m_currentShape.mainPoints = rectFPoints;
            m_shapes.append(m_currentShape);
            qCDebug(dsrApp) << "Appended current shape to shapes list.";
        }

        //qDebug() << "ShapesWidget num:" << m_shapes.length();
        clearSelected();
        //选中当前绘制的图形
        if (m_currentShape.type != "effect" || m_currentShape.isOval != 2) {
            qCDebug(dsrApp) << "Selecting newly drawn shape.";
            m_selectedIndex = m_currentIndex;
            m_selectedShape = m_currentShape;
            m_selectedOrder = m_shapes.length() - 1;
            m_isSelected = true;
        }
    }

    m_isRecording = false;
    if (m_currentShape.type != "text") {
        qCDebug(dsrApp) << "Clearing main points of current shape (if not text).";
        for (int i = 0; i < m_currentShape.mainPoints.length(); i++) {
            m_currentShape.mainPoints[i] = QPointF(0, 0);
        }
    }

    m_currentShape.points.clear();
    m_pos1 = QPointF(0, 0);
    m_pos2 = QPointF(0, 0);

    update();
    emit shapesChanged();
    DFrame::mouseReleaseEvent(e);
    qCDebug(dsrApp) << "mouseReleaseEvent finished.";
}

void ShapesWidget::mouseMoveEvent(QMouseEvent *e)
{
    qCDebug(dsrApp) << "mouseMoveEvent called. Mouse position:" << e->pos();
    m_isMoving = true;
    m_movingPoint = e->pos();
    m_currentCursor = QCursor().pos();

    if (m_isRecording && m_isPressed) {
        qCDebug(dsrApp) << "Recording is active and mouse is pressed.";
        m_pos2 = e->pos();
        updateCursorShape();

        if (m_currentShape.type == "arrow" || m_currentShape.type == "line") {
            qCDebug(dsrApp) << "Moving arrow or line shape.";
            if (m_currentShape.points.length() <= 1) {
                qCDebug(dsrApp) << "Adding second point to arrow/line.";
                if (m_isShiftPressed) {
                    qCDebug(dsrApp) << "Shift pressed, snapping arrow/line to axis.";
                    if (std::atan2(std::abs(m_pos2.y() - m_pos1.y()),
                                   std::abs(m_pos2.x() - m_pos1.x())) * 180 / M_PI < 45) {
                        m_currentShape.points.append(QPointF(m_pos2.x(), m_pos1.y()));
                        qCDebug(dsrApp) << "Snapped to horizontal.";
                    } else {
                        m_currentShape.points.append(QPointF(m_pos1.x(), m_pos2.y()));
                        qCDebug(dsrApp) << "Snapped to vertical.";
                    }
                } else {
                    m_currentShape.points.append(m_pos2);
                    qCDebug(dsrApp) << "Appended pos2 to arrow/line points.";
                }
            } else {
                qCDebug(dsrApp) << "Updating second point of arrow/line.";
                if (m_isShiftPressed) {
                    qCDebug(dsrApp) << "Shift pressed, snapping arrow/line to axis.";
                    if (std::atan2(std::abs(m_pos2.y() - m_pos1.y()),
                                   std::abs(m_pos2.x() - m_pos1.x())) * 180 / M_PI < 45) {
                        m_currentShape.points[1] = QPointF(m_pos2.x(), m_pos1.y());
                        qCDebug(dsrApp) << "Snapped to horizontal.";
                    } else {
                        m_currentShape.points[1] = QPointF(m_pos1.x(), m_pos2.y());
                        qCDebug(dsrApp) << "Snapped to vertical.";
                    }
                } else {
                    m_currentShape.points[1] = m_pos2;
                    qCDebug(dsrApp) << "Appended pos2 to arrow/line points.";
                }
            }
        }
        if (m_currentShape.type == "pen") {
            if (getDistance(m_currentShape.points[m_currentShape.points.length() - 1], m_pos2) > 3) {
                m_currentShape.points.append(m_pos2);
                qCDebug(dsrApp) << "Pen type: distance > 3, appending pos2.";
            }
        }
        // 模糊笔
        if (m_currentShape.type == "effect" && m_currentShape.isOval == 2) {
            double distance = getDistance(m_currentShape.points[m_currentShape.points.length() - 1], m_pos2);
            if (distance > 14) {
                QList<QPointF> interpolationPoints = getInterpolationPoints(m_currentShape.points[m_currentShape.points.length() - 1], m_pos2,
                                                                            (distance / 7));
                for (int i = 0; i < interpolationPoints.size(); ++i) {
                    m_currentShape.points.append(interpolationPoints[i]);
                }
                m_currentShape.points.append(m_pos2);
                qCDebug(dsrApp) << "Effect type: distance > 14, appending interpolated points and pos2.";
            } else if (distance > 7) {
                m_currentShape.points.append(m_pos2);
                qCDebug(dsrApp) << "Effect type: distance > 7, appending pos2.";
            }
        }
        update();
    } else if (!m_isRecording && m_isPressed) {
        if (m_isRotated && m_isPressed) {
            handleRotate(e->pos());
            update();
            qCDebug(dsrApp) << "Not recording but pressed and rotated, calling handleRotate.";
        }

        if (m_isResize && m_isPressed) {
            // resize function
            handleResize(QPointF(e->pos()), m_clickedKey);
            update();
            DFrame::mouseMoveEvent(e);
            qCDebug(dsrApp) << "Not recording but pressed and resizing, calling handleResize.";
            return;
        }

        if (m_isSelected && m_isPressed && m_selectedIndex != -1) {
            handleDrag(m_pressedPoint, m_movingPoint);
            //qDebug() << "move m_selectedOrder:" << m_selectedOrder;
            m_selectedShape = m_shapes[m_selectedOrder];
            m_hoveredShape = m_shapes[m_selectedOrder];

            if (m_selectedShape.type == "text") {
                m_editMap.value(m_selectedIndex)->move(static_cast<int>(m_selectedShape.mainPoints[0].x()),
                                                       static_cast<int>(m_selectedShape.mainPoints[0].y()));
                qCDebug(dsrApp) << "Selected shape is text, moving text edit.";
            }

            m_pressedPoint = m_movingPoint;
            update();
            qCDebug(dsrApp) << "Selected, pressed, and index not -1, calling handleDrag.";
        }

    } else {
        qCDebug(dsrApp) << "Not recording or not pressed.";
        if (!m_isRecording) {
            m_isHovered = false;
            for (int i = 0; i < m_shapes.length(); i++) {
                m_hoveredIndex = m_shapes[i].index;

                if (hoverOnShapes(m_shapes[i],  e->pos())) {
                    m_isHovered = true;
                    m_hoveredShape = m_shapes[i];
                    //悬停状态时，根据悬停的位置不同，光标的形状也不同
                    if (m_resizeDirection == Left) {
                        if (m_isSelected || m_isRotated) {
                            applyOverrideCursor(Qt::SizeHorCursor);
                        } else {
                            applyOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == Top) {
                        if (m_isSelected || m_isRotated) {
                            applyOverrideCursor(Qt::SizeVerCursor);
                        } else {
                            applyOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == Right) {
                        if (m_isSelected || m_isRotated) {
                            applyOverrideCursor(Qt::SizeHorCursor);
                        } else {
                            applyOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == Bottom) {
                        if (m_isSelected || m_isRotated) {
                            applyOverrideCursor(Qt::SizeVerCursor);
                        } else {
                            applyOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == TopLeft) {
                        if (m_isSelected || m_isRotated) {
                            applyOverrideCursor(Qt::SizeFDiagCursor);
                        } else {
                            applyOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == BottomLeft) {
                        if (m_isSelected || m_isRotated) {
                            applyOverrideCursor(Qt::SizeBDiagCursor);
                        } else {
                            applyOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == TopRight) {
                        if (m_isSelected || m_isRotated) {
                            applyOverrideCursor(Qt::SizeBDiagCursor);
                        } else {
                            applyOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == BottomRight) {
                        if (m_isSelected || m_isRotated) {
                            applyOverrideCursor(Qt::SizeFDiagCursor);
                        } else {
                            applyOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == Rotate) {
                        applyOverrideCursor(BaseUtils::setCursorShape("rotate"));
                    } else if (m_resizeDirection == Moving) {
                        applyOverrideCursor(Qt::ClosedHandCursor);
                    } else {
                        updateCursorShape();
                    }
                    //update();
                    break;
                } else {
                    updateCursorShape();
                    //update();
                }
            }
            if (!m_isHovered) {
                for (int j = 0; j < m_hoveredShape.mainPoints.length(); j++) {
                    m_hoveredShape.mainPoints[j] = QPointF(0, 0);
                }
                m_hoveredShape.type = "";
                //update();
            }
            if (m_shapes.length() == 0) {
                updateCursorShape();
            }
        } else {
            //TODO text
        }
    }
    if (m_currentType == "sequence") {
        updateSequencePreview(e->pos());
    }
    update();
    DFrame::mouseMoveEvent(e);
}

bool ShapesWidget::hasSelectedShape() const
{
    return m_selectedIndex != -1 && m_selectedOrder >= 0
            && m_selectedOrder < static_cast<int>(m_shapes.size());
}

bool ShapesWidget::isPressed() const
{
    return m_isPressed;
}

void ShapesWidget::translateAllShapes(const QPointF &delta)
{
    if (delta.isNull()) {
        return;
    }
    auto shiftPoints = [&delta](QList<QPointF> &points) {
        for (QPointF &point : points) {
            point += delta;
        }
    };
    auto hasGeometry = [](const Toolshape &shape) {
        for (const QPointF &point : shape.mainPoints) {
            if (!point.isNull()) {
                return true;
            }
        }
        return !shape.points.isEmpty() || !shape.arrowRotatePos.isNull();
    };
    auto translateShape = [&delta, &shiftPoints, &hasGeometry](Toolshape &shape) {
        // 空 Toolshape 的 mainPoints 全是 (0,0)，不能直接整体加偏移；
        // 否则会把“未绘制”误变成有效几何，后续绘制和拖动都会出现幽灵图形。
        if (!hasGeometry(shape)) {
            return;
        }
        shiftPoints(shape.mainPoints);
        shiftPoints(shape.points);
        if (!shape.arrowRotatePos.isNull()) {
            shape.arrowRotatePos += delta;
        }
    };
    for (Toolshape &shape : m_shapes) {
        translateShape(shape);
    }
    for (Toolshape *shape : {&m_currentShape, &m_selectedShape, &m_hoveredShape}) {
        translateShape(*shape);
    }
    for (TextEdit *edit : m_editMap) {
        if (edit) {
            edit->move(edit->pos() + delta.toPoint());
        }
    }

    // ShapesWidget 的原点移动后，鼠标锚点也必须同步平移，否则下一次
    // mouseMoveEvent 会用新旧两个坐标系计算增量，表现为标注跳跃。
    if (!m_pos1.isNull()) {
        m_pos1 += delta;
    }
    if (!m_pos2.isNull()) {
        m_pos2 += delta;
    }
    if (!m_pressedPoint.isNull()) {
        m_pressedPoint += delta;
    }
    if (!m_movingPoint.isNull()) {
        m_movingPoint += delta;
    }
    update();
    emit shapesChanged();
}

//鼠标选中图形时，可以通过w/a/s/d及小键盘方向键控制光标移动
void ShapesWidget::keyPressEvent(QKeyEvent *keyEvent)
{
    if (m_isPressed) {
        Utils::cursorMove(m_currentCursor, keyEvent);
    }
    
    // 处理删除键
    if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace) {
        deleteCurrentShape();
        return;
    }
    
    // 处理选中图形后的键盘调整
    if (m_selectedIndex == -1 || m_selectedOrder >= m_shapes.length()) {
        qCDebug(dsrApp, ) << "ShapeWidget Exist keyEvent:" << keyEvent->text();
        return;
    }
    

    static const QHash<int, QString> modifierMap = {
        {Qt::ShiftModifier | Qt::ControlModifier, "Ctrl+Shift"},
        {Qt::ControlModifier, "Ctrl"},
        {Qt::NoModifier, ""}
    };
    
    QString direction;
    int modifiers = keyEvent->modifiers();
    
    if (modifierMap.contains(modifiers)) {
        QString prefix = modifierMap[modifiers];
        
        switch (keyEvent->key()) {
            case Qt::Key_Left:
            case Qt::Key_A:
                direction = prefix.isEmpty() ? Direction::LEFT : prefix + "+" + Direction::LEFT;
                break;
            case Qt::Key_Right:
            case Qt::Key_D:
                direction = prefix.isEmpty() ? Direction::RIGHT : prefix + "+" + Direction::RIGHT;
                break;
            case Qt::Key_Up:
            case Qt::Key_W:
                direction = prefix.isEmpty() ? Direction::UP : prefix + "+" + Direction::UP;
                break;
            case Qt::Key_Down:
            case Qt::Key_S:
                direction = prefix.isEmpty() ? Direction::DOWN : prefix + "+" + Direction::DOWN;
                break;
            default:
                break;
        }
    }
    
    // 如果找到了有效的方向，调用microAdjust
    if (!direction.isEmpty()) {
        microAdjust(direction);
    } else {
        qCDebug(dsrApp, ) << "ShapeWidget Exist keyEvent:" << keyEvent->text();
    }
}

void ShapesWidget::updateTextRect(TextEdit *edit, QRectF newRect)
{
    int index = edit->getIndex();
    //    qDebug() << "updateTextRect:" << newRect << index;
    for (int j = 0; j < m_shapes.length(); j++) {
        //        qDebug() << "updateTextRect  updating:" << j << m_shapes[j].index << index;
        if (m_shapes[j].type == "text" && m_shapes[j].index == index) {
            m_shapes[j].mainPoints[0] = QPointF(newRect.x(), newRect.y());
            m_shapes[j].mainPoints[1] = QPointF(newRect.x(), newRect.y() + newRect.height());
            m_shapes[j].mainPoints[2] = QPointF(newRect.x() + newRect.width(), newRect.y());
            m_shapes[j].mainPoints[3] = QPointF(newRect.x() + newRect.width(),
                                                newRect.y() + newRect.height());
            m_currentShape = m_shapes[j];
            m_selectedShape = m_shapes[j];
            m_selectedIndex = m_shapes[j].index;
            m_selectedOrder = j;
        }
    }
    update();
    emit shapesChanged();
}

void ShapesWidget::paintImgPoint(QPainter &painter, QPointF pos, QPixmap img, bool isResize) const
{
    if (isResize) {
        painter.drawPixmap(QPointF(pos.x() - DRAG_BOUND_RADIUS,
                                   pos.y() - DRAG_BOUND_RADIUS), img);
    } else {
        painter.drawPixmap(QPointF(pos.x() - 17,
                                   pos.y() - 10), img);
    }
}

void ShapesWidget::paintRect(QPainter &painter, FourPoints rectFPoints, int index,
                             ShapeBlurStatus rectStatus, bool isBlur, bool isMosaic, int radius) const
{
    QPainterPath rectPath;
    if (rectStatus  == Hovered || ((isBlur || isMosaic) && index == m_selectedIndex)) {
        painter.setPen(QColor("#01bdff"));
    } else if (rectStatus == Drawing || ((isBlur | isMosaic) && index != m_selectedIndex)) {
        painter.setPen(Qt::transparent);
    }

    rectPath.moveTo(rectFPoints[0].x(), rectFPoints[0].y());
    rectPath.lineTo(rectFPoints[1].x(), rectFPoints[1].y());
    rectPath.lineTo(rectFPoints[3].x(), rectFPoints[3].y());
    rectPath.lineTo(rectFPoints[2].x(), rectFPoints[2].y());
    rectPath.lineTo(rectFPoints[0].x(), rectFPoints[0].y());
    painter.drawPath(rectPath);

    //    using namespace utils;
    if (isBlur) {
        painter.setClipPath(rectPath);
        painter.drawPixmap(0, 0, width(), height(), effectPixmap(true, radius));
        painter.drawPath(rectPath);
    }
    if (isMosaic) {
        painter.setClipPath(rectPath);
        painter.drawPixmap(0, 0, width(), height(), effectPixmap(false, radius));
        painter.drawPath(rectPath);
    }
    painter.setClipping(false);
}

void ShapesWidget::paintEllipse(QPainter &painter, FourPoints ellipseFPoints, int index,
                                ShapeBlurStatus  ovalStatus, bool isBlur, bool isMosaic, int radius) const
{
    if (ovalStatus  == Hovered || ((isBlur || isMosaic) && index == m_selectedIndex)) {
        painter.setPen(QColor("#01bdff"));
    } else if (ovalStatus == Drawing || ((isBlur | isMosaic) && index != m_selectedIndex)) {
        painter.setPen(Qt::transparent);
    }

    FourPoints minorPoints = getAnotherFPoints(ellipseFPoints);
    QList<QPointF> eightControlPoints = getEightControlPoint(ellipseFPoints);
    QPainterPath ellipsePath;
    QPainterPath rectPath;
    //    qDebug() << "here" << ellipseFPoints[0].y() - ellipseFPoints[2].y();

    if (qAbs(ellipseFPoints[0].y() - ellipseFPoints[2].y()) <= 0.1
            && qAbs(ellipseFPoints[1].y() - ellipseFPoints[3].y()) <= 0.1) {
        QRect t_rect;
        t_rect.setX(static_cast<int>(ellipseFPoints[0].x()));
        t_rect.setY(static_cast<int>(ellipseFPoints[0].y()));
        t_rect.setWidth(static_cast<int>(ellipseFPoints[3].x() - ellipseFPoints[0].x()));
        t_rect.setHeight(static_cast<int>(ellipseFPoints[3].y() - ellipseFPoints[0].y()));

        ellipsePath.addEllipse(t_rect);
        painter.drawPath(ellipsePath);
    }

    else {
        ellipsePath.moveTo(minorPoints[0].x(), minorPoints[0].y());
        ellipsePath.cubicTo(eightControlPoints[0], eightControlPoints[1], minorPoints[1]);
        ellipsePath.cubicTo(eightControlPoints[4], eightControlPoints[5], minorPoints[2]);
        ellipsePath.cubicTo(eightControlPoints[6], eightControlPoints[7], minorPoints[3]);
        ellipsePath.cubicTo(eightControlPoints[3], eightControlPoints[2], minorPoints[0]);
        painter.drawPath(ellipsePath);
    }


    //    using namespace utils;
    if (isBlur) {
        painter.setClipPath(ellipsePath);
        painter.drawPixmap(0, 0, width(), height(), effectPixmap(true, radius));
        painter.drawPath(ellipsePath);
    }
    if (isMosaic) {
        painter.setClipPath(ellipsePath);
        painter.drawPixmap(0, 0, width(), height(), effectPixmap(false, radius));
        painter.drawPath(ellipsePath);
    }
    painter.setClipping(false);
}

QPixmap ShapesWidget::effectPixmap(bool blur, int radius) const
{
    if (m_effectSourceImage.isNull()) {
        return blur ? TempFile::instance()->getBlurPixmap(radius)
                    : TempFile::instance()->getMosaicPixmap(radius);
    }

    QHash<int, QPixmap> &cache = blur ? m_blurEffectPixmaps
                                      : m_mosaicEffectPixmaps;
    const auto cached = cache.constFind(radius);
    if (cached != cache.cend())
        return cached.value();

    const QPixmap pixmap = QPixmap::fromImage(
            effectAnnotationImage(m_effectSourceImage, blur, radius));
    cache.insert(radius, pixmap);
    return pixmap;
}

void ShapesWidget::paintArrow(QPainter &painter, QList<QPointF> lineFPoints,
                              int lineWidth, bool isStraight) const
{
    if (lineFPoints.length() == 2) {
        if (!isStraight) {
            QList<QPointF> arrowPoints = pointOfArrow(lineFPoints[0],
                                                      lineFPoints[1], 8 + (lineWidth - 1) * 2);
            QPainterPath path;
            const QPen oldPen = painter.pen();
            if (arrowPoints.length() >= 3) {
                painter.drawLine(lineFPoints[0], lineFPoints[1]);
                path.moveTo(arrowPoints[2].x(), arrowPoints[2].y());
                path.lineTo(arrowPoints[0].x(), arrowPoints[0].y());
                path.lineTo(arrowPoints[1].x(), arrowPoints[1].y());
                path.lineTo(arrowPoints[2].x(), arrowPoints[2].y());
            }
            painter.setPen(Qt :: NoPen);
            painter.fillPath(path, QBrush(oldPen.color()));
        } else {
            painter.drawLine(lineFPoints[0], lineFPoints[1]);
        }
    }
}

void ShapesWidget::paintLine(QPainter &painter, QList<QPointF> lineFPoints) const
{
    QPainterPath linePaths;
    if (lineFPoints.length() >= 1)
        linePaths.moveTo(lineFPoints[0]);
    else
        return;

    for (int k = 1; k < lineFPoints.length() - 1; k++) {
        linePaths.quadTo(lineFPoints[k], lineFPoints[k + 1]);
    }
    painter.drawPath(linePaths);
}

void ShapesWidget::paintEffectLine(QPainter &painter, QList<QPointF> lineFPoints, bool isMosaic, int radius, int lineWidth) const
{
    QPainterPath linePaths;
    if (lineFPoints.length() >= 1)
        linePaths.moveTo(lineFPoints[0]);
    else
        return;

    for (int k = 1; k < lineFPoints.length() - 1; k++) {
        FourPoints rectFoints = getRectPoints(lineFPoints[k - 1], lineFPoints[k + 1], lineWidth);
        paintRect(painter, rectFoints, isTreeLandMode() ? -1 : 0, Drawing,
                  isMosaic, !isMosaic, radius);
        //paintRect(painter, rectFoints, 0, Hovered, false, false, radius);
    }
}

void ShapesWidget::paintText(QPainter &painter, FourPoints rectFPoints) const
{
    QPen textPen;
    textPen.setStyle(Qt::DashLine);
    textPen.setColor("#01bdff");
    painter.setPen(textPen);

    if (rectFPoints.length() >= 4) {
        painter.drawLine(rectFPoints[0], rectFPoints[1]);
        painter.drawLine(rectFPoints[1], rectFPoints[3]);
        painter.drawLine(rectFPoints[3], rectFPoints[2]);
        painter.drawLine(rectFPoints[2], rectFPoints[0]);
    }
}

void ShapesWidget::paintText(QPainter &painter, FourPoints rectFPoints, const QString &text, int fontsize) const
{
    qCDebug(dsrApp, ) << "fontsize: " << fontsize;
    QFont font = painter.font() ;
    font.setPixelSize(fontsize);
    painter.setFont(font);
    QRect rect(static_cast<int>(rectFPoints[0].x()),
               static_cast<int>(rectFPoints[0].y()),
               static_cast<int>(rectFPoints[3].x() - rectFPoints[0].x()),
               static_cast<int>(rectFPoints[3].y() - rectFPoints[0].y()));
    painter.drawText(rect, Qt::AlignLeft, text);
}

void ShapesWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    handlePaint(painter);
    if (m_shapes.length() > 0) {
        emit setShapesUndo(true);
    }
}

void ShapesWidget::paintContent(QPainter &painter) const
{
    paintContent(painter, -1);
}

void ShapesWidget::paintContent(QPainter &painter, int excludedShapeIndex) const
{
    painter.setRenderHints(QPainter::Antialiasing);
    QPen pen;
    // Only committed annotations are final content. Current drawing, hover,
    // selection frames, rotate points and resize handles are editor-only.
    for (int i = 0; i < m_shapes.length(); i++) {
        if (m_shapes[i].index == excludedShapeIndex)
            continue;
        pen.setColor(BaseUtils::colorIndexOf(m_shapes[i].colorIndex));
        pen.setWidthF(m_shapes[i].lineWidth - 0.5);

        if (m_shapes[i].type == "rectangle") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintRect(painter, m_shapes[i].mainPoints, m_shapes.length(), Normal, false, false);
        } else if (m_shapes[i].type == "oval") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintEllipse(painter, m_shapes[i].mainPoints, m_shapes.length(), Normal, false, false);
        } else if (m_shapes[i].type == "effect") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            const int effectIndex = isTreeLandMode() ? -1 : i;
            if (m_shapes[i].isOval == 0) {
                paintEllipse(painter, m_shapes[i].mainPoints, effectIndex,
                             Drawing, m_shapes[i].isBlur,
                             !m_shapes[i].isBlur, m_shapes[i].radius);
            } else if (m_shapes[i].isOval == 1) {
                paintRect(painter, m_shapes[i].mainPoints, effectIndex,
                          Drawing, m_shapes[i].isBlur,
                          !m_shapes[i].isBlur, m_shapes[i].radius);
            } else {
                pen.setJoinStyle(Qt::RoundJoin);
                painter.setPen(pen);
                paintEffectLine(painter, m_shapes[i].points, m_shapes[i].isBlur, m_shapes[i].radius, m_shapes[i].lineWidth);
            }
        } else if (m_shapes[i].type == "arrow") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_shapes[i].points, pen.width(), false);
        } else if (m_shapes[i].type == "line") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_shapes[i].points, pen.width(), true);
        } else if (m_shapes[i].type == "pen") {
            pen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pen);
            paintLine(painter, m_shapes[i].points);
        } else if (m_shapes[i].type == "text") {
            const TextEdit *edit = m_editMap.value(m_shapes[i].index);
            if (!isTreeLandMode() && !edit)
                continue;
            const QString text = isTreeLandMode()
                    ? m_shapes[i].text
                    : edit->toPlainText();
            if (text.isEmpty() || text == tr("Input text here"))
                continue;
            const int fontSize = m_shapes[i].fontSize > 1
                    ? m_shapes[i].fontSize
                    : (edit ? qMax(1, edit->font().pixelSize()) : 1);
            if (isTreeLandMode()) {
                // TreeLand paints committed text without a live TextEdit.
                // Set the annotation color explicitly for both editor and export.
                painter.setPen(pen);
            }
            paintText(painter, m_shapes[i].mainPoints, text, fontSize);
        } else if (m_shapes[i].type == "sequence") {
            paintSequence(painter, m_shapes[i].mainPoints, m_shapes[i].sequenceNumber, m_shapes[i].colorIndex);
        }
    }
}

void ShapesWidget::paintLiveContent(QPainter &painter) const
{
    paintContent(painter);
    if (m_isRecording)
        paintInProgressShape(painter);
}

void ShapesWidget::paintInProgressShape(QPainter &painter) const
{
    painter.setRenderHints(QPainter::Antialiasing);
    QPen pen;

    // 绘制进行中的图形。该入口同时供传统画布和 TreeLand 跨屏视图使用，
    // 但不会绘制悬停、选中框、旋转点或缩放手柄等编辑态装饰。
    if ((m_pos1 != QPointF(0, 0) && m_pos2 != QPointF(0, 0)) || m_currentShape.type == "text") {
        FourPoints currentFPoint = getMainPoints(m_pos1, m_pos2, m_isShiftPressed);
        pen.setColor(BaseUtils::colorIndexOf(m_currentShape.colorIndex));
        pen.setWidthF(m_currentShape.lineWidth - 0.5);

        if (m_currentType == "rectangle" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintRect(painter, currentFPoint, m_shapes.length(), Normal, false, false);
        } else if (m_currentType == "oval" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintEllipse(painter, currentFPoint, m_shapes.length(), Normal, false, false);
        } else if (m_currentType == "effect") {
            if (m_currentShape.isOval == 0) {
                paintEllipse(painter, currentFPoint, m_shapes.length(), Drawing, m_currentShape.isBlur, !m_currentShape.isBlur, m_currentShape.radius);
            } else if (m_currentShape.isOval == 1) {
                paintRect(painter, currentFPoint, m_shapes.length(), Drawing, m_currentShape.isBlur, !m_currentShape.isBlur, m_currentShape.radius);
            } else {
                pen.setJoinStyle(Qt::RoundJoin);
                painter.setPen(pen);
                paintEffectLine(painter, m_currentShape.points, m_currentShape.isBlur, m_currentShape.radius, m_currentShape.lineWidth);
            }
        } else if (m_currentType == "arrow" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_currentShape.points, pen.width(), false);
        } else if (m_currentType == "line" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_currentShape.points, pen.width(), true);
        } else if (m_currentType == "pen" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pen);
            paintLine(painter, m_currentShape.points);
        } else if (m_currentType == "text" && !m_clearAllTextBorder) {
            if (m_editing)
                paintText(painter, m_currentShape.mainPoints);
        }
    }
}

void ShapesWidget::paintEditingContent(QPainter &painter) const
{
    // A visible platform inline editor owns the text/number rendering while
    // it is active. Keep the logical shape in the document for bounds and
    // export, but avoid painting a second copy underneath the real editor.
    paintContent(painter, m_inlineEditingShape);
    paintInProgressShape(painter);

    // TreeLand hosts the real inline editor in a sibling widget. Draw the
    // legacy canvas's dashed editing frame underneath it so text input has the
    // same visible bounds as X11.
    if (m_inlineEditingShape >= 0) {
        for (const Toolshape &shape : m_shapes) {
            if (shape.type == "text" && shape.index == m_inlineEditingShape
                    && shape.mainPoints.size() >= 4) {
                paintText(painter, shape.mainPoints);
                break;
            }
        }
    }

    QPen pen;

    //绘制悬停状态的图形
    if ((m_hoveredShape.mainPoints[0] != QPointF(0, 0) ||  m_hoveredShape.points.length() != 0)
            && m_hoveredIndex != -1) {
        pen.setWidthF(0.5);
        pen.setColor("#01bdff");
        if (m_hoveredShape.type == "rectangle") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintRect(painter, m_hoveredShape.mainPoints, m_hoveredIndex,  Hovered, false, false);
        } else if (m_hoveredShape.type == "oval") {
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setCapStyle(Qt::SquareCap);
            painter.setPen(pen);
            paintEllipse(painter, m_hoveredShape.mainPoints, m_hoveredIndex, Hovered, false, false);
        } else if (m_hoveredShape.type == "arrow") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_hoveredShape.points, pen.width(), false);
        } else if (m_hoveredShape.type == "line") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_hoveredShape.points, pen.width(), true);
        } else if (m_hoveredShape.type == "pen") {
            pen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pen);
            paintLine(painter, m_hoveredShape.points);
        } else if (m_hoveredShape.type == "effect" && m_hoveredShape.isOval == 2) {
            pen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pen);
            paintLine(painter, m_hoveredShape.points);
        } else {
            //        qDebug() << "hoveredShape type:" << m_hoveredShape.type;
        }
    }

    qreal ration =  this->devicePixelRatioF();
    QIcon icon(":/other/resize_handle_big.svg");
    QPixmap resizePointImg = icon.pixmap(QSize(RESIZEPOINT_WIDTH,
                                               RESIZEPOINT_WIDTH));
    resizePointImg.setDevicePixelRatio(ration);

    //只有当选中图形时m_selectedShape才会有内容
    if ((m_selectedShape.type == "arrow" ||  m_selectedShape.type == "line") && m_selectedShape.points.length() == 2) {
        paintImgPoint(painter, m_selectedShape.points[0], resizePointImg);
        paintImgPoint(painter, m_selectedShape.points[1], resizePointImg);
    } else if (m_selectedShape.type != "" && m_selectedShape.type != "text") {
        if (m_selectedShape.mainPoints[0] != QPointF(0, 0) || m_selectedShape.type == "arrow") {

            QPointF rotatePoint = getRotatePoint(m_selectedShape.mainPoints[0],
                                                 m_selectedShape.mainPoints[1],
                                                 m_selectedShape.mainPoints[2],
                                                 m_selectedShape.mainPoints[3]);

            if (m_selectedShape.type == "oval"
                    || m_selectedShape.type == "pen"
                    || (isTreeLandMode() && m_selectedShape.type == "effect")
                    || (m_selectedShape.type == "effect"
                        && m_selectedShape.isOval == 2)) {
                pen.setJoinStyle(Qt::MiterJoin);
                pen.setWidth(1);
                pen.setColor(QColor("#01bdff"));
                painter.setPen(pen);
                paintRect(painter,  m_selectedShape.mainPoints, -1);
            }

            // 序号标注选中时只显示控制点，不显示旋转图标；其他图形保持原样。
            if (m_selectedShape.type != "sequence") {
                QPixmap rotatePointImg;
                rotatePointImg = QIcon(":/other/icon_rotate.svg").pixmap(ROTATE_ICON_SIZE);
                rotatePointImg.setDevicePixelRatio(this->devicePixelRatioF());
                paintImgPoint(painter, rotatePoint, rotatePointImg, false);
            }

            //画出形状的主要骨架点
            for (int i = 0; i < m_selectedShape.mainPoints.length(); i ++) {

                paintImgPoint(painter, m_selectedShape.mainPoints[i], resizePointImg);
            }

            //画出形状的其他骨架点
            FourPoints anotherFPoints = getAnotherFPoints(m_selectedShape.mainPoints);
            for (int j = 0; j < anotherFPoints.length(); j++) {
                paintImgPoint(painter, anotherFPoints[j], resizePointImg);
            }
        }
    } else if (m_textSelectionFrameEnabled
               && m_selectedShape.type == "text"
               && m_selectedIndex != m_inlineEditingShape
               && m_selectedShape.mainPoints.size() >= 4) {
        paintText(painter, m_selectedShape.mainPoints);
    }
}

void ShapesWidget::handlePaint(QPainter &painter)
{
    paintEditingContent(painter);
}

//将编辑的内容绘制到图片上
void ShapesWidget::paintImage(QImage &image)
{
    QPainter painter(&image);
    paintContent(painter);
    //    backgroundImage.save("/home/uos/Desktop/temp1.png");
}

bool ShapesWidget::isExistsText()
{
    for (int i = 0; i < m_shapes.length(); i++) {
        if (m_shapes[i].type == "text") {
            return true;
        }
    }
    return  false;
}

void ShapesWidget::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    if (m_currentType == "pen") {
        applyOverrideCursor(BaseUtils::setCursorShape("pen",  BaseUtils::colorIndex(m_penColor)));
    } else if (m_currentType == "effect") {
        int isOval = ConfigSettings::instance()->getValue("effect", "isOval").toInt();
        QCursor setCursorValue;
        if (isOval == 0) {
            setCursorValue = BaseUtils::setCursorShape("oval");
        } else if (isOval == 1) {
            setCursorValue = BaseUtils::setCursorShape("rectangle");
        } else {
            setCursorValue = BaseUtils::setCursorShape("pen", 0);
        }
        applyOverrideCursor(setCursorValue);
    } else {
        applyOverrideCursor(BaseUtils::setCursorShape(m_currentType));
    }
}

void ShapesWidget::pinchTriggered(QPinchGesture *pinch)
{
    if (-1 == m_selectedIndex || "text" == m_currentType)
        return;
    QPinchGesture::ChangeFlags changeFlags = pinch->changeFlags();
    if (changeFlags & QPinchGesture::RotationAngleChanged) {
        qreal rotationDelta = (pinch->rotationAngle() - pinch->lastRotationAngle()) * 0.01;
        qreal centerX = (m_shapes[m_selectedOrder].mainPoints[0].x() + m_shapes[m_selectedOrder].mainPoints[3].x()) / 2;
        qreal centerY = (m_shapes[m_selectedOrder].mainPoints[0].y() + m_shapes[m_selectedOrder].mainPoints[3].y()) / 2;
        for (int i = 0; i < m_shapes[m_selectedOrder].mainPoints.size(); ++i) {
            qreal x = centerX + (m_shapes[m_selectedOrder].mainPoints[i].x() - centerX) * cos(rotationDelta) - (m_shapes[m_selectedOrder].mainPoints[i].y() - centerY) * sin(rotationDelta);
            qreal y = centerY + (m_shapes[m_selectedOrder].mainPoints[i].x() - centerX) * sin(rotationDelta) + (m_shapes[m_selectedOrder].mainPoints[i].y() - centerY) * cos(rotationDelta);
            //图形
            m_shapes[m_selectedOrder].mainPoints[i].setX(x);
            m_shapes[m_selectedOrder].mainPoints[i].setY(y);
        }
        qreal centerMainX = (m_selectedShape.mainPoints[0].x() + m_selectedShape.mainPoints[3].x()) / 2;
        qreal centerMainY = (m_selectedShape.mainPoints[0].y() + m_selectedShape.mainPoints[3].y()) / 2;
        for (int i = 0; i < m_selectedShape.mainPoints.length(); i ++) {
            qreal x = centerMainX + (m_selectedShape.mainPoints[i].x() - centerMainX) * cos(rotationDelta) - (m_selectedShape.mainPoints[i].y() - centerMainY) * sin(rotationDelta);
            qreal y = centerMainY + (m_selectedShape.mainPoints[i].x() - centerMainX) * sin(rotationDelta) + (m_selectedShape.mainPoints[i].y() - centerMainY) * cos(rotationDelta);
            //选中状态，点+线条
            m_selectedShape.mainPoints[i].setX(x);
            m_selectedShape.mainPoints[i].setY(y);
        }
        for (int k = 0; k < m_selectedShape.points.length(); k++) {
            qreal x = centerMainX + (m_shapes[m_selectedOrder].points[k].x() - centerMainX) * cos(rotationDelta) - (m_shapes[m_selectedOrder].points[k].y() - centerMainY) * sin(rotationDelta);
            qreal y = centerMainY + (m_shapes[m_selectedOrder].points[k].x() - centerMainX) * sin(rotationDelta) + (m_shapes[m_selectedOrder].points[k].y() - centerMainY) * cos(rotationDelta);
            //线条
            m_shapes[m_selectedOrder].points[k].setX(x);
            m_shapes[m_selectedOrder].points[k].setY(y);
        }
    }
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        qreal scale = pinch->scaleFactor();
        qreal ascanle = (scale - 1) * 0.3 + 1;
        qreal centerX = (m_shapes[m_selectedOrder].mainPoints[0].x() + m_shapes[m_selectedOrder].mainPoints[3].x()) / 2;
        qreal centerY = (m_shapes[m_selectedOrder].mainPoints[0].y() + m_shapes[m_selectedOrder].mainPoints[3].y()) / 2;
        for (int i = 0; i < m_shapes[m_selectedOrder].mainPoints.size(); ++i) {
            qreal x = m_shapes[m_selectedOrder].mainPoints[i].x() * ascanle + centerX * (1 - ascanle);
            qreal y = m_shapes[m_selectedOrder].mainPoints[i].y() * ascanle + centerY * (1 - ascanle);
            m_shapes[m_selectedOrder].mainPoints[i].setX(x);
            m_shapes[m_selectedOrder].mainPoints[i].setY(y);
        }
        qreal centerMainX = (m_selectedShape.mainPoints[0].x() + m_selectedShape.mainPoints[3].x()) / 2;
        qreal centerMainY = (m_selectedShape.mainPoints[0].y() + m_selectedShape.mainPoints[3].y()) / 2;
        for (int i = 0; i < m_selectedShape.mainPoints.length(); i ++) {
            qreal x = m_selectedShape.mainPoints[i].x() * ascanle + centerMainX * (1 - ascanle);
            qreal y = m_selectedShape.mainPoints[i].y() * ascanle + centerMainY * (1 - ascanle);
            m_selectedShape.mainPoints[i].setX(x);
            m_selectedShape.mainPoints[i].setY(y);
        }
        for (int k = 0; k < m_selectedShape.points.length(); k++) {
            qreal x = m_shapes[m_selectedOrder].points[k].x() * ascanle + centerMainX * (1 - ascanle);
            qreal y = m_shapes[m_selectedOrder].points[k].y() * ascanle + centerMainY * (1 - ascanle);
            m_shapes[m_selectedOrder].points[k].setX(x);
            m_shapes[m_selectedOrder].points[k].setY(y);
        }
    }
}

void ShapesWidget::tapTriggered(QTapGesture *tap)
{
    if (tap->state() == Qt::GestureState::GestureFinished && !clickedOnShapes(m_movingPoint) && -1 != m_selectedIndex && "text" != m_currentType) {
        m_isPressed = false;
        m_isMoving = false;
        clearSelected();
        setAllTextEditReadOnly();
        m_editing = false;
        m_selectedIndex = -1;
        m_selectedOrder = -1;
        m_selectedShape.type = "";
    }
}

void ShapesWidget::deleteCurrentShape()
{
    qCDebug(dsrApp, ) << "delete shape";
    int deletedSequenceOrder = -1;
    int previousSequenceNumber = 0;
    if (m_selectedOrder >= 0 && m_selectedOrder < m_shapes.length()) {
        if (m_shapes[m_selectedOrder].type == "sequence") {
            int sequenceOrder = 0;
            for (int i = 0; i < m_selectedOrder; ++i) {
                if (m_shapes[i].type != "sequence") {
                    continue;
                }
                previousSequenceNumber = m_shapes[i].sequenceNumber;
                ++sequenceOrder;
            }
            deletedSequenceOrder = sequenceOrder;
        }
        m_shapes.removeAt(m_selectedOrder);
    } else {
        qCWarning(dsrApp, ) << "Invalid index";
    }
    if (deletedSequenceOrder >= 0) {
        renumberSequenceFrom(deletedSequenceOrder, previousSequenceNumber + 1);
    }

    if (m_selectedShape.type == "text" && m_editMap.contains(m_selectedShape.index)) {
        m_editMap.value(m_selectedShape.index)->clear();
        m_editMap.remove(m_selectedShape.index);
    }

    clearSelected();
    m_selectedShape.type = "";
    m_currentShape.type = "";
    for (int i = 0; i < m_currentShape.mainPoints.length(); i++) {
        m_currentShape.mainPoints[i] = QPointF(0, 0);
    }

    if (m_shapes.length() == 0) {
        emit setShapesUndo(false);
    }

    update();
    emit shapesChanged();
    m_selectedIndex = -1;
    m_selectedOrder = -1;
}

void ShapesWidget::undoDrawShapes()
{
    textEditIsReadOnly();
    qCDebug(dsrApp, ) << "undoDrawShapes m_selectedIndex:" << m_selectedIndex << m_shapes.length();
    if (m_selectedOrder >= 0 && m_selectedOrder < m_shapes.length() && m_selectedIndex != -1) {
        deleteCurrentShape();
    } else if (m_shapes.length() > 0) {
        int tmpIndex = m_shapes[m_shapes.length() - 1].index;
        if (m_shapes[m_shapes.length() - 1].type == "text" && m_editMap.contains(tmpIndex)) {
            m_editMap.value(tmpIndex)->clear();
            delete m_editMap.value(tmpIndex);
            m_editMap.remove(tmpIndex);
        }

        m_shapes.removeLast();
        if (m_currentType == "sequence") {
            updateSequencePreview(mapFromGlobal(QCursor::pos()));
        }
    }
    qCDebug(dsrApp, ) << "undoDrawShapes m_selectedIndex:" << m_selectedIndex << m_shapes.length();

    if (m_shapes.length() == 0) {
        emit setShapesUndo(false);
    }
    m_isSelected = false;
    clearSelected();
    update();
    emit shapesChanged();
}
void ShapesWidget::undoAllDrawShapes()
{
    qCDebug(dsrApp, ) << "undoAllDrawShapes undoDrawShapes m_selectedIndex:" << m_selectedIndex << m_shapes.length();
    if (m_selectedOrder >= 0 && m_selectedOrder < m_shapes.length() && m_selectedIndex != -1) {
        deleteCurrentShape();
    } else if (m_shapes.length() > 0) {
        while (m_shapes.length() > 0) {
            int tmpIndex = m_shapes[m_shapes.length() - 1].index;
            if (m_shapes[m_shapes.length() - 1].type == "text" && m_editMap.contains(tmpIndex)) {
                m_editMap.value(tmpIndex)->clear();
                delete m_editMap.value(tmpIndex);
                m_editMap.remove(tmpIndex);
            }

            m_shapes.removeLast();
        }
        if (m_currentType == "sequence") {
            updateSequencePreview(mapFromGlobal(QCursor::pos()));
        }
    }
    qCDebug(dsrApp, ) << "undoDrawShapes m_selectedIndex:" << m_selectedIndex << m_shapes.length();

    if (m_shapes.length() == 0) {
        emit setShapesUndo(false);
    }
    m_isSelected = false;
    clearSelected();
    update();
    emit shapesChanged();
}

void ShapesWidget::isInUndoBtn(bool isUndo)
{
    //qDebug() << ">>>>>> isInUndoBtn"  << isUndo;
    m_isUnDo = isUndo;
}

void ShapesWidget::microAdjust(QString direction)
{
    if (m_selectedIndex == -1 || m_selectedOrder >= m_shapes.length()) {
        return;
    }
    
    Toolshape &currentShape = m_shapes[m_selectedOrder];
    
    if (currentShape.type == "text") {
        return;
    }

    // 保存原始的mainPoints，用于计算偏移量
    FourPoints oldMainPoints = currentShape.mainPoints;

    // 根据方向调整mainPoints
    bool isSimpleMove = direction == Direction::LEFT || direction == Direction::RIGHT || 
                        direction == Direction::UP || direction == Direction::DOWN;
                        
    bool isReduceResize = direction == Direction::CTRL_SHIFT_LEFT || direction == Direction::CTRL_SHIFT_RIGHT ||
                          direction == Direction::CTRL_SHIFT_UP || direction == Direction::CTRL_SHIFT_DOWN;

    // 序号标注仅支持整体平移微调，不启用缩放（与需求「仅需移动+删除」口径一致）
    if (currentShape.type == "sequence" && !isSimpleMove) {
        return;
    }

    // 调整mainPoints
    if (isSimpleMove) {
        currentShape.mainPoints = pointMoveMicro(currentShape.mainPoints, direction);
    } else if (isReduceResize) {
        currentShape.mainPoints = pointResizeMicro(currentShape.mainPoints, direction, false);
    } else {
        currentShape.mainPoints = pointResizeMicro(currentShape.mainPoints, direction, true);
    }

    if (currentShape.type == "line" || currentShape.type == "arrow") {
        if (currentShape.portion.length() == 0) {
            for (int k = 0; k < currentShape.points.length(); k++) {
                currentShape.portion.append(relativePosition(currentShape.mainPoints,
                                                                          currentShape.points[k]));
            }
        }
        for (int j = 0; j < currentShape.points.length(); j++) {
            currentShape.points[j] = getNewPosition(
                                                      currentShape.mainPoints, currentShape.portion[j]);
        }
    } 
    // 处理画笔类型的points
    else if (currentShape.type == "pen" || 
            (currentShape.type == "effect" && currentShape.isOval == 2)) {
        
        QPointF offset;
        if (isSimpleMove) {
            // 移动操作，计算偏移量
            offset = currentShape.mainPoints[0] - oldMainPoints[0];
            
            // 对所有点应用相同的偏移量
            for (int j = 0; j < currentShape.points.length(); j++) {
                currentShape.points[j] += offset;
            }
        } else {
            // 缩放操作
            QPointF oldCenter = (oldMainPoints[0] + oldMainPoints[3]) / 2;
            QPointF newCenter = (currentShape.mainPoints[0] + currentShape.mainPoints[3]) / 2;
            
            // 计算缩放比例
            QRectF oldRect = QRectF(oldMainPoints[0], oldMainPoints[3]);
            QRectF newRect = QRectF(currentShape.mainPoints[0], currentShape.mainPoints[3]);
            qreal scaleX = newRect.width() / oldRect.width();
            qreal scaleY = newRect.height() / oldRect.height();
            
            // 对每个点应用缩放和偏移
            for (int j = 0; j < currentShape.points.length(); j++) {
                QPointF point = currentShape.points[j];
                QPointF relativePos = point - oldCenter;
                relativePos.setX(relativePos.x() * scaleX);
                relativePos.setY(relativePos.y() * scaleY);
                currentShape.points[j] = newCenter + relativePos;
            }
        }
    }

    // 更新选中的形状
    m_selectedShape.mainPoints = currentShape.mainPoints;
    m_selectedShape.points = currentShape.points;
    m_hoveredShape.type = "";
    update();
    emit shapesChanged();
}

void ShapesWidget::setShiftKeyPressed(bool isShift)
{
    m_isShiftPressed = isShift;
}

void ShapesWidget::setInteractionPlatform(InteractionPlatform platform)
{
    m_interactionPlatform = platform;
}

bool ShapesWidget::isTreeLandMode() const
{
    return m_interactionPlatform == InteractionPlatform::TreeLand;
}

void ShapesWidget::setCursorManagementEnabled(bool enabled)
{
    m_cursorManagementEnabled = enabled;
}

void ShapesWidget::setTextSelectionFrameEnabled(bool enabled)
{
    if (m_textSelectionFrameEnabled == enabled)
        return;
    m_textSelectionFrameEnabled = enabled;
    update();
}

void ShapesWidget::applyOverrideCursor(const QCursor &cursor)
{
    if (!m_cursorManagementEnabled)
        return;

    // Preserve the legacy X11 cursor stack behavior exactly. TreeLand detaches
    // its document widget and owns the cursor in the mask overlay instead.
    if (!isTreeLandMode()) {
        qApp->setOverrideCursor(cursor);
        return;
    }

    const QCursor *currentOverride = qApp->overrideCursor();
    if (currentOverride == nullptr) {
        qApp->setOverrideCursor(cursor);
    } else if (*currentOverride != cursor) {
        qApp->changeOverrideCursor(cursor);
    }
}

void ShapesWidget::updateCursorShape()
{
//    qDebug() << "func" << __FUNCTION__ << "line" << __LINE__;
    if (!m_isUnDo) {
        QCursor setCursorValue;
        if (m_currentType.isEmpty()) {
            setCursorValue = QCursor(Qt::ArrowCursor);
        } else if (m_currentType == "pen") {
            m_penColor = BaseUtils::colorIndexOf(ConfigSettings::instance()->getValue("pen", "color_index").toInt());
            setCursorValue = BaseUtils::setCursorShape(m_currentType, BaseUtils::colorIndex(m_penColor));
        } else if (m_currentType == "effect") {
            int isOval = ConfigSettings::instance()->getValue("effect", "isOval").toInt();
            if (isOval == 0) {
                setCursorValue = BaseUtils::setCursorShape("oval");
            } else if (isOval == 1) {
                setCursorValue = BaseUtils::setCursorShape("rectangle");
            } else {
                setCursorValue = BaseUtils::setCursorShape("pen", 0);
            }
        } else if (m_currentType == "sequence") {
            setCursorValue = QCursor(Qt::CrossCursor);
        } else {
            setCursorValue = BaseUtils::setCursorShape(m_currentType);
        }
        // Preserve the original X11 changeOverrideCursor path exactly.
        if (!isTreeLandMode()) {
            const QCursor *currentOverride = qApp->overrideCursor();
            if (currentOverride == nullptr || *currentOverride != setCursorValue) {
                qApp->changeOverrideCursor(setCursorValue);
            }
        } else {
            applyOverrideCursor(setCursorValue);
        }
    }
}

void ShapesWidget::menuSaveSlot()
{
    emit saveFromMenu();
}

void ShapesWidget::menuCloseSlot()
{
    emit closeFromMenu();
}
void ShapesWidget::setGlobalRect(QRect rect)
{
    m_globalRect = rect;
}

int ShapesWidget::sequenceCount()
{
    int count = 0;
    for (int i = 0; i < m_shapes.length(); i++) {
        if (m_shapes[i].type == "sequence") {
            count++;
        }
    }
    return count;
}

int ShapesWidget::sequenceOrderForIndex(int shapeIndex) const
{
    int order = 0;
    for (const Toolshape &shape : m_shapes) {
        if (shape.type != "sequence")
            continue;
        if (shape.index == shapeIndex)
            return order;
        ++order;
    }
    return -1;
}

int ShapesWidget::sequenceNumberBeforeOrder(int order)
{
    if (order <= 0) {
        return 0;
    }
    int sequenceOrder = 0;
    for (int i = 0; i < m_shapes.length(); ++i) {
        if (m_shapes[i].type != "sequence") {
            continue;
        }
        if (sequenceOrder == order - 1) {
            return m_shapes[i].sequenceNumber;
        }
        ++sequenceOrder;
    }
    return 0;
}

int ShapesWidget::sequenceNextNumber()
{
    // 新增编号接在最后一个已有序号之后，不受前面手动改号次数影响。
    int lastNumber = 0;
    for (int i = 0; i < m_shapes.length(); ++i) {
        if (m_shapes[i].type == "sequence") {
            lastNumber = m_shapes[i].sequenceNumber;
        }
    }
    return lastNumber + 1;
}

bool ShapesWidget::placeSequenceAt(const QPointF &pos)
{
    const int sequenceNumber = sequenceNextNumber();
    if (sequenceNumber > SEQUENCE_MAX_NUMBER) {
        updateSequencePreview(pos);
        showSequenceLimitTips(pos);
        return false;
    }

    const int tier = qBound(0,
                            ConfigSettings::instance()->getValue("sequence", "line_width").toInt(),
                            2);
    const qreal half = SEQUENCE_DIAMETER(tier) / 2.0;
    m_currentShape.index = m_currentIndex;
    m_currentShape.sequenceNumber = sequenceNumber;
    m_currentShape.mainPoints[0] = QPointF(pos.x() - half, pos.y() - half);
    m_currentShape.mainPoints[1] = QPointF(pos.x() - half, pos.y() + half);
    m_currentShape.mainPoints[2] = QPointF(pos.x() + half, pos.y() - half);
    m_currentShape.mainPoints[3] = QPointF(pos.x() + half, pos.y() + half);
    m_shapes.append(m_currentShape);
    updateSequencePreview(pos);
    return true;
}

void ShapesWidget::renumberSequenceFrom(int startOrder, int firstNumber)
{
    // 只重排当前项及其后续项，前面已有编号保持不变。
    int sequenceOrder = 0;
    int nextNumber = firstNumber;
    for (int i = 0; i < m_shapes.length(); ++i) {
        if (m_shapes[i].type != "sequence") {
            continue;
        }
        if (sequenceOrder >= startOrder) {
            m_shapes[i].sequenceNumber = nextNumber++;
        }
        ++sequenceOrder;
    }
    if (m_currentType == "sequence") {
        updateSequencePreview(mapFromGlobal(QCursor::pos()));
    }
}

void ShapesWidget::resizeSelectedSequence(int lineWidthIndex)
{
    // 保存原始映射关系：lineWidthIndex -> 圆圈直径（与新建时保持一致）
    if (m_selectedShape.mainPoints.length() < 4 || lineWidthIndex < 0) {
        return;
    }

    if (m_sequenceToolEntry) {
        // 工具进入阶段仅恢复面板状态，不回写、不缩放已有选中项
        return;
    }

    const qreal diameter = SEQUENCE_DIAMETER(lineWidthIndex);
    const QPointF center((m_selectedShape.mainPoints[0].x() + m_selectedShape.mainPoints[3].x()) / 2.0,
                         (m_selectedShape.mainPoints[0].y() + m_selectedShape.mainPoints[3].y()) / 2.0);

    // 以圆心为基准等比缩放，保证点击位置与编号不变
    // 与箭头/直线一致，lineWidth 也写入配置值供下次记忆
    const qreal oldDiameter = qMax(qAbs(m_selectedShape.mainPoints[3].x() - m_selectedShape.mainPoints[0].x()),
                                   qAbs(m_selectedShape.mainPoints[3].y() - m_selectedShape.mainPoints[0].y()));
    if (oldDiameter <= 0) {
        return;
    }
    if (qFuzzyCompare(oldDiameter, diameter)) {
        // 尺寸未变化：避免配置写入再次触发本函数形成递归
        m_selectedShape.lineWidth = LINEWIDTH(lineWidthIndex);
        return;
    }
    const qreal scale = diameter / oldDiameter;
    for (int i = 0; i < m_selectedShape.mainPoints.length(); ++i) {
        const QPointF p = m_selectedShape.mainPoints[i];
        m_selectedShape.mainPoints[i] = QPointF(center.x() + (p.x() - center.x()) * scale,
                                                center.y() + (p.y() - center.y()) * scale);
    }
    m_selectedShape.lineWidth = LINEWIDTH(lineWidthIndex);
    ConfigSettings::instance()->setValue("sequence", "line_width", lineWidthIndex);

    // 若正在就地编辑该序号，保持输入框居中并同步字号
    if (m_sequenceEdit && m_sequenceEditOrder >= 0) {
        const QSize editSize(m_sequenceEdit->width(), m_sequenceEdit->height());
        m_sequenceEdit->move(static_cast<int>(center.x() - editSize.width() / 2.0),
                             static_cast<int>(center.y() - editSize.height() / 2.0));
    }
}

bool ShapesWidget::isLightSequenceColor(int colorIndex) const
{
    // 设计稿中仅白色、黄色、亮绿色使用黑色文字，其余颜色使用白色文字。
    return colorIndex == 2 || colorIndex == 5 || colorIndex == 6;
}

void ShapesWidget::paintSequence(QPainter &painter, FourPoints mainFPoints,
                                  int sequenceNumber, int colorIndex) const
{
    if (mainFPoints.length() < 4) {
        return;
    }
    QRectF rectF(mainFPoints[0], mainFPoints[3]);
    QPointF center = rectF.center();
    qreal radius = qMin(rectF.width(), rectF.height()) / 2.0;
    if (radius <= 0) {
        return;
    }
    QColor fillColor = BaseUtils::colorIndexOf(colorIndex);
    // 序号需要填充圆和文字；必须恢复 painter 状态，否则后续图形会继承
    // 这里的 brush，圆形/矩形会被误画成实心图形。
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(fillColor));
    const QRectF ellipseRect(center.x() - radius, center.y() - radius,
                             radius * 2.0, radius * 2.0);
    painter.drawEllipse(ellipseRect);
    QColor textColor = isLightSequenceColor(colorIndex) ? Qt::black : Qt::white;
    painter.setPen(QPen(textColor));
    QFont font = painter.font();
    // 设计稿：24/32/40 圆对应 13/17/22px Roboto-Black 字号。
    font.setPixelSize(qMax(8, qRound(radius * 1.08)));
    font.setWeight(QFont::Black);
    painter.setFont(font);
    painter.drawText(rectF, Qt::AlignCenter, QString::number(sequenceNumber));
    painter.restore();
}

void ShapesWidget::updateSequencePreview(const QPointF &pos)
{
    auto *preview = static_cast<SequenceLimitBadgeWidget *>(m_sequencePreview);
    if (!preview) {
        return;
    }
    int next = sequenceNextNumber();
    // 正常序号模式不显示下一个编号预览；只有下一个编号超过 99 时才显示禁用角标。
    const bool maxReached = next > SEQUENCE_MAX_NUMBER;
    if (!maxReached) {
        preview->hide();
        hideSequenceLimitTips();
        return;
    }

    preview->move(static_cast<int>(pos.x() + SEQUENCE_PREVIEW_ANCHOR),
                  static_cast<int>(pos.y() + SEQUENCE_PREVIEW_ANCHOR));
    preview->show();
    preview->raise();
    // 角标持续跟随鼠标；气泡仅在点击弹出后跟随，2 秒内不因移动而重新弹出。
    updateSequenceLimitTips(pos);
}

void ShapesWidget::moveSequenceLimitTips(const QPointF &pos)
{
    QWidget *host = window();
    if (!host || !m_sequenceLimitTips) {
        return;
    }
    // 提示左上角与禁止角标右下角对齐（设计稿：角标 + (17,17)）
    const QPoint anchor = mapToGlobal(QPoint(qRound(pos.x()) + SEQUENCE_LIMIT_TIPS_ANCHOR,
                                             qRound(pos.y()) + SEQUENCE_LIMIT_TIPS_ANCHOR));
    m_sequenceLimitTips->move(host->mapFromGlobal(anchor));
}

void ShapesWidget::updateSequenceLimitTips(const QPointF &pos)
{
    // 仅在气泡可见时跟随鼠标；隐藏后不因移动重新弹出，需再次点击
    if (!m_sequenceLimitTips || !m_sequenceLimitTips->isVisible()) {
        return;
    }
    moveSequenceLimitTips(pos);
    m_sequenceLimitTips->raise();
}

void ShapesWidget::showSequenceLimitTips(const QPointF &pos)
{
    QWidget *host = window();
    if (!host) {
        return;
    }
    if (!m_sequenceLimitTips) {
        // 复用工程内已有的 DTK 工具提示（对应设计稿 DTK/工具提示），不做自绘、不使用 Qt 样式表
        m_sequenceLimitTips = new ToolTips(tr("Maximum number range exceeded"), host);
        m_sequenceLimitTips->adjustSize();
    }
    moveSequenceLimitTips(pos);
    m_sequenceLimitTips->show();
    m_sequenceLimitTips->raise();
    if (m_sequenceLimitTipsTimer) {
        m_sequenceLimitTipsTimer->start();
    }
}

void ShapesWidget::hideSequenceLimitTips()
{
    if (m_sequenceLimitTipsTimer) {
        m_sequenceLimitTipsTimer->stop();
    }
    if (m_sequenceLimitTips) {
        m_sequenceLimitTips->hide();
    }
}

void ShapesWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (m_currentType == "sequence") {
        int hitOrder = -1;
        int currentDisplay = 0;
        QRectF hitRect;
        for (int i = m_shapes.length() - 1; i >= 0; i--) {
            if (m_shapes[i].type != "sequence") {
                continue;
            }
            int orderUpTo = -1;
            int cnt = 0;
            for (int k = 0; k <= i; k++) {
                if (m_shapes[k].type == "sequence") {
                    if (k == i) {
                        orderUpTo = cnt;
                    }
                    cnt++;
                }
            }
            QRectF r(m_shapes[i].mainPoints[0], m_shapes[i].mainPoints[3]);
            if (r.contains(e->pos())) {
                hitOrder = orderUpTo;
                currentDisplay = m_shapes[i].sequenceNumber;
                hitRect = r;
                break;
            }
        }
        if (hitOrder >= 0) {
            // 就地内联编辑：在命中的序号圆圈位置覆盖一个 QLineEdit 输入框，
            // 预填当前编号；回车确认改号并从当前项起顺延重排，Esc 取消 / 失焦提交。
            // 下限必须大于前一个序号，避免逆序；编辑首项时从 1 开始。
            // 上限保证当前项及后续已有项都不超过 SEQUENCE_MAX_NUMBER。
            const int editMinNum = sequenceNumberBeforeOrder(hitOrder) + 1;
            const int editMaxNum = SEQUENCE_MAX_NUMBER - (sequenceCount() - 1 - hitOrder);
            m_sequenceEdit = new QLineEdit(this);
            m_sequenceEditOrder = hitOrder;
            m_sequenceEdit->setText(QString::number(currentDisplay));
            m_sequenceEdit->setMaxLength(2);
            m_sequenceEdit->setValidator(new QIntValidator(editMinNum, editMaxNum, m_sequenceEdit));
            m_sequenceEdit->setAlignment(Qt::AlignCenter);
            m_sequenceEdit->installEventFilter(this);
            // 将输入框居中覆盖在序号圆圈位置，实现"就地"编辑
            QSize editSize(40, 28);
            QPointF center = hitRect.center();
            m_sequenceEdit->setFixedSize(editSize);
            m_sequenceEdit->move(static_cast<int>(center.x() - editSize.width() / 2.0),
                                 static_cast<int>(center.y() - editSize.height() / 2.0));
            m_sequenceEdit->show();
            m_sequenceEdit->raise();
            m_sequenceEdit->setFocus();
            m_sequenceEdit->selectAll();
            return;
        }
    }
    DFrame::mouseDoubleClickEvent(e);
}

bool ShapesWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_sequenceEdit && m_sequenceEdit) {
        // 拦截全局 Enter/Return/Esc 快捷键（保存截图/退出），避免编辑时泄漏
        if (event->type() == QEvent::ShortcutOverride) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();
            if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Escape) {
                event->accept();
                return true;
            }
        } else if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();
            // 回车确认改号
            if (key == Qt::Key_Return || key == Qt::Key_Enter) {
                commitSequenceEdit();
                return true;
            }
            // Esc 取消编辑
            if (key == Qt::Key_Escape) {
                cancelSequenceEdit();
                return true;
            }
        } else if (event->type() == QEvent::FocusOut) {
            // 失焦提交编辑，与回车行为一致
            commitSequenceEdit();
            return false;
        }
    }
    return DFrame::eventFilter(watched, event);
}

void ShapesWidget::commitSequenceEdit()
{
    if (!m_sequenceEdit) {
        return;
    }
    QLineEdit *edit = m_sequenceEdit;
    int hitOrder = m_sequenceEditOrder;
    bool ok = false;
    int newNum = edit->text().toInt(&ok);
    // 改号后只重排当前项及后续项，前面已有编号保持不动。
    int minNum = sequenceNumberBeforeOrder(hitOrder) + 1;
    int maxNum = SEQUENCE_MAX_NUMBER - (sequenceCount() - 1 - hitOrder);
    if (!ok || newNum < minNum) {
        // 空值或小于前一个序号：不合法，直接取消本次编辑
        cancelSequenceEdit();
        return;
    }
    if (newNum > maxNum) {
        // 只有真正超出最大编号范围时才给出设计稿提示，同样 2 秒后消失
        showSequenceLimitTips(edit->geometry().center());
        cancelSequenceEdit();
        return;
    }
    m_sequenceEdit = nullptr;
    m_sequenceEditOrder = -1;
    renumberSequenceFrom(hitOrder, newNum);
    update();
    edit->deleteLater();
}

void ShapesWidget::cancelSequenceEdit()
{
    if (!m_sequenceEdit) {
        return;
    }
    QLineEdit *edit = m_sequenceEdit;
    m_sequenceEdit = nullptr;
    m_sequenceEditOrder = -1;
    edit->deleteLater();
}
