// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "roundedcornerwidget.h"
#include "toolbutton.h"
#include "filter.h"
#include "tooltips.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../utils/log.h"

#include <DBlurEffectWidget>
#include <DFloatingWidget>
#include <DGuiApplicationHelper>
#include <DStyle>
#include <DStyleOptionButton>
#include <DStylePainter>
#include <DLabel>
#include <DMenu>
#include <DSlider>
#include <DSpinBox>
#include <DFontSizeManager>

#include <QActionGroup>
#include <QApplication>
#include <QMouseEvent>
#include <QLineEdit>
#include <QIntValidator>
#include <QPainter>
#include <QPainterPath>
#include <QProxyStyle>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QIcon>
#include <QHideEvent>

DWIDGET_USE_NAMESPACE

namespace {

// DPushButton 挂 QMenu 后会按默认样式同时预留文字和菜单箭头空间；
// 56px 宽的中文“尺寸/比例”在 T6 字体下会被最后一个字裁掉。
// 保留 DTK 按钮背景绘制，只重新分配内部文字与箭头区域。
// Sketch 中尺寸/比例下拉弹层为 152x64、圆角 12px，两行菜单项各 26px，
// 上下留白各 6px。DTK 默认 QMenu 的纵向留白会把它撑到 104px 高，
// 因此只对这一个菜单使用紧凑尺寸策略，不影响应用内其他菜单。
class SizeModeMenuStyle : public QProxyStyle
{
public:
    explicit SizeModeMenuStyle(QObject *parent)
        // 和圆角滑块样式同理：不接管应用样式所有权，只在绘制时动态委托。
        : QProxyStyle(nullptr)
    {
        setParent(parent);
    }

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override
    {
        switch (metric) {
        case PM_MenuHMargin:
        case PM_MenuVMargin:
            return 6;
        case PM_MenuPanelWidth:
        case PM_MenuTearoffHeight:
        case PM_MenuDesktopFrameWidth:
            return 0;
        default:
            return QProxyStyle::pixelMetric(metric, option, widget);
        }
    }

    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &contentsSize, const QWidget *widget) const override
    {
        if (type == CT_MenuItem) {
            QSize size = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
            size.setHeight(26);
            // QMenu 用内容宽度计算 actionGeometry。DTK 默认内容宽度会把
            // 菜单项撑到 162px，右侧被 152px 的菜单裁掉，hover 因此只有
            // 左边距、没有右边距；这里收敛到菜单内宽 140px，左右各留 6px。
            size.setWidth(140);
            return size;
        }
        if (type == CT_Menu) {
            return QSize(152, 64);
        }
        return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
    }
};

class SizeModeButton : public DPushButton
{
public:
    explicit SizeModeButton(QWidget *parent = nullptr)
        : DPushButton(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QStylePainter painter(this);
        QStyleOptionButton option;
        initStyleOption(&option);
        // 只关闭默认菜单指示器，保留 Flat 等按钮状态，避免 DTK
        // 把原本透明的扁平按钮画成实心深色背景。文字和箭头在下方自行绘制。
        option.text.clear();
        option.icon = QIcon();
        option.features.setFlag(QStyleOptionButton::HasMenu, false);
        painter.drawControl(QStyle::CE_PushButton, option);

        // 对齐 Sketch：背景 56x24，文字从 x=4 开始，箭头图标区域
        // 位于 x=36、16x16。按钮总宽保持不变，只收紧内部留白。
        constexpr int arrowSize = 16;
        constexpr int arrowRight = 4;
        constexpr int leftPadding = 4;
        constexpr int textArrowGap = 2;

        const QRect arrowRect(width() - arrowRight - arrowSize,
                              (height() - arrowSize) / 2,
                              arrowSize,
                              arrowSize);
        const QRect textRect(leftPadding,
                             0,
                             qMax(0, arrowRect.left() - textArrowGap - leftPadding),
                             height());

        const QPalette::ColorGroup colorGroup = isEnabled()
                ? QPalette::Normal : QPalette::Disabled;
        painter.setPen(palette().color(colorGroup, QPalette::ButtonText));
        painter.setFont(font());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());

        QStyleOption arrowOption;
        arrowOption.initFrom(this);
        arrowOption.rect = arrowRect;
        arrowOption.state = option.state;
        style()->drawPrimitive(QStyle::PE_IndicatorArrowDown,
                               &arrowOption,
                               &painter,
                               this);
    }
};

// 设计稿「截图录屏-工具栏-圆角 / 比例」尺寸（24x24 图标按 32px 资源提供，控件按 1x 逻辑像素实现）
const int GROUP_WIDTH = 56;
const int GROUP_HEIGHT = 103;
const int BUTTON_SIZE = 36;
const int BUTTON_ICON_SIZE = 24;
const int BUTTON_MARGIN_X = 10;
const int FIRST_BUTTON_Y = 11;
const int SECOND_BUTTON_Y = 57;

const int PANEL_WIDTH = 270;
const int PANEL_HEIGHT = 56;
const int PANEL_RADIUS = 12;
const int PANEL_PADDING = 10;

const int SLIDER_X = 36;
const int SLIDER_Y = 16;
const int SLIDER_WIDTH = 152;
const int SLIDER_HEIGHT = 24;
const int SPIN_WIDTH = 62;
const int SPIN_HEIGHT = 36;
const int ADJUST_ICON_SIZE = 16;

const int SIZE_MODE_X = 10;
const int SIZE_MODE_Y = 16;
const int SIZE_MODE_WIDTH = 56;
const int SIZE_MODE_HEIGHT = 24;
const int SIZE_MODE_RADIUS = 6;
const int SIZE_MODE_INPUT_GAP = 8;
const int WIDTH_SPIN_X = SIZE_MODE_X + SIZE_MODE_WIDTH + SIZE_MODE_INPUT_GAP;
const int HEIGHT_SPIN_X = 154;
const int DIMENSION_SPIN_WIDTH = 60;
const int DIMENSION_SEPARATOR_X = 136;
const int DIMENSION_SEPARATOR_Y = 19;
const int DIMENSION_SEPARATOR_SIZE = 16;
const int LOCK_BUTTON_X = 224;

const int RADIUS_MAX = 60;
const int MAX_DIMENSION_VALUE = 99999;
const int MAX_RATIO_VALUE = 999;
// 按钮组与选区、按钮组与属性面板的间距
const int SELECTION_GAP = 16;
const int PANEL_GAP = 10;
// 按钮组显示在选区内部时，与选区上边缘、右边缘保留的间距
const int INNER_SELECTION_GAP = 10;

// 把单色图标整体着色成指定颜色（保留原始 alpha 形状）。
// 图标资源本身固定是 #000000，DTK 对普通 QIcon 不会按 palette 重新着色，
// 所以选中态（蓝底白图标）需要在这里显式着色。
QIcon tintedIcon(const QIcon &icon, const QSize &iconSize, const QColor &color)
{
    if (icon.isNull()) {
        return icon;
    }
    const qreal dpr = qApp->devicePixelRatio();
    const QPixmap base = icon.pixmap(iconSize, dpr);
    if (base.isNull()) {
        return icon;
    }
    QPixmap tinted(base.size());
    tinted.fill(Qt::transparent);
    tinted.setDevicePixelRatio(dpr);
    QPainter painter(&tinted);
    painter.drawPixmap(0, 0, base);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(tinted.rect(), color);
    painter.end();
    return QIcon(tinted);
}

// 设计稿「截图录屏-工具栏-比例2（激活）」中的图钉按钮：
// 36x36、圆角 8；normal 黑色 10% 背景 + 深色图标、hover 20% 背景、
// press 更深背景 + 主题高亮色图标、active（选中）主题高亮色背景 + 白色图标。
// 按钮仍继承 DTK DIconButton，只重写背景和图标着色，保证交互行为复用 DTK。
class SelectionLockButton : public DIconButton
{
public:
    explicit SelectionLockButton(QWidget *parent = nullptr)
        : DIconButton(parent)
    {
        setCheckable(true);
        setFlat(true);
    }

protected:
    void initStyleOption(DStyleOptionButton *option) const override
    {
        DIconButton::initStyleOption(option);
        // 选中态配色由本类自己绘制，不再交给 DTK 的蓝色选中态处理，避免与
        // 自绘背景叠加出两层高亮。
        option->state &= ~QStyle::State_On;
    }

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        // 本应用通过 setApplicationPalette() 套用系统主题，DGuiApplicationHelper::themeType()
        // 不一定会跟随实际调色板（本机出现 palette 为深色但 themeType() 仍报浅色，
        // 导致深色模式下钉住图标被涂成纯黑）。这里直接按调色板判断，与 ToolButton 保持一致。
        const bool darkTheme = palette().color(QPalette::Window).lightness() < 128;
        const bool active = isChecked();

        // 设计稿：normal #000000 10%、hover #000000 20%，press 再深一档；
        // 深色主题下换成同比例的白色蒙层，保证在深色底板上依然可辨。
        QColor background;
        if (active) {
            background = palette().highlight().color();
        } else if (isDown()) {
            background = darkTheme ? QColor(255, 255, 255, 51) : QColor(0, 0, 0, 64);
        } else if (underMouse()) {
            background = darkTheme ? QColor(255, 255, 255, 38) : QColor(0, 0, 0, 51);
        } else {
            background = darkTheme ? QColor(255, 255, 255, 26) : QColor(0, 0, 0, 26);
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        painter.drawRoundedRect(rect(), 8, 8);
        painter.end();

        DStylePainter stylePainter(this);
        DStyleOptionButton option;
        initStyleOption(&option);

        const QSize iconSize = option.iconSize.isValid() ? option.iconSize
                                                         : QSize(BUTTON_ICON_SIZE, BUTTON_ICON_SIZE);
        // selection-fixed 不会被 builtin 图标引擎自动重着色，需按当前调色板手工着色。
        QColor iconColor;
        if (active) {
            iconColor = option.palette.highlightedText().color();
        } else if (isDown()) {
            iconColor = option.palette.highlight().color();
        } else {
            iconColor = darkTheme ? QColor(255, 255, 255) : QColor(0, 0, 0);
        }
        option.icon = tintedIcon(option.icon, iconSize, iconColor);

        stylePainter.drawControl(DStyle::CE_IconButton, option);
    }
};

QColor blurMaskColor()
{
    // 与主工具栏保持一致的模糊背景：浅色 #F7F7F7 60%，深色 #000000 30%
    if (Utils::themeType == 1) {
        return QColor(0xF7, 0xF7, 0xF7, 153);
    }
    return QColor(0, 0, 0, 76);
}

// Qt6 用 globalPosition()，Qt5 只有 globalPos()，拖动时取鼠标的全局坐标。
QPoint eventGlobalPos(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->globalPosition().toPoint();
#else
    return event->globalPos();
#endif
}

// 只替换当前 override cursor，不反复 push，避免鼠标移动过程中光标栈持续增长。
void applyOverrideCursorShape(Qt::CursorShape shape)
{
    const QCursor *current = QApplication::overrideCursor();
    if (!current) {
        QApplication::setOverrideCursor(QCursor(shape));
    } else if (current->shape() != shape) {
        QApplication::changeOverrideCursor(QCursor(shape));
    }
}

// 按钮组水平位置的①/②/③判定（设计稿 2.3）：
// reposition() 与 groupPlateRect() 共用，避免两处几何计算出现偏差。
struct GroupPlacement
{
    int groupLeft = 0;          // 按钮组可见底板相对父窗口的左边
    int groupTop = 0;           // 按钮组可见底板相对父窗口的上边
    bool panelOnLeft = false;   // 属性面板是否向左展开
    bool insideSelection = false; // 两侧空间不足，按钮组显示在选区内部
};

GroupPlacement computeGroupPlacement(const QRect &selection, const QRect &screen)
{
    const int totalWidth = GROUP_WIDTH + PANEL_GAP + PANEL_WIDTH;
    GroupPlacement placement;
    if (selection.right() + SELECTION_GAP + totalWidth <= screen.right()) {
        // ① 选区右侧空间足够：按钮组在选区右上角右侧，面板向右展开
        placement.groupLeft = selection.right() + SELECTION_GAP;
        placement.groupTop = selection.top();
    } else if (selection.left() - SELECTION_GAP - totalWidth >= screen.left()) {
        // ② 选区左侧空间足够：按钮组在选区左上角左侧，面板向左展开
        placement.panelOnLeft = true;
        placement.groupLeft = selection.left() - SELECTION_GAP - GROUP_WIDTH;
        placement.groupTop = selection.top();
    } else {
        // ③ 两侧均不足：按钮组显示在选区右上角内部，
        // 与选区上边缘、右边缘各保留 10px 间距，面板向左展开。
        placement.panelOnLeft = true;
        placement.insideSelection = true;
        placement.groupLeft = selection.right() - INNER_SELECTION_GAP - GROUP_WIDTH + 1;
        placement.groupTop = selection.top() + INNER_SELECTION_GAP;
    }
    return placement;
}

// DFloatingWidget 会在内容四周预留阴影留白（常规档位为 6px），
// 设计稿给出的 56x103 / 270x56 指的是可见底板，控件尺寸要再加上两侧留白。
int plateShadowMargin(const QWidget *widget)
{
    return DStyle::pixelMetric(widget->style(), DStyle::PM_FloatingWidgetShadowMargins, nullptr, widget);
}

QWidget *createFloatingPlate(QWidget *parent, const QSize &plateSize, int radius, int margin)
{
    // 与主工具栏（ToolBarWidget/DFloatingWidget）保持同一套悬浮样式：
    // 高斯模糊背景 + DTK 悬浮阴影（设计稿：偏移 0/4、模糊 8、10% 黑）
    DFloatingWidget *plate = new DFloatingWidget(parent);
    plate->setBlurBackgroundEnabled(true);
    plate->blurBackground()->setRadius(30);
    plate->blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    plate->blurBackground()->setBlurEnabled(true);
    plate->blurBackground()->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    plate->blurBackground()->setMaskColor(blurMaskColor());
    plate->setFramRadius(radius);
    plate->setFixedSize(plateSize.width() + margin * 2, plateSize.height() + margin * 2);
    return plate;
}

// 输入框/Spinbox 的底板颜色需要跟随主题：
// 浅色主题沿用设计稿的 #FFFFFF 60%；深色主题使用 #FFFFFF 10%，
// 避免深色模式下仍显示浅色底板，同时保证与半透明深色面板有可见区分。
QColor inputPlateColor(const QWidget *widget)
{
    const bool darkTheme = widget->palette().color(QPalette::Window).lightness() < 128;
    return darkTheme ? QColor(255, 255, 255, 26) : QColor(255, 255, 255, 153);
}

void applySpinBoxPlateColor(DSpinBox *spin)
{
    QPalette pal = spin->palette();
    pal.setColor(QPalette::Button, inputPlateColor(spin));
    spin->setPalette(pal);
}

void applyLineEditPlateColor(DLineEdit *edit)
{
    if (!edit) {
        return;
    }

    const QColor plateColor = inputPlateColor(edit);
    QPalette pal = edit->palette();
    pal.setColor(QPalette::Base, plateColor);
    pal.setColor(QPalette::Button, plateColor);
    edit->setPalette(pal);

    if (QLineEdit *lineEdit = edit->lineEdit()) {
        QPalette lineEditPal = lineEdit->palette();
        lineEditPal.setColor(QPalette::Base, plateColor);
        lineEditPal.setColor(QPalette::Button, plateColor);
        lineEdit->setPalette(lineEditPal);
    }
}

int greatestCommonDivisor(int a, int b)
{
    a = qAbs(a);
    b = qAbs(b);
    while (b != 0) {
        const int remainder = a % b;
        a = b;
        b = remainder;
    }
    return qMax(1, a);
}

QPair<int, int> simplifiedRatio(int width, int height)
{
    const int divisor = greatestCommonDivisor(width, height);
    return {qMax(1, width / divisor), qMax(1, height / divisor)};
}

DLabel *createIconLabel(QWidget *parent, const QString &iconName, const QString &objectName,
                        const QSize &size, const QSize &iconSize)
{
    DLabel *label = new DLabel(parent);
    label->setObjectName(objectName);
    label->setAccessibleName(objectName);
    label->setFixedSize(size);
    label->setAlignment(Qt::AlignCenter);
    label->setPixmap(QIcon::fromTheme(iconName).pixmap(iconSize));
    return label;
}

}

RoundedCornerWidget::RoundedCornerWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    m_hintFilter = new HintFilter(this);
    const int margin = plateShadowMargin(this);
    setFixedSize(GROUP_WIDTH + margin * 2, GROUP_HEIGHT + margin * 2);
    initUi();
    syncFromConfig();
}

RoundedCornerWidget::~RoundedCornerWidget()
{
    flushPendingPersist();
}

void RoundedCornerWidget::initUi()
{
    m_groupFrame = buildGroup();
    // 底板（按钮组）悬停时显示手型。主窗口在选区内部使用全局 override cursor，
    // 会覆盖控件自己的 cursor，因此按钮组及子控件也需要主动同步手型。
    m_groupFrame->setCursor(Qt::OpenHandCursor);
    m_groupFrame->installEventFilter(this);
    const QList<QWidget *> groupChildren = m_groupFrame->findChildren<QWidget *>();
    for (QWidget *child : groupChildren) {
        child->installEventFilter(this);
    }
    m_cornerPanel = buildCornerPanel();
    m_sizePanel = buildSizePanel();
    // 主窗口会在选区内部使用“拖动选区”的全局手型，面板必须在该区域
    // 主动保护为普通箭头，否则手型会覆盖面板及其子控件的光标。输入框
    // 内部的 QLineEdit 也要单独监听，才能在其上恢复 I 型文本光标。
    installPanelCursorFilters(m_cornerPanel);
    installPanelCursorFilters(m_sizePanel);

    m_cornerPanel->hide();
    m_sizePanel->hide();

    // 圆角按钮：单击只展开/收起属性面板并高亮按钮。
    // 圆角半径默认保持 0（设计稿：0px~60px，默认 0），需要用户拖动滑块或输入数值才真正生效。
    connect(m_cornerButton, &ToolButton::clicked, this, [this]() {
        setPanel(m_panel == CornerPanel ? NoPanel : CornerPanel);
    });

    // 尺寸/比例按钮：展开/收起尺寸比例面板
    connect(m_ratioButton, &ToolButton::clicked, this, [this]() {
        setPanel(m_panel == SizePanel ? NoPanel : SizePanel);
    });

    // 拖动滑块时每一步都写配置会造成明显卡顿：QSettings::sync() 在 /home 上单次实测
    // 可达 150ms 以上。这里改为防抖落盘（松手/输入完成/控件隐藏时立即写入）。
    m_persistTimer = new QTimer(this);
    m_persistTimer->setSingleShot(true);
    m_persistTimer->setInterval(300);
    connect(m_persistTimer, &QTimer::timeout, this, &RoundedCornerWidget::persistRadius);

    connect(m_radiusSlider, &DSlider::valueChanged, this, [this](int value) {
        if (m_radiusSpin->value() != value) {
            m_radiusSpin->setValue(value);
        }
        if (m_radius == value) {
            return;
        }
        m_radius = value;
        m_persistTimer->start();
        emit radiusChanged(m_radius);
    });
    // 松开滑块 / 输入框结束编辑：立刻落盘，保证拖完就记住
    connect(m_radiusSlider, &DSlider::sliderReleased, this, [this]() {
        flushPendingPersist();
    });
    connect(m_radiusSpin, QOverload<int>::of(&DSpinBox::valueChanged), this, [this](int value) {
        if (m_radiusSlider->value() != value) {
            m_radiusSlider->setValue(value);
        }
    });
    connect(m_radiusSpin, &DSpinBox::editingFinished, this, [this]() {
        flushPendingPersist();
    });

    // 面板输入框里回车代表“确认当前数值”：确认后主动移出焦点，
    // 这样紧接着再按一次回车才会轮到主窗口的“保存截图”快捷方式。
    const QList<QLineEdit *> panelEditors = {
        m_radiusSpin->lineEdit(),
        m_widthEdit->lineEdit(),
        m_heightEdit->lineEdit(),
    };
    for (QLineEdit *editor : panelEditors) {
        if (!editor) {
            continue;
        }
        connect(editor, &QLineEdit::returnPressed, this, [editor]() {
            editor->clearFocus();
        });
    }

    // 宽/高输入框用 DLineEdit，整段编辑完成（回车或失焦）后再提交，避免输入中途反复改选区。
    connect(m_widthEdit, &DLineEdit::editingFinished, this, [this]() {
        if (m_syncingSelectionSize) {
            return;
        }

        const int maxValue = m_ratioLocked ? MAX_RATIO_VALUE : MAX_DIMENSION_VALUE;
        const int widthValue = qBound(1, m_widthEdit->text().toInt(), maxValue);
        const int heightValue = qBound(1, m_heightEdit->text().toInt(), maxValue);
        if (m_ratioLocked) {
            // 比例是选区拖动的约束条件，不是像素尺寸。原样保存用户输入，
            // 不能用当前像素尺寸换算后再反算，否则 9:16 会被改写成其他比例。
            m_ratioWidth = widthValue;
            m_ratioHeight = heightValue;
            updateDimensionFields();
            persistFixedSize();
            emit ratioChanged(m_ratioWidth, m_ratioHeight);
            return;
        }
        m_selectionWidth = widthValue;
        m_selectionHeight = heightValue;
        updateDimensionFields();
        persistFixedSize();
        emit selectionSizeChanged(m_selectionWidth, m_selectionHeight);
    });
    connect(m_heightEdit, &DLineEdit::editingFinished, this, [this]() {
        if (m_syncingSelectionSize) {
            return;
        }

        const int maxValue = m_ratioLocked ? MAX_RATIO_VALUE : MAX_DIMENSION_VALUE;
        const int widthValue = qBound(1, m_widthEdit->text().toInt(), maxValue);
        const int heightValue = qBound(1, m_heightEdit->text().toInt(), maxValue);
        if (m_ratioLocked) {
            m_ratioWidth = widthValue;
            m_ratioHeight = heightValue;
            updateDimensionFields();
            persistFixedSize();
            emit ratioChanged(m_ratioWidth, m_ratioHeight);
            return;
        }
        m_selectionWidth = widthValue;
        m_selectionHeight = heightValue;
        updateDimensionFields();
        persistFixedSize();
        emit selectionSizeChanged(m_selectionWidth, m_selectionHeight);
    });

    connect(m_lockButton, &DIconButton::clicked, this, [this]() {
        m_rememberSetting = m_lockButton->isChecked();
        if (m_rememberSetting) {
            // 图钉只负责“下次启动继续使用”，当前这次框选仍由用户自己控制。
            // 勾选时把面板当前值固化为记忆值，避免只有图钉状态而没有参数。
            captureCurrentSetting();
        }
        m_pendingRememberedSetting = false;
        persistRememberSetting();
        persistFixedSize();
        syncButtonStates();
    });
}

QWidget *RoundedCornerWidget::buildGroup()
{
    const int margin = plateShadowMargin(this);
    QWidget *group = createFloatingPlate(this, QSize(GROUP_WIDTH, GROUP_HEIGHT), PANEL_RADIUS, margin);

    // 与横向工具栏复用完全相同的 ToolTips/HintFilter，保证提示的样式、
    // 延迟时间和“按钮上方居中”的定位规则一致。
    const auto installTipHint = [this](QWidget *button, const QString &text) {
        auto *hintWidget = new ToolTips("", window());
        hintWidget->hide();
        hintWidget->setText(text);
        button->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hintWidget));
        button->setProperty("HintAnchorWidget", QVariant::fromValue<QWidget *>(m_cornerButton));
        button->installEventFilter(m_hintFilter);
    };

    m_cornerButton = new ToolButton(group);
    m_cornerButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_cornerButton->setIconSize(QSize(BUTTON_ICON_SIZE, BUTTON_ICON_SIZE));
    m_cornerButton->setCheckable(true);
    m_cornerButton->setIcon(QIcon::fromTheme("corner-radius"));
    m_cornerButton->setObjectName("RoundedCornerButton");
    m_cornerButton->setAccessibleName("RoundedCornerButton");
    installTipHint(m_cornerButton, tr("Radius"));
    m_cornerButton->move(margin + BUTTON_MARGIN_X, margin + FIRST_BUTTON_Y);

    m_ratioButton = new ToolButton(group);
    m_ratioButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_ratioButton->setIconSize(QSize(BUTTON_ICON_SIZE, BUTTON_ICON_SIZE));
    m_ratioButton->setCheckable(true);
    m_ratioButton->setIcon(QIcon::fromTheme("size-ratio"));
    m_ratioButton->setObjectName("SizeRatioButton");
    m_ratioButton->setAccessibleName("SizeRatioButton");
    installTipHint(m_ratioButton, tr("Size / ratio"));
    m_ratioButton->move(margin + BUTTON_MARGIN_X, margin + SECOND_BUTTON_Y);

    return group;
}

QWidget *RoundedCornerWidget::buildCornerPanel()
{
    const int margin = plateShadowMargin(this);
    QWidget *panel = createFloatingPlate(this, QSize(PANEL_WIDTH, PANEL_HEIGHT), PANEL_RADIUS, margin);

    DLabel *leading = createIconLabel(panel, "corner-adjust", "CornerAdjustIcon",
                                      QSize(ADJUST_ICON_SIZE, ADJUST_ICON_SIZE),
                                      QSize(ADJUST_ICON_SIZE, ADJUST_ICON_SIZE));
    leading->move(margin + PANEL_PADDING, margin + (PANEL_HEIGHT - ADJUST_ICON_SIZE) / 2);

    m_radiusSlider = new DSlider(Qt::Horizontal, panel);
    m_radiusSlider->setMinimum(0);
    m_radiusSlider->setMaximum(RADIUS_MAX);
    m_radiusSlider->setFixedSize(SLIDER_WIDTH, SLIDER_HEIGHT);
    m_radiusSlider->setObjectName("CornerRadiusSlider");
    m_radiusSlider->setAccessibleName("CornerRadiusSlider");
    m_radiusSlider->move(margin + SLIDER_X, margin + SLIDER_Y);
    // 圆角滑动条使用 DTK DSlider 的默认样式。
    QSlider *radiusSlider = m_radiusSlider->slider();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            radiusSlider, qOverload<>(&QWidget::update));

    m_radiusSpin = new DSpinBox(panel);
    m_radiusSpin->setEnabledEmbedStyle(true);
    applySpinBoxPlateColor(m_radiusSpin);
    m_radiusSpin->setRange(0, RADIUS_MAX);
    m_radiusSpin->setFixedSize(SPIN_WIDTH, SPIN_HEIGHT);
    m_radiusSpin->setObjectName("CornerRadiusSpinBox");
    m_radiusSpin->setAccessibleName("CornerRadiusSpinBox");
    m_radiusSpin->move(margin + PANEL_WIDTH - PANEL_PADDING - SPIN_WIDTH,
                       margin + (PANEL_HEIGHT - SPIN_HEIGHT) / 2);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() { applySpinBoxPlateColor(m_radiusSpin); });

    return panel;
}

QWidget *RoundedCornerWidget::buildSizePanel()
{
    const int margin = plateShadowMargin(this);
    QWidget *panel = createFloatingPlate(this, QSize(PANEL_WIDTH, PANEL_HEIGHT), PANEL_RADIUS, margin);

    // 设计稿「尺寸 ▾」：56x24，前面是文字，后面是下拉箭头。
    // 用 DTK 的 DPushButton（QPushButton 派生）挂 DMenu，按钮自带“文本 + 下拉箭头”，
    // 不去自绘箭头，也不依赖 ComboBox 的额外内边距。
    m_sizeModeButton = new SizeModeButton(panel);
    m_sizeModeButton->setFlat(true);
    // 下拉按钮背景宽 56，与右侧宽度输入框保留 8px 间距。
    m_sizeModeButton->setFixedSize(SIZE_MODE_WIDTH, SIZE_MODE_HEIGHT);
    // 下拉按钮沿用 DTK 默认背景，只把背景圆角调整为 6px。
    DStyle::setFrameRadius(m_sizeModeButton, SIZE_MODE_RADIUS);
    m_sizeModeButton->setObjectName("SizeRatioModeButton");
    m_sizeModeButton->setAccessibleName("SizeRatioModeButton");
    m_sizeModeButton->setToolTip(tr("Size / ratio"));
    m_sizeModeButton->setText(tr("Size"));
    DFontSizeManager::instance()->bind(m_sizeModeButton, DFontSizeManager::T6);

    DMenu *modeMenu = new DMenu(m_sizeModeButton);
    modeMenu->setStyle(new SizeModeMenuStyle(modeMenu));
    modeMenu->setFixedSize(152, 64);
    DStyle::setFrameRadius(modeMenu, 12);
    m_sizeAction = modeMenu->addAction(tr("Size"));
    m_ratioAction = modeMenu->addAction(tr("Ratio"));
    m_sizeAction->setCheckable(true);
    m_ratioAction->setCheckable(true);
    m_sizeAction->setChecked(true);
    QActionGroup *modeGroup = new QActionGroup(modeMenu);
    modeGroup->setExclusive(true);
    modeGroup->addAction(m_sizeAction);
    modeGroup->addAction(m_ratioAction);

    // 不用 QActionGroup::triggered 间接分发：直接监听每个动作，重复选择当前项时
    // 也会重新同步输入框，避免菜单勾选状态和实际数据显示偶发不一致。
    connect(m_sizeAction, &QAction::triggered, this, [this]() {
        applySizeMode(false);
    });
    connect(m_ratioAction, &QAction::triggered, this, [this]() {
        applySizeMode(true);
    });
    // 菜单显示前从唯一状态源回写勾选状态，防止外部配置切换或上一次
    // 编辑提交造成 action 状态与实际模式暂时不同步。
    connect(modeMenu, &QMenu::aboutToShow, this, &RoundedCornerWidget::syncSizeModeControls);
    m_sizeModeButton->setMenu(modeMenu);
    m_sizeModeButton->move(margin + SIZE_MODE_X, margin + SIZE_MODE_Y);

    // 宽/高输入框：设计稿 60x36 的 DTK 输入框，无上下调节按钮，数字居中。
    m_widthEdit = new DLineEdit(panel);
    applyLineEditPlateColor(m_widthEdit);
    m_widthEdit->setFixedSize(DIMENSION_SPIN_WIDTH, SPIN_HEIGHT);
    m_widthEdit->setClearButtonEnabled(false);
    m_widthEdit->setObjectName("SelectionWidthEdit");
    m_widthEdit->setAccessibleName("SelectionWidthEdit");
    m_widthEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_widthValidator = new QIntValidator(1, MAX_DIMENSION_VALUE, m_widthEdit);
    m_widthEdit->lineEdit()->setValidator(m_widthValidator);
    DFontSizeManager::instance()->bind(m_widthEdit, DFontSizeManager::T6);
    m_widthEdit->move(margin + WIDTH_SPIN_X, margin + (PANEL_HEIGHT - SPIN_HEIGHT) / 2);

    DLabel *separator = new DLabel(QString(QChar(0x00D7)), panel);
    separator->setFixedSize(DIMENSION_SEPARATOR_SIZE, DIMENSION_SEPARATOR_SIZE);
    separator->setAlignment(Qt::AlignCenter);
    separator->setObjectName("SelectionSizeSeparator");
    separator->setAccessibleName("SelectionSizeSeparator");
    DFontSizeManager::instance()->bind(separator, DFontSizeManager::T6);
    separator->move(margin + DIMENSION_SEPARATOR_X, margin + DIMENSION_SEPARATOR_Y);

    m_heightEdit = new DLineEdit(panel);
    applyLineEditPlateColor(m_heightEdit);
    m_heightEdit->setFixedSize(DIMENSION_SPIN_WIDTH, SPIN_HEIGHT);
    m_heightEdit->setClearButtonEnabled(false);
    m_heightEdit->setObjectName("SelectionHeightEdit");
    m_heightEdit->setAccessibleName("SelectionHeightEdit");
    m_heightEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_heightValidator = new QIntValidator(1, MAX_DIMENSION_VALUE, m_heightEdit);
    m_heightEdit->lineEdit()->setValidator(m_heightValidator);
    DFontSizeManager::instance()->bind(m_heightEdit, DFontSizeManager::T6);
    m_heightEdit->move(margin + HEIGHT_SPIN_X, margin + (PANEL_HEIGHT - SPIN_HEIGHT) / 2);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this]() {
                applyLineEditPlateColor(m_widthEdit);
                applyLineEditPlateColor(m_heightEdit);
            });

    m_lockButton = new SelectionLockButton(panel);
    m_lockButton->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_lockButton->setIconSize(QSize(BUTTON_ICON_SIZE, BUTTON_ICON_SIZE));
    m_lockButton->setIcon(QIcon::fromTheme("selection-fixed"));
    m_lockButton->setObjectName("RatioLockButton");
    m_lockButton->setAccessibleName("RatioLockButton");
    m_lockButton->setToolTip(tr("Keep the selected size or ratio for the next screenshot"));
    DStyle::setFocusRectVisible(m_lockButton, false);
    m_lockButton->move(margin + LOCK_BUTTON_X, margin + (PANEL_HEIGHT - BUTTON_SIZE) / 2);

    return panel;
}

void RoundedCornerWidget::syncFromConfig()
{
    ConfigSettings *config = ConfigSettings::instance();
    m_radius = config->getValue("rounded", "radius").toInt();
    m_radius = qBound(0, m_radius, RADIUS_MAX);
    m_persistedRadius = m_radius;
    m_radiusSlider->setValue(m_radius);
    m_radiusSpin->setValue(m_radius);

    // ratio_type 只表示面板当前下拉选择：0 尺寸、1 比例；图钉单独用 remember 保存。
    // 此前两者共用 ratio_type，导致仅切换“比例”也会在下次启动被当成已勾选图钉。
    const bool dropdownRatio = config->getValue("rounded", "ratio_type").toInt() > 0;
    m_ratioLocked = dropdownRatio;
    m_rememberSetting = config->getValue("rounded", "remember").toBool();
    m_rememberedIsRatio = config->getValue("rounded", "remember_ratio").toBool();
    m_fixedWidth = qMax(0, config->getValue("rounded", "fixed_width").toInt());
    m_fixedHeight = qMax(0, config->getValue("rounded", "fixed_height").toInt());

    // 图钉选中时，以 remember_ratio 作为权威模式回显：这样下次打开时
    // 图钉的 checked 状态、下拉菜单的 checked 项和实际要应用的参数三者一致。
    if (m_rememberSetting) {
        m_ratioLocked = m_rememberedIsRatio;
        if (dropdownRatio != m_ratioLocked) {
            persistRatioType();
        }
    }
    m_ratioWidth = 0;
    m_ratioHeight = 0;
    if (m_ratioLocked && m_rememberedIsRatio && m_fixedWidth > 0 && m_fixedHeight > 0) {
        // 比例模式下配置保存的 fixed_width/fixed_height 就是比例分子/分母。
        m_ratioWidth = m_fixedWidth;
        m_ratioHeight = m_fixedHeight;
    }
    m_pendingRememberedSetting = m_rememberSetting && m_fixedWidth > 0 && m_fixedHeight > 0;

    m_lockButton->setChecked(m_rememberSetting);
    syncSizeModeControls();
    updateDimensionFields();
    syncButtonStates();
}

void RoundedCornerWidget::applySizeMode(bool ratioMode)
{
    if (!m_sizeModeButton || !m_sizeAction || !m_ratioAction) {
        return;
    }

    // 菜单可能在输入框仍持有焦点时执行。先显式结束编辑，让旧模式下的最后一次
    // 输入提交完成，再切换模式并刷新字段，避免迟到的 editingFinished 覆盖比例数据。
    if (m_widthEdit && m_widthEdit->lineEdit()->hasFocus()) {
        m_widthEdit->lineEdit()->clearFocus();
    }
    if (m_heightEdit && m_heightEdit->lineEdit()->hasFocus()) {
        m_heightEdit->lineEdit()->clearFocus();
    }

    const bool changed = (m_ratioLocked != ratioMode);
    if (ratioMode && changed) {
        // 从尺寸模式切到比例模式时，以当前选区比例作为新的约束起点。
        syncRatioFromSelection();
    }
    m_ratioLocked = ratioMode;
    if (changed) {
        persistRatioType();
        // 图钉已经选中时，前面切换“尺寸/比例”也属于修改记忆设置，
        // 立即同步 remember_ratio，保证图钉、下拉选项和实际参数一致。
        if (m_rememberSetting) {
            captureCurrentSetting();
        }
    }

    syncSizeModeControls();
    updateDimensionFields();
    syncButtonStates();
    if (ratioMode && m_ratioWidth > 0 && m_ratioHeight > 0) {
        emit ratioChanged(m_ratioWidth, m_ratioHeight);
    }
}

void RoundedCornerWidget::syncSizeModeControls()
{
    if (!m_sizeModeButton || !m_sizeAction || !m_ratioAction) {
        return;
    }
    QAction *activeAction = m_ratioLocked ? m_ratioAction : m_sizeAction;
    m_sizeAction->setChecked(!m_ratioLocked);
    m_ratioAction->setChecked(m_ratioLocked);
    m_sizeModeButton->setText(activeAction->text());
}

void RoundedCornerWidget::syncButtonStates()
{
    // 竖排工具组是单选语义：只有当前展开的属性面板对应按钮高亮。
    // 半径值、比例锁定是持久化设置，不代表按钮应长期处于选中态。
    m_cornerButton->setChecked(m_panel == CornerPanel);
    m_ratioButton->setChecked(m_panel == SizePanel);
}

void RoundedCornerWidget::persistRadius()
{
    if (m_persistedRadius == m_radius) {
        return;
    }
    m_persistedRadius = m_radius;
    ConfigSettings::instance()->setValue("rounded", "radius", m_radius);
}

// 把防抖待写入的值立刻落盘（松手、输入完成、控件隐藏/销毁时调用）
void RoundedCornerWidget::flushPendingPersist()
{
    if (m_persistTimer && m_persistTimer->isActive()) {
        m_persistTimer->stop();
    }
    persistRadius();
}

void RoundedCornerWidget::hideEvent(QHideEvent *event)
{
    // 控件隐藏时（例如拖到一半按 Esc）复位拖动状态，避免残留到下一次显示
    m_dragging = false;
    m_groupFrame->setCursor(Qt::OpenHandCursor);
    if (m_groupCursorActive) {
        applyOverrideCursorShape(m_groupPreviousCursorShape);
        m_groupCursorActive = false;
    }
    if (m_panelCursorActive) {
        applyOverrideCursorShape(m_panelPreviousCursorShape);
        m_panelCursorActive = false;
    }
    flushPendingPersist();
    QWidget::hideEvent(event);
}

void RoundedCornerWidget::persistRatioType()
{
    ConfigSettings::instance()->setValue("rounded", "ratio_type", m_ratioLocked ? 1 : 0);
}

void RoundedCornerWidget::persistRememberSetting()
{
    ConfigSettings::instance()->setValue("rounded", "remember", m_rememberSetting);
}

void RoundedCornerWidget::persistFixedSize()
{
    m_rememberedIsRatio = m_ratioLocked;
    if (m_rememberedIsRatio) {
        if (m_ratioWidth <= 0 || m_ratioHeight <= 0) {
            syncRatioFromSelection();
        }
        if (m_ratioWidth <= 0 || m_ratioHeight <= 0) {
            return;
        }
        m_fixedWidth = m_ratioWidth;
        m_fixedHeight = m_ratioHeight;
    } else {
        if (m_selectionWidth <= 0 || m_selectionHeight <= 0) {
            return;
        }
        m_fixedWidth = qMax(1, m_selectionWidth);
        m_fixedHeight = qMax(1, m_selectionHeight);
    }

    ConfigSettings *config = ConfigSettings::instance();
    config->setValue("rounded", "remember_ratio", m_rememberedIsRatio);
    config->setValue("rounded", "fixed_width", m_fixedWidth);
    config->setValue("rounded", "fixed_height", m_fixedHeight);
}

void RoundedCornerWidget::captureCurrentSetting()
{
    persistFixedSize();
}

bool RoundedCornerWidget::hasPendingRememberedSetting() const
{
    return m_pendingRememberedSetting && m_rememberSetting
            && m_fixedWidth > 0 && m_fixedHeight > 0;
}

bool RoundedCornerWidget::isRememberedSettingRatio() const
{
    return m_rememberedIsRatio;
}

QSize RoundedCornerWidget::rememberedSetting() const
{
    return QSize(m_fixedWidth, m_fixedHeight);
}

void RoundedCornerWidget::consumeRememberedSetting()
{
    m_pendingRememberedSetting = false;
}

int RoundedCornerWidget::radius() const
{
    return m_radius;
}

bool RoundedCornerWidget::isCornerEnabled() const
{
    return m_radius > 0;
}

bool RoundedCornerWidget::isRatioLocked() const
{
    return m_ratioLocked;
}

qreal RoundedCornerWidget::lockedRatio() const
{
    if (!m_ratioLocked) {
        return 0.0;
    }
    if (m_ratioWidth > 0 && m_ratioHeight > 0) {
        return static_cast<qreal>(m_ratioWidth) / static_cast<qreal>(m_ratioHeight);
    }
    if (m_selectionWidth > 0 && m_selectionHeight > 0) {
        return static_cast<qreal>(m_selectionWidth) / static_cast<qreal>(m_selectionHeight);
    }
    return 0.0;
}

void RoundedCornerWidget::setRadius(int radius)
{
    m_radiusSlider->setValue(qBound(0, radius, RADIUS_MAX));
}

void RoundedCornerWidget::setPanel(PanelType panel)
{
    if (m_panel != panel) {
        m_panel = panel;
        updateLayout();
        reposition();
        emit panelExpandedChanged(m_panel != NoPanel);
    }
    // 无论面板是切换还是收起，都从面板状态统一回写按钮选中态，
    // 保证圆角和尺寸/比例两个按钮始终至多只有一个高亮。
    syncButtonStates();
}

void RoundedCornerWidget::collapsePanels()
{
    flushPendingPersist();
    setPanel(NoPanel);
}

Qt::CursorShape RoundedCornerWidget::cursorShapeAt(const QPoint &parentPos) const
{
    // 拖动期间始终保持闭合手型，即使在快速移动时鼠标暂时离开了底板。
    if (m_dragging) {
        return Qt::ClosedHandCursor;
    }
    if (currentGroupPlateRect().contains(parentPos)) {
        return Qt::OpenHandCursor;
    }

    const QPoint localPos = mapFromParent(parentPos);
    const auto contains = [this, &localPos](const QWidget *widget) {
        if (!widget || !widget->isVisible()) {
            return false;
        }
        const QRect rect(widget->mapTo(this, QPoint(0, 0)), widget->size());
        return rect.contains(localPos);
    };
    if (contains(m_widthEdit) || contains(m_heightEdit) || contains(m_radiusSpin)) {
        return Qt::IBeamCursor;
    }
    return Qt::ArrowCursor;
}

void RoundedCornerWidget::installPanelCursorFilters(QWidget *panel)
{
    if (!panel) {
        return;
    }
    panel->installEventFilter(this);
    const QList<QWidget *> children = panel->findChildren<QWidget *>();
    for (QWidget *child : children) {
        child->installEventFilter(this);
    }
}

bool RoundedCornerWidget::isTextInputWidget(const QObject *watched) const
{
    const auto *widget = qobject_cast<const QWidget *>(watched);
    if (!widget) {
        return false;
    }
    if (qobject_cast<const QLineEdit *>(widget)) {
        return true;
    }
    const auto isWithin = [widget](const QWidget *container) {
        for (const QWidget *parent = widget; parent; parent = parent->parentWidget()) {
            if (parent == container) {
                return true;
            }
        }
        return false;
    };
    return isWithin(m_widthEdit) || isWithin(m_heightEdit) || isWithin(m_radiusSpin);
}

void RoundedCornerWidget::setSelectionSize(int width, int height)
{
    m_selectionWidth = width;
    m_selectionHeight = height;
    updateDimensionFields();
}

void RoundedCornerWidget::syncRatioFromSelection()
{
    if (m_selectionWidth > 0 && m_selectionHeight > 0) {
        const QPair<int, int> ratio = simplifiedRatio(m_selectionWidth, m_selectionHeight);
        m_ratioWidth = ratio.first;
        m_ratioHeight = ratio.second;
    } else if (m_ratioWidth <= 0 || m_ratioHeight <= 0) {
        m_ratioWidth = 1;
        m_ratioHeight = 1;
    }
}

void RoundedCornerWidget::updateDimensionFields()
{
    if (!m_widthEdit || !m_heightEdit) {
        return;
    }

    const int maxValue = m_ratioLocked ? MAX_RATIO_VALUE : MAX_DIMENSION_VALUE;
    if (m_widthValidator) {
        m_widthValidator->setRange(1, maxValue);
    }
    if (m_heightValidator) {
        m_heightValidator->setRange(1, maxValue);
    }

    m_syncingSelectionSize = true;
    if (m_ratioLocked) {
        if (m_ratioWidth <= 0 || m_ratioHeight <= 0) {
            syncRatioFromSelection();
        }
        if (m_widthEdit->text().toInt() != m_ratioWidth) {
            m_widthEdit->setText(QString::number(m_ratioWidth));
        }
        if (m_heightEdit->text().toInt() != m_ratioHeight) {
            m_heightEdit->setText(QString::number(m_ratioHeight));
        }
    } else {
        if (m_widthEdit->text().toInt() != m_selectionWidth) {
            m_widthEdit->setText(QString::number(m_selectionWidth));
        }
        if (m_heightEdit->text().toInt() != m_selectionHeight) {
            m_heightEdit->setText(QString::number(m_selectionHeight));
        }
    }
    m_syncingSelectionSize = false;
}

void RoundedCornerWidget::updateLayout()
{
    const bool hasPanel = (m_panel != NoPanel);
    const int panelHeightOffset = (m_panel == SizePanel) ? SECOND_BUTTON_Y + BUTTON_SIZE / 2 : FIRST_BUTTON_Y + BUTTON_SIZE / 2;
    const int totalWidth = hasPanel ? (GROUP_WIDTH + PANEL_GAP + PANEL_WIDTH) : GROUP_WIDTH;
    const int margin = plateShadowMargin(this);

    // 控件尺寸 = 可见底板尺寸 + DFloatingWidget 的阴影留白
    setFixedSize(totalWidth + margin * 2, GROUP_HEIGHT + margin * 2);

    const int groupX = (hasPanel && m_panelOnLeft) ? (PANEL_WIDTH + PANEL_GAP) : 0;
    m_groupFrame->move(groupX, 0);

    const int panelX = m_panelOnLeft ? 0 : (GROUP_WIDTH + PANEL_GAP);
    const int panelY = panelHeightOffset - PANEL_HEIGHT / 2;

    m_cornerPanel->setVisible(m_panel == CornerPanel);
    m_sizePanel->setVisible(m_panel == SizePanel);
    m_cornerPanel->move(panelX, panelY);
    m_sizePanel->move(panelX, panelY);

    if (m_panel != NoPanel) {
        (m_panel == CornerPanel ? m_cornerPanel : m_sizePanel)->raise();
    }
}

void RoundedCornerWidget::followSelection(const QRect &selection, const QRect &screen)
{
    if (selection.isEmpty() || screen.isEmpty()) {
        return;
    }
    m_lastSelection = selection;
    m_lastScreen = screen;
    reposition();
}

void RoundedCornerWidget::reposition()
{
    const QRect &selection = m_lastSelection;
    const QRect &screen = m_lastScreen;
    if (selection.isEmpty() || screen.isEmpty()) {
        return;
    }

    const bool hasPanel = (m_panel != NoPanel);

    // 面板展开方向与位置都以「按钮组 + 面板」的完整宽度计算，
    // 保证面板展开前后按钮组位置不会跳动（设计稿 ①/②/③ 判定条件）。
    int groupLeft = 0;
    int groupTop = 0;
    bool panelOnLeft = false;
    if (m_userMoved) {
        // 用户手动拖过按钮组：不再按①②③贴边，保持用户放的位置；
        // 面板朝空间大的一侧展开，避免拖到边缘后面板出屏。
        groupLeft = m_groupAnchor.x();
        groupTop = m_groupAnchor.y();
        panelOnLeft = groupLeft + GROUP_WIDTH / 2 < screen.center().x();
    } else {
        const GroupPlacement placement = computeGroupPlacement(selection, screen);
        groupLeft = placement.groupLeft;
        groupTop = placement.groupTop;
        panelOnLeft = placement.panelOnLeft;
    }

    if (m_panelOnLeft != panelOnLeft) {
        m_panelOnLeft = panelOnLeft;
        updateLayout();
    }

    // 面板向左展开时按钮组在控件内部有固定偏移；面板收起时该偏移为 0。
    // 必须按当前展开状态换算控件原点，否则收起面板时按钮组会偏移一个面板宽度。
    const int groupOffset = (panelOnLeft && hasPanel) ? (PANEL_WIDTH + PANEL_GAP) : 0;
    // 控件左上角比可见底板多出一个阴影留白，定位时要去掉
    const int margin = plateShadowMargin(this);
    int x = groupLeft - groupOffset - margin;
    int y = groupTop - margin;

    x = qBound(screen.left(), x, qMax(screen.left(), screen.right() - width() + 1));
    y = qBound(screen.top(), y, qMax(screen.top(), screen.bottom() - height() + 1));
    move(x, y);
    m_groupAnchor = QPoint(x + groupOffset + margin, y + margin);
}

QRect RoundedCornerWidget::currentGroupPlateRect() const
{
    const int margin = plateShadowMargin(this);
    return QRect(pos() + m_groupFrame->pos() + QPoint(margin, margin),
                 QSize(GROUP_WIDTH, GROUP_HEIGHT));
}

bool RoundedCornerWidget::eventFilter(QObject *watched, QEvent *event)
{
    const auto *watchedWidget = qobject_cast<const QWidget *>(watched);
    const bool isGroupWidget = watched == m_groupFrame
            || (watchedWidget && m_groupFrame && m_groupFrame->isAncestorOf(watchedWidget));
    if (isGroupWidget) {
        if (event->type() == QEvent::Enter && !m_groupCursorActive) {
            const QCursor *current = QApplication::overrideCursor();
            m_groupPreviousCursorShape = current ? current->shape() : Qt::ArrowCursor;
            m_groupCursorActive = true;
        }
        if (event->type() == QEvent::Enter || event->type() == QEvent::MouseMove) {
            applyOverrideCursorShape(m_dragging ? Qt::ClosedHandCursor : Qt::OpenHandCursor);
        } else if (event->type() == QEvent::Leave && m_groupCursorActive
                   && m_groupFrame
                   && !m_groupFrame->rect().contains(m_groupFrame->mapFromGlobal(QCursor::pos()))) {
            applyOverrideCursorShape(m_groupPreviousCursorShape);
            m_groupCursorActive = false;
        }
    }

    const bool isPropertyPanel = (watched == m_cornerPanel || watched == m_sizePanel);
    if (isPropertyPanel) {
        if (event->type() == QEvent::Enter && !m_panelCursorActive) {
            const QCursor *current = QApplication::overrideCursor();
            m_panelPreviousCursorShape = current ? current->shape() : Qt::ArrowCursor;
            applyOverrideCursorShape(Qt::ArrowCursor);
            m_panelCursorActive = true;
        } else if (event->type() == QEvent::Leave && m_panelCursorActive) {
            applyOverrideCursorShape(m_panelPreviousCursorShape);
            m_panelCursorActive = false;
        }
    }

    const auto *panelChild = qobject_cast<const QWidget *>(watched);
    const bool isPanelChild = panelChild
            && ((m_cornerPanel && m_cornerPanel->isAncestorOf(panelChild))
                || (m_sizePanel && m_sizePanel->isAncestorOf(panelChild)));
    if (isPanelChild && (event->type() == QEvent::Enter || event->type() == QEvent::MouseMove)) {
        applyOverrideCursorShape(isTextInputWidget(watched) ? Qt::IBeamCursor : Qt::ArrowCursor);
    }
    return QWidget::eventFilter(watched, event);
}

void RoundedCornerWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    // 与主工具栏一致：按住按钮组底板即可整体拖动（按钮、滑块等子控件会先消费自己的点击）
    m_groupFrame->setCursor(Qt::ClosedHandCursor);
    applyOverrideCursorShape(Qt::ClosedHandCursor);
    m_dragging = true;
    m_dragMouseStart = eventGlobalPos(event);
    m_dragWidgetStart = pos();
    event->accept();
}

void RoundedCornerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    QPoint target = m_dragWidgetStart + (eventGlobalPos(event) - m_dragMouseStart);

    // 限制在截图背景范围内，避免拖出屏幕后找不回来
    const QRect &screen = m_lastScreen;
    if (!screen.isEmpty()) {
        target.setX(qBound(screen.left(), target.x(), qMax(screen.left(), screen.right() - width() + 1)));
        target.setY(qBound(screen.top(), target.y(), qMax(screen.top(), screen.bottom() - height() + 1)));
    }

    if (target != pos()) {
        m_userMoved = true;
        move(target);
        // 拖动过程中同步锚点，面板展开/收起时按钮组不会跳
        const int margin = plateShadowMargin(this);
        m_groupAnchor = pos() + m_groupFrame->pos() + QPoint(margin, margin);
    }
    event->accept();
}

void RoundedCornerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    const bool wasDragging = m_dragging;
    m_dragging = false;
    m_groupFrame->setCursor(Qt::OpenHandCursor);
    applyOverrideCursorShape(Qt::OpenHandCursor);
    event->accept();
    if (wasDragging && m_userMoved) {
        emit groupMoved();
    }
}

QRect RoundedCornerWidget::groupPlateRect(const QRect &selection, const QRect &screen) const
{
    if (selection.isEmpty() || screen.isEmpty()) {
        return {};
    }

    const GroupPlacement placement = computeGroupPlacement(selection, screen);
    const bool hasPanel = (m_panel != NoPanel);
    const int margin = plateShadowMargin(this);
    const int groupOffset = (placement.panelOnLeft && hasPanel) ? (PANEL_WIDTH + PANEL_GAP) : 0;
    const int widgetWidth = (hasPanel ? (GROUP_WIDTH + PANEL_GAP + PANEL_WIDTH) : GROUP_WIDTH) + margin * 2;
    const int widgetHeight = GROUP_HEIGHT + margin * 2;

    // 与 reposition() 使用完全相同的边界收敛，保证算出来的底板位置就是实际显示位置。
    int x = placement.groupLeft - groupOffset - margin;
    int y = placement.groupTop - margin;
    x = qBound(screen.left(), x, qMax(screen.left(), screen.right() - widgetWidth + 1));
    y = qBound(screen.top(), y, qMax(screen.top(), screen.bottom() - widgetHeight + 1));

    const int groupX = (hasPanel && placement.panelOnLeft) ? (PANEL_WIDTH + PANEL_GAP) : 0;
    return QRect(x + groupX + margin, y + margin, GROUP_WIDTH, GROUP_HEIGHT);
}

QPixmap RoundedCornerWidget::applyRoundedCorners(const QPixmap &source, int radius)
{
    if (source.isNull() || radius <= 0) {
        return source;
    }
    QImage image = source.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const qreal dpr = source.devicePixelRatio() > 0 ? source.devicePixelRatio() : 1.0;
    // 全程在「设备像素」坐标系下裁剪，最后再恢复 devicePixelRatio。
    // 若在绘制前给 out 设置 dpr，QPainter 会切换到逻辑坐标，而下面的
    // QPainterPath 仍以设备像素构造（宽高、radius*dpr），路径会被放大 dpr 倍，
    // 导致只有左上角落在画布内、其余三个圆角被裁到画布外（高 DPI 下只圆了左上角）。
    image.setDevicePixelRatio(1.0);
    const qreal scaledRadius = radius * dpr;
    QImage out(image.size(), QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::transparent);
    out.setDevicePixelRatio(1.0);

    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, image.width(), image.height()), scaledRadius, scaledRadius);
    painter.setClipPath(path);
    painter.drawImage(0, 0, image);
    painter.end();

    out.setDevicePixelRatio(dpr);
    QPixmap result = QPixmap::fromImage(out);
    result.setDevicePixelRatio(dpr);
    return result;
}

QPixmap RoundedCornerWidget::compositeOnWhite(const QPixmap &source)
{
    if (source.isNull() || !source.hasAlphaChannel()) {
        return source;
    }
    QImage image = source.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const qreal dpr = image.devicePixelRatio() > 0 ? image.devicePixelRatio() : 1.0;
    QImage out(image.size(), QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::white);
    out.setDevicePixelRatio(dpr);

    QPainter painter(&out);
    painter.drawImage(0, 0, image);
    painter.end();

    QPixmap result = QPixmap::fromImage(out);
    result.setDevicePixelRatio(dpr);
    return result;
}
