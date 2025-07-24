// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAPETOOLWIDGET_H
#define SHAPETOOLWIDGET_H

#include <DLabel>
#include <DWidget>
#include <QButtonGroup>

#include "toolbutton.h"

DWIDGET_USE_NAMESPACE

class ShapeToolWidget : public DLabel
{
    Q_OBJECT
public:
    explicit ShapeToolWidget(DWidget *parent = nullptr);
    ~ShapeToolWidget();

    void initWidget();
    void selectShape(const QString &shape);

signals:
    void shapeSelected(const QString &shape);

private:
    void initShapeButtons();

private:
    QButtonGroup *m_shapeBtnGroup;
    ToolButton *m_rectButton;
    ToolButton *m_ovalButton;
};

#endif // SHAPETOOLWIDGET_H
