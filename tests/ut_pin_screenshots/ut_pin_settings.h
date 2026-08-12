// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PIN_SETTINGS_H
#define UT_PIN_SETTINGS_H
#include <gtest/gtest.h>
#include "settings.h"
#include "ui/pinsavemenumanager.h"

class TestPinSettings : public testing::Test
{
public:
    void SetUp() override
    {
        Settings::release();
        m_s = Settings::instance();
    }
    void TearDown() override
    {
        Settings::release();
    }
    Settings *m_s;
};

TEST_F(TestPinSettings, instanceSingleton)
{
    Settings *s1 = Settings::instance();
    Settings *s2 = Settings::instance();
    EXPECT_EQ(s1, s2);
}

TEST_F(TestPinSettings, saveOptionRoundTrip)
{
    QPair<int, int> opt(DESKTOP, 1);
    m_s->setSaveOption(opt);
    QPair<int, int> result = m_s->getSaveOption();
    EXPECT_EQ(result.first, DESKTOP);
    EXPECT_EQ(result.second, 1);
}

TEST_F(TestPinSettings, savePathRoundTrip)
{
    m_s->setSavePath("/tmp/test_pin_savepath");
    EXPECT_EQ(m_s->getSavePath().toStdString(), "/tmp/test_pin_savepath");
}

TEST_F(TestPinSettings, askSavePathRoundTrip)
{
    m_s->setAskSavePath("/tmp/test_pin_askpath");
    EXPECT_EQ(m_s->getAskSavePath().toStdString(), "/tmp/test_pin_askpath");
}

TEST_F(TestPinSettings, isChangeSavePathRoundTrip)
{
    m_s->setIsChangeSavePath(true);
    EXPECT_TRUE(m_s->getIsChangeSavePath());
    m_s->setIsChangeSavePath(false);
    EXPECT_FALSE(m_s->getIsChangeSavePath());
}

#endif // UT_PIN_SETTINGS_H
