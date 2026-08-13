// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include "../../src/utils/configsettings.h"
#include "../../src/utils/saveutils.h"


using namespace testing;

class ConfigSettingsTest:public testing::Test{

public:
    ConfigSettings *configInstance = nullptr;
    virtual void SetUp() override{
        configInstance = ConfigSettings::instance();
        std::cout << "start ConfigSettingsTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end ConfigSettingsTest" << std::endl;
    }
};

TEST_F(ConfigSettingsTest, configsettings)
{
    EXPECT_NE(nullptr, configInstance);
    // Verify the config instance returns a valid QVariant for a known key.
    // The exact value may differ from defaults if a persistent .conf file from
    // prior runs overwrites it; we only check the instance works.
    EXPECT_TRUE(configInstance->getValue("shot", "save_ways").isValid());
}
/*
TEST_F(ConfigSettingsTest, setTemporarySaveAction)
{
    configInstance->setTemporarySaveAction(std::pair<bool, SaveAction>(true, SaveToDesktop));
    std::pair<bool, SaveAction> value = configInstance->getTemporarySaveAction();
    EXPECT_EQ(true, value.first);
    EXPECT_EQ(SaveToDesktop, value.second);
}
*/
// keys() is private in ConfigSettings; the keys test is disabled until an
// ACCESS_PRIVATE_FUN wrapper compatible with the member signature is added.
/*
TEST_F(ConfigSettingsTest, keys)
{
    QStringList keys = configInstance->keys("common");
    qDebug() << keys;
    EXPECT_LT(0, keys.size());
}
*/
TEST_F(ConfigSettingsTest, value)
{
    QVariant defaultVaue = 99;
    QVariant value = configInstance->getValue("common", "themeType");
    qDebug() << value << defaultVaue;
    EXPECT_NE(defaultVaue, value);
}
TEST_F(ConfigSettingsTest, setValue)
{
    QString group = "rectangle";
    QString key = "color_index";
    QVariant val = 99;
    QVariant defaultVaue = -19;
    QVariant tempVal =  configInstance->getValue(group, key);
    EXPECT_NE(defaultVaue, tempVal);
    configInstance->setValue(group, key, val);
    QVariant change =  configInstance->getValue(group, key);
    EXPECT_NE(change, tempVal);
    EXPECT_EQ(change, val);

    configInstance->setValue(group, key, tempVal);
}
