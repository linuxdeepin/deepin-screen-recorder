// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLEGLASSPANEL_H
#define TABLEGLASSPANEL_H

#include <DBlurEffectWidget>
#include <DWidget>

#include <QColor>
#include <QImage>
#include <QPointer>
#include <QVector>

DWIDGET_USE_NAMESPACE

/**
 * @brief 表格识别界面共用的毛玻璃面板
 *
 * 设计稿里卡片/提示条都带一层背景模糊（blur radius + saturation）与半透明填充，
 * 即毛玻璃效果。这里不再自绘：模糊、圆角、遮罩全部交给 DTK 现成的
 * DBlurEffectWidget 绘制，模糊源通过 DTK 的 DBlurEffectGroup 复用同一张
 * 贴图窗口截图。投影使用 DTK 的 DGraphicsGlowEffect。
 *
 *   - blurRadius   对应设计稿 blur.radius
 *   - maskColor    对应设计稿 fills 的颜色，alpha 即 fills 的 alpha
 *   - borderColor  对应设计稿 borders 的描边
 * 面板内部的控件一律使用 DTK 基础控件。
 */
class TableGlassPanel : public DBlurEffectWidget
{
    Q_OBJECT

public:
    TableGlassPanel(int blurRadius,
                    int cornerRadius,
                    const QColor &maskColor,
                    const QColor &borderColor,
                    DWidget *parent = nullptr);
    ~TableGlassPanel() override;

    /// 毛玻璃的背景来源：贴图窗口里那张截图（与窗口坐标一一对应）
    void setBackdropImage(const QImage &image);

    /**
     * @brief 背景截图对应的宿主窗口
     *
     * 面板作为贴图窗口的子控件时二者坐标一致，可以直接 mapTo；
     * 面板作为独立置顶浮层时（不跟随贴图窗口大小），需要用屏幕坐标换算。
     */
    void setBackdropHost(QWidget *host);

    /// 设计稿里的圆角块（对话框按钮 / 通知行动按钮）背景
    struct Pill {
        QRect rect;
        int radius = 6;
        QColor color;
    };
    void setPills(const QVector<Pill> &pills);
    void clearPills();

    /// 设计稿 border(position=1) 的白色内高光（玻璃边缘）
    void setHighlightColor(const QColor &color);

    /// 主题切换时更新卡片填充与描边（设计稿的浅色/深色两版取值）
    void setPanelColors(const QColor &maskColor, const QColor &borderColor);

    /**
     * @brief 设计稿 shadow：用 DTK 的 DGraphicsGlowEffect 给面板加投影
     *
     * 会在四周预留绘制投影所需的边距，卡片实体绘制在 inset 边距之后的区域内，
     * 因此调用方需要按 cardSize + 2*contentMargin() 设置控件尺寸。
     */
    void setSoftShadow(int blurRadius, int offsetY, const QColor &color);

    /// 为投影预留的边距（卡片实体向外扩展这么多像素）
    int contentMargin() const { return m_shadowMargin; }

    /**
     * @brief 按 DTK DDialog 的方式使用窗口级毛玻璃
     *
     * 模糊交给窗管的 BehindWindowBlend，填充色用 DTK 的 AutoColor + 80% alpha，
     * 圆角/边框/阴影由 DPlatformWindowHandle 绘制，控件本身不再预留投影边距。
     * 面板需要铺满顶层窗口时调用（成功弹窗、提示条）。
     */
    void useWindowBackdrop();

    int cornerRadius() const { return m_cornerRadius; }

protected:
    /**
     * @brief 把面板变成独立的不抢焦点置顶浮层窗口
     *
     * 面板尺寸固定为设计稿尺寸，不再随贴图窗口缩放；需要宿主窗口仅用于
     * 定位（相对宿主居中/底部居中）以及毛玻璃背景截图的坐标换算。
     */
    void makeFloatingOverlay();

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void updateBlurSource();
    void updateBlurOffset();
    QRect cardRect() const;

    int m_blurRadius;
    int m_cornerRadius;
    QColor m_maskColor;
    QColor m_borderColor;
    QColor m_highlightColor;
    QImage m_backdrop;
    QPointer<QWidget> m_backdropHost;
    QVector<Pill> m_pills;

    /// DTK 提供的模糊源共享容器
    DBlurEffectGroup *m_blurGroup = nullptr;
    QImage m_blurSource;
    int m_blurSourceRadius = -1;

    int m_shadowMargin = 0;
    bool m_windowBackdrop = false;
};

/// 设计稿 shadow 需要预留的绘制边距（卡片容器按此边距扩边）
int tableShadowMargin(int blurRadius, int offsetY);

#endif // TABLEGLASSPANEL_H
