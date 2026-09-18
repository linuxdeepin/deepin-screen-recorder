// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <gtest/gtest.h>
#include "../../src/utils/shortcut.h"

using namespace testing;

// Coverage tests for Shortcut. The existing ut_shortcut.h only checks toStr()
// is non-empty. Here we exercise getDefaultValue (all branches) and getSysShortcuts
// (which, on CI with no keybinding service, falls back to the default values).
class ShortcutCovTest : public testing::Test
{
public:
    Shortcut shortcut;
    void SetUp() override {}
    void TearDown() override {}
};

// getDefaultValue: cover every named branch plus the unknown-type else.
TEST_F(ShortcutCovTest, getDefaultValueScreenshot)
{
    EXPECT_EQ(QStringLiteral("Ctrl+Alt+A"), shortcut.getDefaultValue("screenshot"));
}

TEST_F(ShortcutCovTest, getDefaultValueRecorder)
{
    EXPECT_EQ(QStringLiteral("Ctrl+Alt+R"), shortcut.getDefaultValue("deepin-screen-recorder"));
}

TEST_F(ShortcutCovTest, getDefaultValueWindow)
{
    EXPECT_EQ(QStringLiteral("Alt+PrintScreen"), shortcut.getDefaultValue("screenshot-window"));
}

TEST_F(ShortcutCovTest, getDefaultValueDelayed)
{
    EXPECT_EQ(QStringLiteral("Ctrl+PrintScreen"), shortcut.getDefaultValue("screenshot-delayed"));
}

TEST_F(ShortcutCovTest, getDefaultValueFullscreen)
{
    EXPECT_EQ(QStringLiteral("PrintScreen"), shortcut.getDefaultValue("screenshot-fullscreen"));
}

TEST_F(ShortcutCovTest, getDefaultValueUnknownType)
{
    // scroll/ocr have no default branch -> returns empty string
    EXPECT_TRUE(shortcut.getDefaultValue("screenshot-scroll").isEmpty());
    EXPECT_TRUE(shortcut.getDefaultValue("screenshot-ocr").isEmpty());
    EXPECT_TRUE(shortcut.getDefaultValue("totally-unknown").isEmpty());
}

// getSysShortcuts: without the keybinding DBus service it falls back to
// getDefaultValue for the known types, and "" for unknown ones. Must not crash.
TEST_F(ShortcutCovTest, getSysShortcutsFallbackAllTypes)
{
    EXPECT_NO_FATAL_FAILURE(shortcut.getSysShortcuts("screenshot"));
    EXPECT_NO_FATAL_FAILURE(shortcut.getSysShortcuts("deepin-screen-recorder"));
    EXPECT_NO_FATAL_FAILURE(shortcut.getSysShortcuts("screenshot-window"));
    EXPECT_NO_FATAL_FAILURE(shortcut.getSysShortcuts("screenshot-delayed"));
    EXPECT_NO_FATAL_FAILURE(shortcut.getSysShortcuts("screenshot-fullscreen"));
    EXPECT_NO_FATAL_FAILURE(shortcut.getSysShortcuts("screenshot-scroll"));
    EXPECT_NO_FATAL_FAILURE(shortcut.getSysShortcuts("screenshot-ocr"));
}

// The constructed JSON has a "shortcut" array with 6 groups.
TEST_F(ShortcutCovTest, toStrProducesValidShortcutJson)
{
    QString s = shortcut.toStr();
    ASSERT_FALSE(s.isEmpty());
    QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
    ASSERT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();
    ASSERT_TRUE(obj.contains("shortcut"));
    QJsonArray groups = obj.value("shortcut").toArray();
    EXPECT_EQ(6, groups.size());
    // each group has groupName + groupItems
    for (const QJsonValue &gv : groups) {
        QJsonObject g = gv.toObject();
        EXPECT_TRUE(g.contains("groupName"));
        EXPECT_TRUE(g.contains("groupItems"));
    }
}
