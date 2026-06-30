// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QAction>
#include <QActionGroup>
#include <QMetaObject>
#include <QStandardPaths>
#include <QDir>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/widgets/savemenumanager.h"
#include "../../src/utils/configsettings.h"

using namespace testing;

// Covers the previously-uncovered private helpers and the deeper branch bodies
// of SaveMenuManager: updateConfigSettings, updateSubMenuForFirstTime,
// updateSubMenuForExistingPath, formatDisplayPath, getStandardPath, plus the
// interior branches of initializeFromConfig / onSaveOptionTriggered /
// onLocationActionTriggered that the existing _ext header does not reach.
//
// Config is mutated via ConfigSettings::setValue so the initializeFromConfig
// branches (Desktop/Pictures/saveOp legacy/save_dir_change/fixed-path) all
// execute. Private helpers are reached through ACCESS_PRIVATE_FUN and private
// state through ACCESS_PRIVATE_FIELD (declared once at namespace scope).

ACCESS_PRIVATE_FUN(SaveMenuManager, void(), updateConfigSettings);
ACCESS_PRIVATE_FUN(SaveMenuManager, void(), updateSubMenuForFirstTime);
ACCESS_PRIVATE_FUN(SaveMenuManager, void(), updateSubMenuForExistingPath);
ACCESS_PRIVATE_FUN(SaveMenuManager, QString(const QString &) const, formatDisplayPath);
ACCESS_PRIVATE_FUN(SaveMenuManager, QString(LocationState) const, getStandardPath);
ACCESS_PRIVATE_FIELD(SaveMenuManager, SaveOption, m_currentSaveOption);
ACCESS_PRIVATE_FIELD(SaveMenuManager, LocationState, m_currentLocationState);
ACCESS_PRIVATE_FIELD(SaveMenuManager, QString, m_currentCustomPath);

class SaveMenuManagerCovTest : public Test
{
public:
    SaveMenuManager *m_m = nullptr;
    void SetUp() override { m_m = new SaveMenuManager; }
    void TearDown() override { delete m_m; }

    void setShot(const char *key, const QVariant &v)
    {
        ConfigSettings::instance()->setValue("shot", key, v);
    }
    QVariant getShot(const char *key)
    {
        return ConfigSettings::instance()->getValue("shot", key);
    }
};

// ---------- private helper coverage ----------

TEST_F(SaveMenuManagerCovTest, updateConfigSettingsAllLocationStates)
{
    namespace apf = access_private_field;
    // AskEachTime -> save_ways=0
    apf::SaveMenuManagerm_currentSaveOption(*m_m) = SaveOption::AskEachTime;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SaveMenuManagerupdateConfigSettings(*m_m));
    EXPECT_EQ(getShot("save_ways").toInt(), 0);

    // SpecifiedLocation + Desktop
    apf::SaveMenuManagerm_currentSaveOption(*m_m) = SaveOption::SpecifiedLocation;
    apf::SaveMenuManagerm_currentLocationState(*m_m) = LocationState::Desktop;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SaveMenuManagerupdateConfigSettings(*m_m));
    EXPECT_EQ(getShot("save_op").toInt(), 1);

    // Pictures
    apf::SaveMenuManagerm_currentLocationState(*m_m) = LocationState::Pictures;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SaveMenuManagerupdateConfigSettings(*m_m));
    EXPECT_EQ(getShot("save_op").toInt(), 2);

    // CustomPath with a non-empty path -> writes save_dir
    apf::SaveMenuManagerm_currentCustomPath(*m_m) = QStringLiteral("/tmp/dsr_cov_dir");
    apf::SaveMenuManagerm_currentLocationState(*m_m) = LocationState::CustomPath;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SaveMenuManagerupdateConfigSettings(*m_m));
    EXPECT_EQ(getShot("save_op").toInt(), 3);

    // ChooseOnSave
    apf::SaveMenuManagerm_currentLocationState(*m_m) = LocationState::ChooseOnSave;
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SaveMenuManagerupdateConfigSettings(*m_m));
    EXPECT_EQ(getShot("save_dir_change").toBool(), true);
}

TEST_F(SaveMenuManagerCovTest, updateSubMenuForFirstTime)
{
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SaveMenuManagerupdateSubMenuForFirstTime(*m_m));
    SUCCEED();
}

TEST_F(SaveMenuManagerCovTest, updateSubMenuForExistingPath)
{
    access_private_field::SaveMenuManagerm_currentCustomPath(*m_m) = QStringLiteral("/tmp/dsr_cov_subdir");
    EXPECT_NO_FATAL_FAILURE(call_private_fun::SaveMenuManagerupdateSubMenuForExistingPath(*m_m));
    SUCCEED();
}

TEST_F(SaveMenuManagerCovTest, formatDisplayPathBranches)
{
    // empty -> placeholder
    QString empty = call_private_fun::SaveMenuManagerformatDisplayPath(*m_m, QString());
    EXPECT_FALSE(empty.isEmpty());

    // short folder name
    QString shortP = call_private_fun::SaveMenuManagerformatDisplayPath(*m_m, QStringLiteral("/tmp/short"));
    EXPECT_TRUE(shortP.contains(QStringLiteral("short")));

    // long folder name gets truncated
    QString longName = QStringLiteral("aaaaaaaaaaaaLongFolderName");
    QDir().mkpath(QStringLiteral("/tmp/") + longName);
    QString longP = call_private_fun::SaveMenuManagerformatDisplayPath(*m_m, QStringLiteral("/tmp/") + longName);
    EXPECT_TRUE(longP.contains(QStringLiteral("...")));
}

TEST_F(SaveMenuManagerCovTest, getStandardPathBranches)
{
    QString desk = call_private_fun::SaveMenuManagergetStandardPath(*m_m, LocationState::Desktop);
    EXPECT_FALSE(desk.isEmpty());
    QString pics = call_private_fun::SaveMenuManagergetStandardPath(*m_m, LocationState::Pictures);
    EXPECT_FALSE(pics.isEmpty());
    QString other = call_private_fun::SaveMenuManagergetStandardPath(*m_m, LocationState::ChooseOnSave);
    EXPECT_TRUE(other.isEmpty());
}

// ---------- initializeFromConfig branch coverage ----------

TEST_F(SaveMenuManagerCovTest, initializeAskEachTimeWithExistingPath)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov_ask"));
    setShot("save_ways", 0);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov_ask"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentSaveOption(), SaveOption::AskEachTime);
}

TEST_F(SaveMenuManagerCovTest, initializeAskEachTimeNoPath)
{
    setShot("save_ways", 0);
    setShot("save_dir", QString());
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    SUCCEED();
}

TEST_F(SaveMenuManagerCovTest, initializeSpecifiedLocationDesktop)
{
    setShot("save_ways", 1);
    setShot("location_state", static_cast<int>(LocationState::Desktop));
    setShot("save_dir", QString());
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentLocationState(), LocationState::Desktop);
}

TEST_F(SaveMenuManagerCovTest, initializeSpecifiedLocationPictures)
{
    setShot("save_ways", 1);
    setShot("location_state", static_cast<int>(LocationState::Pictures));
    setShot("save_dir", QString());
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentLocationState(), LocationState::Pictures);
}

TEST_F(SaveMenuManagerCovTest, initializeLegacySaveOp1)
{
    setShot("save_ways", 1);
    setShot("location_state", 999); // not Desktop/Pictures
    setShot("save_op", 1);
    setShot("save_dir", QString());
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentLocationState(), LocationState::Desktop);
}

TEST_F(SaveMenuManagerCovTest, initializeLegacySaveOp2)
{
    setShot("save_ways", 1);
    setShot("location_state", 999);
    setShot("save_op", 2);
    setShot("save_dir", QString());
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentLocationState(), LocationState::Pictures);
}

TEST_F(SaveMenuManagerCovTest, initializeSpecifiedChangeOnSave)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov_chg"));
    setShot("save_ways", 1);
    setShot("location_state", 999);
    setShot("save_op", 0);
    setShot("save_dir_change", true);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov_chg"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentLocationState(), LocationState::ChooseOnSave);
}

TEST_F(SaveMenuManagerCovTest, initializeSpecifiedFixedPath)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov_fixed"));
    setShot("save_ways", 1);
    setShot("location_state", 999);
    setShot("save_op", 0);
    setShot("save_dir_change", false);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov_fixed"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentLocationState(), LocationState::CustomPath);
}

TEST_F(SaveMenuManagerCovTest, initializeSpecifiedInvalidPathFallback)
{
    setShot("save_ways", 1);
    setShot("location_state", 999);
    setShot("save_op", 0);
    setShot("save_dir_change", false);
    setShot("save_dir", QStringLiteral("/no/such/dsr/path/xyz"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentLocationState(), LocationState::ChooseOnSave);
}

// ---------- slot interiors ----------

TEST_F(SaveMenuManagerCovTest, onSaveOptionTriggeredSpecifiedLocation)
{
    QSignalSpy spy(m_m, &SaveMenuManager::saveOptionChanged);
    QAction *specified = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (a->menu()) { // the "Specified location" action carries the sub-menu
            specified = a;
            break;
        }
    }
    ASSERT_NE(specified, nullptr);
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, specified)));
    EXPECT_GE(spy.count(), 1);
}

TEST_F(SaveMenuManagerCovTest, onLocationActionTriggeredAllActions)
{
    // Exercise each branch (chooseOnSave/desktop/pictures/customPath/updateOnSave)
    // by invoking the slot with every action found in the location sub-menu.
    DMenu *sub = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (a->menu()) {
            sub = qobject_cast<DMenu *>(a->menu());
            break;
        }
    }
    ASSERT_NE(sub, nullptr);
    for (QAction *locAct : sub->actions()) {
        EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onLocationActionTriggered",
            Qt::DirectConnection, Q_ARG(QAction *, locAct)));
    }
    SUCCEED();
}
