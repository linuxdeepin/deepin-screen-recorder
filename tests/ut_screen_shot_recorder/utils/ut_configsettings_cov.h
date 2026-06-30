// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include "../../src/utils/configsettings.h"

using namespace testing;

// Coverage tests for ConfigSettings. The existing ut_configsettings.h covers
// getValue/setValue on the "common" group and the constructor via instance().
// Here we exercise additional groups, the default-value path (unset key), the
// setValue-no-such-group early return, and the shapeConfigChanged signal.
class ConfigSettingsCovTest : public testing::Test
{
public:
    ConfigSettings *cs = nullptr;
    void SetUp() override { cs = ConfigSettings::instance(); }
    void TearDown() override {}
};

// instance() is idempotent (singleton).
TEST_F(ConfigSettingsCovTest, instanceReturnsSamePointer)
{
    EXPECT_EQ(cs, ConfigSettings::instance());
}

// getValue on every default-config group returns the documented default for an
// unset key (exercises getDefaultValue across all groups).
TEST_F(ConfigSettingsCovTest, getValueDefaultsForAllGroups)
{
    // Reset to default by removing a known key then reading it.
    EXPECT_NO_FATAL_FAILURE(cs->getValue("rectangle", "color_index"));
    EXPECT_NO_FATAL_FAILURE(cs->getValue("oval", "color_index"));
    EXPECT_NO_FATAL_FAILURE(cs->getValue("line", "line_width"));
    EXPECT_NO_FATAL_FAILURE(cs->getValue("effect", "isBlur"));
    EXPECT_NO_FATAL_FAILURE(cs->getValue("text", "fontsize"));
    EXPECT_NO_FATAL_FAILURE(cs->getValue("shape", "current"));
    EXPECT_NO_FATAL_FAILURE(cs->getValue("shot", "format"));
    EXPECT_NO_FATAL_FAILURE(cs->getValue("recorder", "frame_rate"));
}

// getValue on a group/key that is not in the default config returns an invalid
// QVariant (the getDefaultValue else-branch).
TEST_F(ConfigSettingsCovTest, getValueUnknownGroupReturnsInvalid)
{
    QVariant v = cs->getValue("no_such_group", "no_such_key");
    EXPECT_FALSE(v.isValid());
}

TEST_F(ConfigSettingsCovTest, getValueUnknownKeyInKnownGroupReturnsInvalid)
{
    QVariant v = cs->getValue("rectangle", "no_such_key");
    EXPECT_FALSE(v.isValid());
}

// setValue on a non-default group/key must be a no-op (early return).
TEST_F(ConfigSettingsCovTest, setValueUnknownGroupIsNoop)
{
    EXPECT_NO_FATAL_FAILURE(cs->setValue("no_such_group", "x", QVariant(5)));
}

// setValue with an Int emits shapeConfigChanged twice (before & after persist).
TEST_F(ConfigSettingsCovTest, setValueIntEmitsShapeConfigChanged)
{
    QSignalSpy spy(cs, &ConfigSettings::shapeConfigChanged);
    cs->setValue("rectangle", "color_index", QVariant(7));
    EXPECT_GE(spy.count(), 2);
    // restore a sane value
    cs->setValue("rectangle", "color_index", QVariant(3));
}

// setValue with a non-Int (bool) value must NOT emit shapeConfigChanged.
TEST_F(ConfigSettingsCovTest, setValueBoolDoesNotEmitSignal)
{
    QSignalSpy spy(cs, &ConfigSettings::shapeConfigChanged);
    cs->setValue("effect", "isBlur", QVariant(false));
    EXPECT_EQ(0, spy.count());
}

// round-trip an int value through setValue/getValue.
TEST_F(ConfigSettingsCovTest, setValueGetValueRoundTripInt)
{
    const QVariant newVal(2);
    cs->setValue("rectangle", "line_width", newVal);
    QVariant got = cs->getValue("rectangle", "line_width");
    EXPECT_EQ(newVal.toInt(), got.toInt());
    // restore default-ish
    cs->setValue("rectangle", "line_width", QVariant(0));
}

// round-trip a string value.
TEST_F(ConfigSettingsCovTest, setValueGetValueRoundTripString)
{
    const QVariant newVal(QStringLiteral("oval"));
    cs->setValue("shape", "current", newVal);
    QVariant got = cs->getValue("shape", "current");
    EXPECT_EQ(newVal.toString(), got.toString());
    cs->setValue("shape", "current", QVariant(QStringLiteral("rectangle")));
}
