// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROUNDEDCORNERWIDGET_H
#define ROUNDEDCORNERWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QTimer>
#include <QIntValidator>

#include <DIconButton>
#include <DPushButton>
#include <DLineEdit>
#include <DSlider>
#include <DSpinBox>
#include <DToolButton>

DWIDGET_USE_NAMESPACE

class QAction;
class QMouseEvent;
class ToolButton;
class HintFilter;

/**
 * @brief 选区右上角的圆角 / 尺寸比例工具组。
 *
 * 对齐设计稿「截图录屏-工具栏-圆角」「截图录屏-工具栏-比例」：
 * 56x103 竖排按钮组（圆角按钮在上、尺寸/比例按钮在下），
 * 属性面板 270x56 在按钮组左侧或右侧水平展开。
 *
 * 本控件只是按钮组与面板的容器，容器本身保持透明，
 * 按钮组与面板背景使用 DTK 的 DBlurEffectWidget（与主工具栏一致的模糊背景）。
 */
class RoundedCornerWidget : public QWidget
{
    Q_OBJECT
public:
    enum PanelType {
        NoPanel = 0,
        CornerPanel,
        SizePanel
    };

    explicit RoundedCornerWidget(QWidget *parent = nullptr);
    ~RoundedCornerWidget() override;

    int radius() const;
    bool isCornerEnabled() const;
    void setRadius(int radius);

    bool isRatioLocked() const;
    // 当前锁定的宽高比（宽 / 高）；未锁定或无效时返回 0。
    qreal lockedRatio() const;
    PanelType currentPanel() const { return m_panel; }

    // 本次启动是否还有待应用的记忆尺寸/比例。图钉开启且配置中有有效值时，
    // 第一次完成框选后由主窗口应用一次。
    bool hasPendingRememberedSetting() const;
    bool isRememberedSettingRatio() const;
    QSize rememberedSetting() const;
    void consumeRememberedSetting();

    /**
     * @brief 跟随选区右上角；按设计稿优先级计算位置与面板展开方向
     *   ① 选区右侧空间足够 -> 按钮组在选区右上角右侧，面板向右展开
     *   ② 右侧不足、左侧足够 -> 按钮组在选区左上角左侧，面板向左展开
     *   ③ 两侧均不足 -> 按钮组在选区右上角内侧，面板向左展开
     */
    void followSelection(const QRect &selection, const QRect &screen);
    /**
     * @brief 按同一套①/②/③规则计算按钮组可见底板在父窗口坐标系中的矩形，
     * 但不移动控件。供主窗口判断工具栏是否会被按钮组遮挡：
     * 设计稿③下工具栏会顺移到按钮组下方显示。
     */
    QRect groupPlateRect(const QRect &selection, const QRect &screen) const;
    // 用户是否手动拖动过按钮组；拖动之后不再按①/②/③自动贴边，
    // 与主工具栏「拖动过就保持用户位置」的行为保持一致。
    bool isUserMoved() const { return m_userMoved; }
    // 按钮组可见底板当前的实际矩形（父窗口坐标系）。用户拖动过之后它不再等于
    // groupPlateRect() 算出来的位置，主窗口判断工具栏是否被遮挡时要用这个。
    QRect currentGroupPlateRect() const;
    // 主窗口同步当前选区尺寸到尺寸/比例面板
    void setSelectionSize(int width, int height);
    // 按父窗口坐标返回浮层当前应使用的光标：面板输入框 I 型、
    // 按钮组张开/闭合手型、其余区域普通箭头。
    Qt::CursorShape cursorShapeAt(const QPoint &parentPos) const;

    // 生成带圆角透明通道的图片；radius <= 0 时返回原图
    static QPixmap applyRoundedCorners(const QPixmap &source, int radius);
    // JPG/BMP 等不支持透明格式：把含 Alpha 的成图合成到白底上，
    // 使圆角外区域呈现白色（投影等半透明像素也一并正确压在白色上）。
    static QPixmap compositeOnWhite(const QPixmap &source);

signals:
    void radiusChanged(int radius);
    // 用户在尺寸面板中输入了新的选区尺寸
    void selectionSizeChanged(int width, int height);
    // 用户在比例面板中输入了新的锁定比例（宽:高）
    void ratioChanged(int width, int height);
    void panelExpandedChanged(bool expanded);
    // 用户拖动按钮组结束，主窗口据此重新检查工具栏是否被遮挡
    void groupMoved();

public slots:
    // 点击其它工具时收起属性面板（保留圆角与尺寸比例设置）
    void collapsePanels();

protected:
    // 按钮组支持像主工具栏一样拖动；按钮/滑块等子控件自己消费点击，不受影响。
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    // 保护属性面板区域的光标，避免被主窗口的选区拖动全局光标覆盖
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUi();
    QWidget *buildGroup();
    QWidget *buildCornerPanel();
    QWidget *buildSizePanel();
    void installPanelCursorFilters(QWidget *panel);
    bool isTextInputWidget(const QObject *watched) const;
    void updateLayout();
    void reposition();
    void setPanel(PanelType panel);
    void syncFromConfig();
    void applySizeMode(bool ratioMode);
    void syncSizeModeControls();
    void persistRadius();
    void flushPendingPersist();
    void hideEvent(QHideEvent *event) override;
    void persistRatioType();
    void persistRememberSetting();
    void persistFixedSize();
    void captureCurrentSetting();
    void syncButtonStates();
    void syncRatioFromSelection();
    void updateDimensionFields();

    QWidget *m_groupFrame = nullptr;
    ToolButton *m_cornerButton = nullptr;
    ToolButton *m_ratioButton = nullptr;
    QWidget *m_cornerPanel = nullptr;
    QWidget *m_sizePanel = nullptr;
    DSlider *m_radiusSlider = nullptr;
    DSpinBox *m_radiusSpin = nullptr;
    DPushButton *m_sizeModeButton = nullptr;
    QAction *m_sizeAction = nullptr;
    QAction *m_ratioAction = nullptr;
    DLineEdit *m_widthEdit = nullptr;
    DLineEdit *m_heightEdit = nullptr;
    QIntValidator *m_widthValidator = nullptr;
    QIntValidator *m_heightValidator = nullptr;
    DIconButton *m_lockButton = nullptr;
    HintFilter *m_hintFilter = nullptr;

    QTimer *m_persistTimer = nullptr;
    int m_radius = 0;
    // 已经写入配置文件的值，避免无变化的重复落盘
    int m_persistedRadius = -1;
    bool m_ratioLocked = false;
    bool m_rememberSetting = false;
    bool m_pendingRememberedSetting = false;
    bool m_rememberedIsRatio = false;
    int m_fixedWidth = 0;
    int m_fixedHeight = 0;
    // 用户在比例模式中确认的宽高比，必须与像素尺寸分开保存，避免换算后被反算覆盖。
    int m_ratioWidth = 0;
    int m_ratioHeight = 0;
    int m_selectionWidth = 0;
    int m_selectionHeight = 0;
    PanelType m_panel = NoPanel;
    bool m_panelOnLeft = false;
    QRect m_lastSelection;
    QRect m_lastScreen;
    bool m_syncingSelectionSize = false;
    // 拖动状态：m_groupAnchor 始终记录按钮组可见底板的左上角（父窗口坐标系），
    // 面板展开/收起导致控件尺寸变化时据此回算控件原点，按钮组不会跟着跑。
    bool m_userMoved = false;
    bool m_dragging = false;
    bool m_panelCursorActive = false;
    Qt::CursorShape m_panelPreviousCursorShape = Qt::ArrowCursor;
    bool m_groupCursorActive = false;
    Qt::CursorShape m_groupPreviousCursorShape = Qt::ArrowCursor;
    QPoint m_dragMouseStart;
    QPoint m_dragWidgetStart;
    QPoint m_groupAnchor;
};

#endif // ROUNDEDCORNERWIDGET_H
