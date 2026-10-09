// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PUTILS_H
#define PUTILS_H

#include <QColor>
#include <QObject>

class PUtils : public QObject
{
public:
    static bool isWaylandMode;
    /** TreeLand 合成器（Wayland 会话下 DDE_CURRENT_COMPOSITOR=TreeLand） */
    static bool isTreelandMode;

    /**
     * @brief 当前是否为深色主题
     *
     * 与 DTK 控件一致，统一走 DGuiApplicationHelper::themeType() 判断；
     * 不直接读 palette，避免控件被局部调色后取到错误结论。
     */
    static bool isDarkTheme();

    /**
     * @brief 按主题在两套设计稿取值之间二选一
     *
     * 表格识别相关卡片/文字的颜色都来自设计稿的浅色与深色两版，
     * 用同一套判断切换，避免各文件各写一份。
     */
    static QColor themeColor(const QColor &light, const QColor &dark);
};

#endif  // PUTILS_H
