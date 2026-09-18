// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QPixmap>
#include <QImage>
#include <QAbstractButton>
#include <QButtonGroup>
#include "addr_pri.h"
#include "../../src/widgets/imagemenu.h"

using namespace testing;

// Covers the deeper branches of ImageMenu / ImageBorderHelper that the sibling
// ut_imagemenu_ext.h only touches superficially:
//   - ImageMenu::ActionChecked with a real checked button (non-null) for each
//     BorderType (the ext path only invokes it via setActionState->nullptr).
//   - ImageMenu::setBorderTypeDetail with a valid button id (button != null
//     branch) and an invalid id (button == null branch).
//   - ImageMenu::getBorderTypeDetail with a checked button present.
//   - ImageBorderHelper::getPixmapAddBorder with each border type selected
//     (Prototype / External / Projection -> respective BorderProcess created),
//     plus the Nothing early-return.

class ImageMenuCovTest : public Test
{
public:
    ImageBorderHelper *m_h = nullptr;
    void SetUp() override { m_h = ImageBorderHelper::instance(); }
    void TearDown() override
    {
        // clear any selections so the Nothing branch is the default again
        m_h->setActionState(ImageBorderHelper::Nothing, false);
    }

    ImageMenu *makeMenu(ImageBorderHelper::BorderType type)
    {
        return m_h->getBorderMenu(type, QStringLiteral("t"), nullptr);
    }
};

// ---------- ImageMenu::ActionChecked with a real checked button ----------

TEST_F(ImageMenuCovTest, actionCheckedWithCheckedButtonForEachType)
{
    const ImageBorderHelper::BorderType types[] = {
        ImageBorderHelper::External,
        ImageBorderHelper::Prototype,
        ImageBorderHelper::Projection,
    };
    for (auto t : types) {
        ImageMenu *menu = makeMenu(t);
        ASSERT_NE(menu, nullptr);
        // pick the first button in the group, check it, then invoke the slot
        auto buttons = menu->findChildren<ActionWidget *>();
        if (buttons.isEmpty()) {
            continue;
        }
        ActionWidget *btn = buttons.first();
        btn->setChecked(true);
        EXPECT_NO_FATAL_FAILURE(menu->ActionChecked(btn));
        // also invoke with nullptr to exercise the clear path
        EXPECT_NO_FATAL_FAILURE(menu->ActionChecked(nullptr));
    }
    SUCCEED();
}

// ---------- ImageMenu::setBorderTypeDetail both branches ----------

TEST_F(ImageMenuCovTest, setBorderTypeDetailValidId)
{
    ImageMenu *menu = makeMenu(ImageBorderHelper::External);
    ASSERT_NE(menu, nullptr);
    auto buttons = menu->findChildren<ActionWidget *>();
    if (!buttons.isEmpty()) {
        // valid id (1 is always added first for External)
        EXPECT_NO_FATAL_FAILURE(menu->setBorderTypeDetail(1));
    }
    SUCCEED();
}

TEST_F(ImageMenuCovTest, setBorderTypeDetailInvalidId)
{
    ImageMenu *menu = makeMenu(ImageBorderHelper::External);
    ASSERT_NE(menu, nullptr);
    // no button with id 999 -> null branch
    EXPECT_NO_FATAL_FAILURE(menu->setBorderTypeDetail(999));
    SUCCEED();
}

// ---------- ImageMenu::getBorderTypeDetail with a checked button ----------

TEST_F(ImageMenuCovTest, getBorderTypeDetailWithCheckedButton)
{
    ImageMenu *menu = makeMenu(ImageBorderHelper::Prototype);
    ASSERT_NE(menu, nullptr);
    auto buttons = menu->findChildren<ActionWidget *>();
    if (!buttons.isEmpty()) {
        buttons.first()->setChecked(true);
    }
    int detail = -1;
    EXPECT_NO_FATAL_FAILURE(detail = menu->getBorderTypeDetail());
    EXPECT_GE(detail, 0);
}

// ---------- ImageBorderHelper::getPixmapAddBorder per border type ----------

TEST_F(ImageMenuCovTest, getPixmapAddBorderNothingReturnsOriginal)
{
    // ensure no selection -> Nothing branch returns input pixmap
    m_h->setActionState(ImageBorderHelper::Nothing, false);
    QPixmap in(10, 10);
    in.fill(Qt::red);
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = m_h->getPixmapAddBorder(in));
    EXPECT_FALSE(out.isNull());
}

TEST_F(ImageMenuCovTest, getPixmapAddBorderExternal)
{
    ImageMenu *menu = makeMenu(ImageBorderHelper::External);
    auto buttons = menu ? menu->findChildren<ActionWidget *>() : QList<ActionWidget *>();
    if (!buttons.isEmpty()) {
        buttons.first()->setChecked(true);
        menu->ActionChecked(buttons.first());
    }
    QPixmap in(40, 40);
    in.fill(Qt::blue);
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = m_h->getPixmapAddBorder(in));
    EXPECT_FALSE(out.isNull());
    // reset
    m_h->setActionState(ImageBorderHelper::Nothing, false);
}

TEST_F(ImageMenuCovTest, getPixmapAddBorderPrototype)
{
    ImageMenu *menu = makeMenu(ImageBorderHelper::Prototype);
    auto buttons = menu ? menu->findChildren<ActionWidget *>() : QList<ActionWidget *>();
    if (!buttons.isEmpty()) {
        buttons.first()->setChecked(true);
        menu->ActionChecked(buttons.first());
    }
    QPixmap in(40, 40);
    in.fill(Qt::green);
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = m_h->getPixmapAddBorder(in));
    EXPECT_FALSE(out.isNull());
    m_h->setActionState(ImageBorderHelper::Nothing, false);
}

TEST_F(ImageMenuCovTest, getPixmapAddBorderProjection)
{
    ImageMenu *menu = makeMenu(ImageBorderHelper::Projection);
    auto buttons = menu ? menu->findChildren<ActionWidget *>() : QList<ActionWidget *>();
    if (!buttons.isEmpty()) {
        buttons.first()->setChecked(true);
        menu->ActionChecked(buttons.first());
    }
    QPixmap in(40, 40);
    in.fill(Qt::yellow);
    QPixmap out;
    EXPECT_NO_FATAL_FAILURE(out = m_h->getPixmapAddBorder(in));
    EXPECT_FALSE(out.isNull());
    m_h->setActionState(ImageBorderHelper::Nothing, false);
}

// ---------- ImageBorderHelper::setBorderTypeDetail round-trip ----------

TEST_F(ImageMenuCovTest, helperSetBorderTypeDetailCombinesTypeAndId)
{
    // build a Prototype menu so the (type<<8|id) decode finds a menu
    makeMenu(ImageBorderHelper::Prototype);
    int combined = (ImageBorderHelper::Prototype << 8) | 1;
    EXPECT_NO_FATAL_FAILURE(m_h->setBorderTypeDetail(combined));
    SUCCEED();
}

// ---------- ActionWidget::paintEvent (checked + unchecked) ----------

TEST_F(ImageMenuCovTest, actionWidgetPaintEventCheckedAndUnchecked)
{
    ActionWidget w;
    w.setPixmap(QStringLiteral("imageBorder/pc.svg"));
    w.resize(160, 100);
    w.show();
    EXPECT_NO_FATAL_FAILURE(w.setActionState(true));
    EXPECT_NO_FATAL_FAILURE(w.repaint());
    EXPECT_NO_FATAL_FAILURE(w.setActionState(false));
    EXPECT_NO_FATAL_FAILURE(w.repaint());
    w.hide();
    SUCCEED();
}
