// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QStandardPaths>
#include <QTemporaryDir>
#include "../../../src/screenshot/core/screenshottypes.h"
#include "../../../src/screenshot/export/localscreenshotresultsink.h"

using namespace Dsr::Screenshot;
using namespace testing;

class LocalScreenshotResultSinkTest : public Test
{
protected:
    LocalScreenshotResultSink sink;
    QImage makeValidImage()
    {
        QImage img(10, 10, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::red);
        return img;
    }
};

TEST_F(LocalScreenshotResultSinkTest, SaveInvalidResultReturnsFalse)
{
    Result result;
    QString savedPath;
    QString error;
    EXPECT_FALSE(sink.save(result, QString(), &savedPath, &error));
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(LocalScreenshotResultSinkTest, SaveValidResultToExplicitPath)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.path() + QDir::separator() + QStringLiteral("shot.png");

    Result result;
    result.image = makeValidImage();
    result.globalSelection = QRect(0, 0, 10, 10);

    QString savedPath;
    QString error;
    ASSERT_TRUE(sink.save(result, path, &savedPath, &error));
    EXPECT_EQ(savedPath, path);
    EXPECT_TRUE(QFile::exists(path));
}

TEST_F(LocalScreenshotResultSinkTest, SaveAddsPngSuffixWhenMissing)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString basePath = dir.path() + QDir::separator() + QStringLiteral("nosuffix");

    Result result;
    result.image = makeValidImage();
    result.globalSelection = QRect(0, 0, 10, 10);

    QString savedPath;
    QString error;
    ASSERT_TRUE(sink.save(result, basePath, &savedPath, &error));
    EXPECT_TRUE(savedPath.endsWith(QStringLiteral(".png")));
    EXPECT_TRUE(QFile::exists(savedPath));
}

TEST_F(LocalScreenshotResultSinkTest, SaveWithEmptyPathUsesPicturesLocation)
{
    Result result;
    result.image = makeValidImage();
    result.globalSelection = QRect(0, 0, 10, 10);

    QString savedPath;
    QString error;
    ASSERT_TRUE(sink.save(result, QString(), &savedPath, &error));
    EXPECT_TRUE(savedPath.contains(QStringLiteral("Screenshots"), Qt::CaseInsensitive));
    EXPECT_TRUE(QFile::exists(savedPath));
    QFile::remove(savedPath);
}

TEST_F(LocalScreenshotResultSinkTest, CopyInvalidResultReturnsFalse)
{
    Result result;
    QString error;
    EXPECT_FALSE(sink.copy(result, &error));
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(LocalScreenshotResultSinkTest, CopyValidResultSetsClipboard)
{
    Result result;
    result.image = makeValidImage();
    result.globalSelection = QRect(0, 0, 10, 10);

    QString error;
    ASSERT_TRUE(sink.copy(result, &error));
    QImage clipboardImage = QGuiApplication::clipboard()->image();
    EXPECT_EQ(clipboardImage.size(), result.image.size());
}
