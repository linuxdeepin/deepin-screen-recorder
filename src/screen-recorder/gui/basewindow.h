// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QQuickView>
#include <QQmlContext>

class BaseWindow : public QQuickView
{
    Q_OBJECT

protected:
    explicit BaseWindow(QQmlEngine *engine, QWindow *parent = nullptr);
    ~BaseWindow();

    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    const std::unique_ptr<QQmlContext> m_context;
    QKeySequence m_pressedKeys;
};
