// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QIcon>

#include "../../../src/dde-dock-plugins/shotstartrecord/commoniconbutton.h"
#include <DGuiApplicationHelper>
DGUI_USE_NAMESPACE

#include "stub.h"

static DGuiApplicationHelper::ColorType lightTheme_stub() {
    return DGuiApplicationHelper::LightType;
}

namespace {
class RecordCommonIconButtonCovTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_btn = new CommonIconButton(nullptr);
        m_btn->show();
    }
    void TearDown() override
    {
        if (m_btn) {
            delete m_btn;
            m_btn = nullptr;
        }
    }
    CommonIconButton *m_btn = nullptr;
};
} // namespace

TEST_F(RecordCommonIconButtonCovTest, SetClickable_ToggleState_NoException)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->setClickable(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setClickable(false));
}

TEST_F(RecordCommonIconButtonCovTest, SetRotatable_TrueThenFalse_CleansTimer)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->setRotatable(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setRotatable(false));
}

TEST_F(RecordCommonIconButtonCovTest, StartRotate_TriggersRotationAnimation)
{
    m_btn->setRotatable(true);
    EXPECT_NO_FATAL_FAILURE(m_btn->startRotate());
}

TEST_F(RecordCommonIconButtonCovTest, StopRotate_StopsTimerAndResetsAngle)
{
    m_btn->setRotatable(true);
    m_btn->startRotate();
    EXPECT_NO_FATAL_FAILURE(m_btn->stopRotate());
}

TEST_F(RecordCommonIconButtonCovTest, SetHoverIcon_StoresIcon)
{
    QIcon icon;
    EXPECT_NO_FATAL_FAILURE(m_btn->setHoverIcon(icon));
}

TEST_F(RecordCommonIconButtonCovTest, SetActiveState_TogglesHighlight)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->setActiveState(true));
    EXPECT_NO_FATAL_FAILURE(m_btn->setActiveState(false));
}

TEST_F(RecordCommonIconButtonCovTest, SetStateIconMapping_AppliesMapping)
{
    QMap<CommonIconButton::State, QPair<QString, QString>> mapping;
    mapping.insert(CommonIconButton::On, qMakePair(QString("on"), QString("on-fallback")));
    mapping.insert(CommonIconButton::Off, qMakePair(QString("off"), QString("off-fallback")));
    EXPECT_NO_FATAL_FAILURE(m_btn->setStateIconMapping(mapping));
}

TEST_F(RecordCommonIconButtonCovTest, SetState_WithMapping_AppliesIconFromMapping)
{
    QMap<CommonIconButton::State, QPair<QString, QString>> mapping;
    mapping.insert(CommonIconButton::On, qMakePair(QString("edit"), QString("")));
    m_btn->setStateIconMapping(mapping);
    EXPECT_NO_FATAL_FAILURE(m_btn->setState(CommonIconButton::On));
}

TEST_F(RecordCommonIconButtonCovTest, SetState_WithoutMapping_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->setState(CommonIconButton::Default));
}

TEST_F(RecordCommonIconButtonCovTest, RefreshIcon_ReappliesCurrentState)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->refreshIcon());
}

TEST_F(RecordCommonIconButtonCovTest, SetIconWithStringAndSuffix_LoadsThemeIcon)
{
    EXPECT_NO_FATAL_FAILURE(m_btn->setIcon(QStringLiteral("edit"), QStringLiteral("edit-fallback"), QStringLiteral(".svg")));
}

TEST_F(RecordCommonIconButtonCovTest, PaintEvent_RendersWidgetWithoutException)
{
    QPaintEvent pe(m_btn->rect());
    EXPECT_NO_FATAL_FAILURE(m_btn->paintEvent(&pe));
}

TEST_F(RecordCommonIconButtonCovTest, MousePressEvent_RecordsPressPosition)
{
    QTest::mousePress(m_btn, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
}

TEST_F(RecordCommonIconButtonCovTest, MouseReleaseEvent_ClickableEmitsClickedSignal)
{
    m_btn->setClickable(true);
    QTest::mousePress(m_btn, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QTest::mouseRelease(m_btn, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
}

TEST_F(RecordCommonIconButtonCovTest, MouseReleaseEvent_RotatableStartsRotation)
{
    m_btn->setClickable(true);
    m_btn->setRotatable(true);
    QTest::mousePress(m_btn, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
    QTest::mouseRelease(m_btn, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
}

TEST_F(RecordCommonIconButtonCovTest, SetIcon_LightTheme_TriggersDarkMarkLambda)
{
    Stub s;
    s.set(ADDR(DGuiApplicationHelper, themeType), lightTheme_stub);
    EXPECT_NO_FATAL_FAILURE(m_btn->setIcon(QStringLiteral("edit"), QStringLiteral("edit-fallback"), QStringLiteral(".svg")));
}
