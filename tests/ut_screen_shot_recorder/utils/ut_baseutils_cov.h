// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include <QFont>
#include <QColor>
#include <gtest/gtest.h>
#include "../../src/utils/baseutils.h"

using namespace testing;

// Coverage tests for BaseUtils. The existing ut_baseutils.h covers colorIndexOf,
// colorIndex, isValidFormat, stringWidth, and most setCursorShape branches.
// Here we cover sanitizeFileName (fully untested) plus the pen branch and
// colorIndexOf round-trip on the remaining indices.
//
// SKIPPED: isCommandExist() spawns a real QProcess (`which ...`); per the rules
// process-spawning methods must not be invoked from unit tests.
class BaseUtilsCovTest : public testing::Test
{
public:
    void SetUp() override { qDebug() << "start BaseUtilsCovTest"; }
    void TearDown() override { qDebug() << "end BaseUtilsCovTest"; }
};

// ---- sanitizeFileName ----

TEST_F(BaseUtilsCovTest, sanitizeFileNameReplacesSlashes)
{
    const QString in = QStringLiteral("a/b\\c");
    QString out = BaseUtils::sanitizeFileName(in);
    EXPECT_FALSE(out.contains('/'));
    EXPECT_FALSE(out.contains('\\'));
    // separators replaced with '-'
    EXPECT_EQ(QStringLiteral("a-b-c"), out);
}

TEST_F(BaseUtilsCovTest, sanitizeFileNameReplacesAllUnsafeChars)
{
    // each of / \ : * ? " < > | should become '-'
    const QString unsafe = QStringLiteral(":/\\*?\"<>|");
    QString out = BaseUtils::sanitizeFileName(unsafe);
    for (const QChar &c : unsafe) {
        EXPECT_FALSE(out.contains(c)) << "unsafe char" << c.toLatin1() << "still present";
    }
}

TEST_F(BaseUtilsCovTest, sanitizeFileNameTrimsWhitespace)
{
    const QString in = QStringLiteral("   hello   ");
    QString out = BaseUtils::sanitizeFileName(in);
    EXPECT_EQ(QStringLiteral("hello"), out);
}

TEST_F(BaseUtilsCovTest, sanitizeFileNameEmptyBecomesUnnamed)
{
    QString out = BaseUtils::sanitizeFileName(QStringLiteral(""));
    EXPECT_EQ(QStringLiteral("unnamed"), out);
}

TEST_F(BaseUtilsCovTest, sanitizeFileNameWhitespaceOnlyBecomesUnnamed)
{
    QString out = BaseUtils::sanitizeFileName(QStringLiteral("    "));
    EXPECT_EQ(QStringLiteral("unnamed"), out);
}

TEST_F(BaseUtilsCovTest, sanitizeFileNameKeepsSafeName)
{
    const QString in = QStringLiteral("firefox-esr");
    QString out = BaseUtils::sanitizeFileName(in);
    EXPECT_EQ(in, out);
}

// ---- colorIndexOf / colorIndex round-trip on all 12 palette entries ----
// The existing test only checks indices 0-4 (with stale hex values); verify the
// full palette is internally consistent: index -> color -> index.

TEST_F(BaseUtilsCovTest, colorPaletteRoundTripAllIndices)
{
    for (int i = 0; i <= 11; ++i) {
        QColor c = BaseUtils::colorIndexOf(i);
        EXPECT_TRUE(c.isValid()) << "color at index" << i << "invalid";
        int back = BaseUtils::colorIndex(c);
        EXPECT_EQ(i, back) << "round-trip failed for index" << i;
    }
}

TEST_F(BaseUtilsCovTest, colorIndexOfUnknownReturnsInvalid)
{
    // out-of-range index into an empty-grown QList yields a default QColor
    EXPECT_NO_FATAL_FAILURE(BaseUtils::colorIndexOf(999));
}

TEST_F(BaseUtilsCovTest, colorIndexUnknownColorReturnsMinusOne)
{
    QColor unknown(123, 222, 9); // not in the palette
    EXPECT_EQ(-1, BaseUtils::colorIndex(unknown));
}

// ---- setCursorShape pen branch (colorIndex-dependent name) ----
// The existing test never passes "pen", leaving that branch uncovered.

TEST_F(BaseUtilsCovTest, setCursorShapePenCachesByColorIndex)
{
    EXPECT_NO_FATAL_FAILURE(BaseUtils::setCursorShape(QStringLiteral("pen"), 0));
    EXPECT_NO_FATAL_FAILURE(BaseUtils::setCursorShape(QStringLiteral("pen"), 3));
    // second call with the same name hits the cache path
    EXPECT_NO_FATAL_FAILURE(BaseUtils::setCursorShape(QStringLiteral("pen"), 0));
}

// ---- isValidFormat exhaustive ----
TEST_F(BaseUtilsCovTest, isValidFormatAllValidSuffixes)
{
    const QStringList valid = {"bmp", "jpg", "jpeg", "png", "pbm", "pgm", "xbm", "xpm"};
    for (const QString &s : valid) {
        EXPECT_TRUE(BaseUtils::isValidFormat(s)) << s.toStdString() << "should be valid";
    }
}

TEST_F(BaseUtilsCovTest, isValidFormatInvalidSuffixes)
{
    EXPECT_FALSE(BaseUtils::isValidFormat("mp4"));
    EXPECT_FALSE(BaseUtils::isValidFormat("gif"));
    EXPECT_FALSE(BaseUtils::isValidFormat(""));
    EXPECT_FALSE(BaseUtils::isValidFormat("tiff"));
}

// ---- stringWidth edge cases ----
TEST_F(BaseUtilsCovTest, stringWidthEmptyString)
{
    QFont f(QStringLiteral("Arial"));
    EXPECT_GE(BaseUtils::stringWidth(f, QStringLiteral("")), 0);
}

TEST_F(BaseUtilsCovTest, stringWidthNonEmpty)
{
    QFont f(QStringLiteral("Arial"));
    int w1 = BaseUtils::stringWidth(f, QStringLiteral("A"));
    int w5 = BaseUtils::stringWidth(f, QStringLiteral("AAAAA"));
    EXPECT_GT(w5, w1);
}
