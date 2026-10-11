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

class ToolButton;

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
    // 主窗口同步当前选区尺寸到尺寸/比例面板
    void setSelectionSize(int width, int height);

    // 生成带圆角透明通道的图片；radius <= 0 时返回原图
    static QPixmap applyRoundedCorners(const QPixmap &source, int radius);
    // JPG/BMP 等不支持透明格式：把含 Alpha 的成图合成到白底上，
    // 使圆角外区域呈现白色（投影等半透明像素也一并正确压在白色上）。
    static QPixmap compositeOnWhite(const QPixmap &source);

signals:
    void radiusChanged(int radius);
    // 用户在尺寸/比例面板中输入了新的选区尺寸
    void selectionSizeChanged(int width, int height);
    void panelExpandedChanged(bool expanded);

public slots:
    // 点击其它工具时收起属性面板（保留圆角与尺寸比例设置）
    void collapsePanels();

private:
    void initUi();
    QWidget *buildGroup();
    QWidget *buildCornerPanel();
    QWidget *buildSizePanel();
    void updateLayout();
    void reposition();
    void setPanel(PanelType panel);
    void syncFromConfig();
    void persistRadius();
    void flushPendingPersist();
    void hideEvent(QHideEvent *event) override;
    void persistRatioType();
    void persistRememberSetting();
    void persistFixedSize();
    void captureCurrentSetting();
    void syncButtonStates();
    void updateDimensionFields();

    QWidget *m_groupFrame = nullptr;
    ToolButton *m_cornerButton = nullptr;
    ToolButton *m_ratioButton = nullptr;
    QWidget *m_cornerPanel = nullptr;
    QWidget *m_sizePanel = nullptr;
    DSlider *m_radiusSlider = nullptr;
    DSpinBox *m_radiusSpin = nullptr;
    DPushButton *m_sizeModeButton = nullptr;
    DLineEdit *m_widthEdit = nullptr;
    DLineEdit *m_heightEdit = nullptr;
    QIntValidator *m_widthValidator = nullptr;
    QIntValidator *m_heightValidator = nullptr;
    DIconButton *m_lockButton = nullptr;

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
    int m_selectionWidth = 0;
    int m_selectionHeight = 0;
    PanelType m_panel = NoPanel;
    bool m_panelOnLeft = false;
    QRect m_lastSelection;
    QRect m_lastScreen;
    bool m_syncingSelectionSize = false;
};

#endif // ROUNDEDCORNERWIDGET_H
