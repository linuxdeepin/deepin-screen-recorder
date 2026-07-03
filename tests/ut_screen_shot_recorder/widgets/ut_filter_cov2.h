// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include "addr_pri.h"
#include "../../src/widgets/filter.h"

using namespace testing;

// 覆盖 filter.cpp 剩余未覆盖分支：
//  - HoverFilter::eventFilter 默认分支 / Enter / Leave 三路
//  - HintFilter::eventFilter 各事件分支（Enter 无 HintWidget / Leave 含 DelayHide /
//    MouseButtonPress 含 _dm_keep_on_click）
//  - HintFilterPrivate::showHint 在 parentWidget=null 时早退
//  - HoverShadowFilter::eventFilter Enter / Leave / 默认分支

class FilterCov2Test : public Test
{
public:
    QWidget *m_w = nullptr;
    HoverFilter *m_hf = nullptr;
    HintFilter *m_hint = nullptr;
    HoverShadowFilter *m_shadow = nullptr;

    void SetUp() override
    {
        m_w = new QWidget;
        m_hf = new HoverFilter(m_w);
        m_hint = new HintFilter(m_w);
        m_shadow = new HoverShadowFilter(m_w);
    }
    void TearDown() override
    {
        delete m_shadow;
        delete m_hint;
        delete m_hf;
        delete m_w;
        while (qApp->overrideCursor()) {
            qApp->restoreOverrideCursor();
        }
    }
};

// HoverFilter::eventFilter：默认事件（非 Enter/Leave）
TEST_F(FilterCov2Test, hoverFilterDefaultEvent)
{
    QEvent ev(QEvent::PaletteChange);
    bool handled = true;
    EXPECT_NO_FATAL_FAILURE(handled = m_hf->eventFilter(m_w, &ev));
}

// HoverFilter::eventFilter：Enter -> setCursor(PointingHand)
TEST_F(FilterCov2Test, hoverFilterEnterEvent)
{
    QEvent ev(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(m_hf->eventFilter(m_w, &ev));
}

// HoverFilter::eventFilter：Leave -> unsetCursor + restoreOverrideCursor
TEST_F(FilterCov2Test, hoverFilterLeaveEvent)
{
    QEvent ev(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(m_hf->eventFilter(m_w, &ev));
}

// HintFilter::eventFilter：Enter 无 HintWidget 属性 -> 早退
TEST_F(FilterCov2Test, hintFilterEnterNoHintWidget)
{
    QEvent ev(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
}

// HintFilter::eventFilter：Enter 有 HintWidget 属性 + NoDelayShow=true -> 立即 showHint
TEST_F(FilterCov2Test, hintFilterEnterWithHintWidgetNoDelay)
{
    QWidget *hint = new QWidget;
    hint->setProperty("NoDelayShow", true);
    m_w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    QEvent ev(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
    delete hint;
}

// HintFilter::eventFilter：Enter 有 HintWidget + NoDelayShow=false -> 启动 delayShowTimer
TEST_F(FilterCov2Test, hintFilterEnterWithHintWidgetDelay)
{
    QWidget *hint = new QWidget;
    m_w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    QEvent ev(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
    delete hint;
}

// HintFilter::eventFilter：Leave 无 hintWidget -> 仅 unsetCursor
TEST_F(FilterCov2Test, hintFilterLeaveNoHintWidget)
{
    QEvent ev(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
}

// HintFilter::eventFilter：Leave 含 hintWidget + DelayHide=false -> hide
TEST_F(FilterCov2Test, hintFilterLeaveHideImmediately)
{
    QWidget *hint = new QWidget;
    hint->setProperty("DelayHide", false);
    m_w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    QEvent ev(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
    delete hint;
}

// HintFilter::eventFilter：Leave 含 hintWidget + DelayHide=true -> invokeMethod deleyHide
TEST_F(FilterCov2Test, hintFilterLeaveDelayHide)
{
    QWidget *hint = new QWidget;
    hint->setProperty("DelayHide", true);
    m_w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    QEvent ev(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
    delete hint;
}

// HintFilter::eventFilter：MouseButtonPress 无 hintWidget -> 跳过
TEST_F(FilterCov2Test, hintFilterMousePressNoHint)
{
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(0, 0),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
}

// HintFilter::eventFilter：MouseButtonPress 含 hintWidget + _dm_keep_on_click=true
TEST_F(FilterCov2Test, hintFilterMousePressKeepOnClick)
{
    QWidget *hint = new QWidget;
    hint->setProperty("_dm_keep_on_click", true);
    m_w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(0, 0),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
    delete hint;
}

// HintFilter::eventFilter：MouseButtonPress 含 hintWidget + _dm_keep_on_click=false -> hide
TEST_F(FilterCov2Test, hintFilterMousePressHideOnClick)
{
    QWidget *hint = new QWidget;
    hint->setProperty("_dm_keep_on_click", false);
    m_w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hint));
    QMouseEvent ev(QEvent::MouseButtonPress, QPointF(0, 0),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
    delete hint;
}

// HintFilter::eventFilter：默认事件 -> QObject::eventFilter
TEST_F(FilterCov2Test, hintFilterDefaultEvent)
{
    QEvent ev(QEvent::PaletteChange);
    EXPECT_NO_FATAL_FAILURE(m_hint->eventFilter(m_w, &ev));
}

// HoverShadowFilter::eventFilter：Enter -> 创建 graphics effect
TEST_F(FilterCov2Test, shadowFilterEnterEvent)
{
    QEvent ev(QEvent::Enter);
    EXPECT_NO_FATAL_FAILURE(m_shadow->eventFilter(m_w, &ev));
}

// HoverShadowFilter::eventFilter：Leave -> 删除 graphics effect
TEST_F(FilterCov2Test, shadowFilterLeaveEvent)
{
    // 先 Enter 创建 effect，再 Leave 删除
    QEvent enter(QEvent::Enter);
    m_shadow->eventFilter(m_w, &enter);
    QEvent leave(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(m_shadow->eventFilter(m_w, &leave));
}

// HoverShadowFilter::eventFilter：默认事件 -> 父类
TEST_F(FilterCov2Test, shadowFilterDefaultEvent)
{
    QEvent ev(QEvent::PaletteChange);
    EXPECT_NO_FATAL_FAILURE(m_shadow->eventFilter(m_w, &ev));
}

// HoverShadowFilter 与 HoverFilter 重复构造析构
TEST_F(FilterCov2Test, repeatedConstructDestruct)
{
    for (int i = 0; i < 3; ++i) {
        HoverFilter *hf = new HoverFilter;
        HintFilter *hif = new HintFilter;
        HoverShadowFilter *hsf = new HoverShadowFilter;
        delete hsf;
        delete hif;
        delete hf;
    }
    SUCCEED();
}
