// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QPixmap>
#include "../../src/widgets/imagemenu.h"

using namespace testing;

TEST(ImageBorderHelperTest, singletonAndState)
{
    auto *h = ImageBorderHelper::instance();
    ASSERT_NE(h, nullptr);
    // 各 BorderType 的 setActionState 分支
    EXPECT_NO_FATAL_FAILURE(h->setActionState(ImageBorderHelper::External, true));
    EXPECT_NO_FATAL_FAILURE(h->setActionState(ImageBorderHelper::Prototype, false));
    EXPECT_NO_FATAL_FAILURE(h->setActionState(ImageBorderHelper::Projection, true));
    EXPECT_NO_FATAL_FAILURE(h->setActionState(ImageBorderHelper::Nothing, false));
    EXPECT_NO_FATAL_FAILURE(h->setBorderTypeDetail(3));
    EXPECT_NO_FATAL_FAILURE(h->getBorderTypeDetail());
    EXPECT_NO_FATAL_FAILURE(h->pruneBorderMenus());
}

TEST(ImageBorderHelperTest, getBorderMenuAndPixmap)
{
    auto *h = ImageBorderHelper::instance();
    QPixmap pix(20, 20);
    pix.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE(h->getBorderMenu(ImageBorderHelper::External, QStringLiteral("ext"), nullptr));
    EXPECT_NO_FATAL_FAILURE(h->getBorderMenu(ImageBorderHelper::Prototype, QStringLiteral("proto"), nullptr));
    EXPECT_NO_FATAL_FAILURE(h->getPixmapAddBorder(pix));
}

TEST(ImageMenuTest, constructAndAccessors)
{
    for (int t = ImageBorderHelper::External; t <= ImageBorderHelper::Projection; ++t) {
        auto type = static_cast<ImageBorderHelper::BorderType>(t);
        EXPECT_NO_FATAL_FAILURE({
            ImageMenu m(type, QStringLiteral("title"), nullptr);
            m.AddAction();
            EXPECT_EQ(m.borderType(), type);
            m.setBorderTypeDetail(2);
            m.getBorderTypeDetail();
        });
    }
}

TEST(ActionWidgetTest, setPixmapAndState)
{
    ActionWidget w;
    EXPECT_NO_FATAL_FAILURE(w.setPixmap(QStringLiteral(":/res/1070/light.svg")));
    EXPECT_NO_FATAL_FAILURE(w.setActionState(true));
    EXPECT_NO_FATAL_FAILURE(w.setActionState(false));
}
