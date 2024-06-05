// Copyright (C) 2020 ~ 2024 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "basewindow.h"

#include <QQmlEngine>
#include <QApplication>
#include <QWidget>

BaseWindow::BaseWindow(QQmlEngine *engine, QWindow *parent)
    : QQuickView(engine, parent)
    , m_context(new QQmlContext(engine->rootContext(), this))
{
    connect(engine, &QQmlEngine::quit, QCoreApplication::instance(), &QCoreApplication::quit, Qt::QueuedConnection);
    connect(this, &QQuickView::statusChanged, this, [](QQuickView::Status status){
        if (status == QQuickView::Error) {
            QCoreApplication::quit();
        }
    });

    setTextRenderType(QQuickWindow::NativeTextRendering);

    // set up QML
    setResizeMode(QQuickView::SizeRootObjectToView);
    m_context->setContextProperty("contextWindow", this);
}

BaseWindow::~BaseWindow()
{

}

void BaseWindow::mousePressEvent(QMouseEvent *event)
{
    // QMenus need to be closed by hand when used from QML, see plasma-workspace/shellcorona.cpp
    if (auto popup = QApplication::activePopupWidget()) {
        popup->close();
        event->accept();
    } else {
        QQuickView::mousePressEvent(event);
    }
}

void BaseWindow::keyPressEvent(QKeyEvent *event)
{
    // Events need to be processed normally first for events to reach items
    QQuickView::keyPressEvent(event);
    if (event->isAccepted()) {
        return;
    }
    m_pressedKeys = event->key() | event->modifiers();
}

void BaseWindow::keyReleaseEvent(QKeyEvent *event)
{
    // Events need to be processed normally first for events to reach items
    QQuickView::keyReleaseEvent(event);
    if (event->isAccepted()) {
        return;
    }
    if ((event->matches(QKeySequence::Quit)
        || event->matches(QKeySequence::Close)
        || event->matches(QKeySequence::Cancel))
        && m_pressedKeys == (event->key() | event->modifiers())
    ) {
        event->accept();
        _Exit(0);
    }
    m_pressedKeys = {};
}
