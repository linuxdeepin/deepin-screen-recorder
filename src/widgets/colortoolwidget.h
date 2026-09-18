// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORTOOLWIDGET_H
#define COLORTOOLWIDGET_H

#include "toolbutton.h"
#include "utils/baseutils.h"

#include <DStackedWidget>
#include <DLabel>
#include <QMap>
#include <QGridLayout>
#include <QMetaEnum>

DWIDGET_USE_NAMESPACE

class ColorToolWidget : public DLabel
{
    Q_OBJECT
public:
    explicit ColorToolWidget(DWidget *parent = nullptr);
    ~ColorToolWidget();

    void initWidget();
    //颜色按钮初始化
    void initColorLabel();

signals:
    void colorChecked(QString colorType);

public slots:
    //切换当前选择的形状时，会触发当前函数
    void setFunction(const QString &func);

private:
    QGridLayout *m_baseLayout;
    bool m_isChecked;
    QString m_function;
    // 注意：原 m_redBtn/m_yellowBtn/m_blueBtn/m_greenBtn 成员自首次提交以来从未在
    // colortoolwidget.cpp 中赋值或使用，属于死代码。未初始化的裸指针会被单元测试
    // 通过 ACCESS_PRIVATE_FIELD 读取到垃圾值并解引用导致段错误。这些字段已在
    // initColorLabel() 中由 m_colorButtonGroup 统一管理，此处删除以免误用。
    /**
     * @brief 颜色按钮：key:颜色名称 value:按钮对象
     */
    QMap<QString, ToolButton> m_colorButtons;
    /**
     * @brief 颜色
     */
    QStringList m_colors;

    QButtonGroup *m_colorButtonGroup;

    /**
     * @brief 颜色按钮的颜色枚举对象
     */
    QMetaEnum m_buttonColors;
};

#endif // COLORTOOLWIDGET_H
