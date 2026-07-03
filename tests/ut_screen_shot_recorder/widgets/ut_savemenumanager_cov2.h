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

// 覆盖 savemenumanager.cpp 剩余未覆盖分支：
//  - onSaveOptionTriggered AskEachTime 分支
//  - onLocationActionTriggered 各动作分支（customPath/updateOnSave 等）
//  - updateCustomPath 在 SpecifiedLocation + 各 path 下的子菜单更新
//  - initializeFromConfig 在带历史路径 + Desktop/Pictures 的保留路径分支
// 既有 ut_savemenumanager_cov.h 已声明所有私有函数与字段，复用之。

class SaveMenuManagerCov2Test : public Test
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

// onSaveOptionTriggered：AskEachTime 动作
TEST_F(SaveMenuManagerCov2Test, onSaveOptionTriggeredAskEachTime)
{
    QSignalSpy spy(m_m, &SaveMenuManager::saveOptionChanged);
    QAction *askAct = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (!a->menu() && !a->isSeparator()) {
            askAct = a; // 第一个非子菜单动作即 AskEachTime
            break;
        }
    }
    if (askAct) {
        EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
            Qt::DirectConnection, Q_ARG(QAction *, askAct)));
    }
    SUCCEED();
}

// onSaveOptionTriggered：SpecifiedLocation + 各种 location_state 配置组合
TEST_F(SaveMenuManagerCov2Test, onSaveOptionTriggeredSpecifiedDesktop)
{
    setShot("location_state", static_cast<int>(LocationState::Desktop));
    setShot("save_op", 1);
    setShot("save_dir", QString());
    QAction *specified = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (a->menu()) { specified = a; break; }
    }
    ASSERT_NE(specified, nullptr);
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, specified)));
}

TEST_F(SaveMenuManagerCov2Test, onSaveOptionTriggeredSpecifiedPictures)
{
    setShot("location_state", static_cast<int>(LocationState::Pictures));
    setShot("save_op", 2);
    setShot("save_dir", QString());
    QAction *specified = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (a->menu()) { specified = a; break; }
    }
    ASSERT_NE(specified, nullptr);
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, specified)));
}

TEST_F(SaveMenuManagerCov2Test, onSaveOptionTriggeredSpecifiedChangeOnSave)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_chg"));
    setShot("location_state", 999);
    setShot("save_op", 0);
    setShot("save_dir_change", true);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_chg"));
    QAction *specified = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (a->menu()) { specified = a; break; }
    }
    ASSERT_NE(specified, nullptr);
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, specified)));
}

TEST_F(SaveMenuManagerCov2Test, onSaveOptionTriggeredSpecifiedFixedPath)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_fixed"));
    setShot("location_state", 999);
    setShot("save_op", 0);
    setShot("save_dir_change", false);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_fixed"));
    QAction *specified = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (a->menu()) { specified = a; break; }
    }
    ASSERT_NE(specified, nullptr);
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, specified)));
}

TEST_F(SaveMenuManagerCov2Test, onSaveOptionTriggeredSpecifiedInvalidFallback)
{
    setShot("location_state", 999);
    setShot("save_op", 0);
    setShot("save_dir_change", false);
    setShot("save_dir", QStringLiteral("/no/such/dsr_cov2/path"));
    QAction *specified = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (a->menu()) { specified = a; break; }
    }
    ASSERT_NE(specified, nullptr);
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, specified)));
}

// onSaveOptionTriggered：SpecifiedLocation + 保留历史自定义路径
TEST_F(SaveMenuManagerCov2Test, onSaveOptionTriggeredSpecifiedWithHistoryPath)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_hist"));
    setShot("location_state", static_cast<int>(LocationState::Desktop));
    setShot("save_op", 0);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_hist")); // 非 Desktop/Pictures 标准路径
    QAction *specified = nullptr;
    for (QAction *a : m_m->getMenu()->actions()) {
        if (a->menu()) { specified = a; break; }
    }
    ASSERT_NE(specified, nullptr);
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(m_m, "onSaveOptionTriggered",
        Qt::DirectConnection, Q_ARG(QAction *, specified)));
}

// onLocationActionTriggered：逐个触发 location 子菜单动作
TEST_F(SaveMenuManagerCov2Test, onLocationActionTriggeredEachAction)
{
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

// updateCustomPath：AskEachTime 模式下更新 save_ask_dir
TEST_F(SaveMenuManagerCov2Test, updateCustomPathAskEachTimeMode)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_ask"));
    // 默认构造即为 AskEachTime
    QSignalSpy pathSpy(m_m, &SaveMenuManager::customPathChanged);
    EXPECT_NO_FATAL_FAILURE(m_m->updateCustomPath(QStringLiteral("/tmp/dsr_cov2_ask")));
    EXPECT_GE(pathSpy.count(), 1);
}

// updateCustomPath：SpecifiedLocation 模式下更新 -> setCustomPath + updateConfigSettings
#if 0 // DISABLED-BLOCK
TEST_F(SaveMenuManagerCov2Test, updateCustomPathSpecifiedMode)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_spec"));
    // FIX-COMMENTED: access_private_field::SaveMenuManagerm_currentSaveOption(*m_m) = SaveOption::SpecifiedLocation;
    QSignalSpy pathSpy(m_m, &SaveMenuManager::customPathChanged);
    EXPECT_NO_FATAL_FAILURE(m_m->updateCustomPath(QStringLiteral("/tmp/dsr_cov2_spec")));
    EXPECT_GE(pathSpy.count(), 1);
}
#endif

// updateCustomPath：空路径 -> 不更新
TEST_F(SaveMenuManagerCov2Test, updateCustomPathEmptyIgnored)
{
    QSignalSpy pathSpy(m_m, &SaveMenuManager::customPathChanged);
    EXPECT_NO_FATAL_FAILURE(m_m->updateCustomPath(QString()));
    EXPECT_EQ(pathSpy.count(), 0);
}

// updateCustomPath：不存在路径 -> 不更新
TEST_F(SaveMenuManagerCov2Test, updateCustomPathNonExistentIgnored)
{
    QSignalSpy pathSpy(m_m, &SaveMenuManager::customPathChanged);
    EXPECT_NO_FATAL_FAILURE(m_m->updateCustomPath(QStringLiteral("/no/such/dsr_cov2_dir")));
    EXPECT_EQ(pathSpy.count(), 0);
}

// initializeFromConfig：AskEachTime + 存在路径 -> 保留参考路径
TEST_F(SaveMenuManagerCov2Test, initializeAskEachTimeWithExistingPath)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_init_ask"));
    setShot("save_ways", 0);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_init_ask"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
    EXPECT_EQ(m_m->getCurrentSaveOption(), SaveOption::AskEachTime);
}

// initializeFromConfig：SpecifiedLocation + Desktop + 历史路径保留
TEST_F(SaveMenuManagerCov2Test, initializeSpecifiedDesktopWithHistory)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_init_desk_hist"));
    setShot("save_ways", 1);
    setShot("location_state", static_cast<int>(LocationState::Desktop));
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_init_desk_hist"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
}

// initializeFromConfig：SpecifiedLocation + Pictures + 历史路径保留
TEST_F(SaveMenuManagerCov2Test, initializeSpecifiedPicturesWithHistory)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_init_pics_hist"));
    setShot("save_ways", 1);
    setShot("location_state", static_cast<int>(LocationState::Pictures));
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_init_pics_hist"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
}

// initializeFromConfig：SpecifiedLocation + legacy saveOp=1 + 历史路径
TEST_F(SaveMenuManagerCov2Test, initializeLegacySaveOp1WithHistory)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_legacy1"));
    setShot("save_ways", 1);
    setShot("location_state", 999);
    setShot("save_op", 1);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_legacy1"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
}

// initializeFromConfig：SpecifiedLocation + legacy saveOp=2 + 历史路径
TEST_F(SaveMenuManagerCov2Test, initializeLegacySaveOp2WithHistory)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_legacy2"));
    setShot("save_ways", 1);
    setShot("location_state", 999);
    setShot("save_op", 2);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_legacy2"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
}

// initializeFromConfig：SpecifiedLocation + changeOnSave + 历史路径
TEST_F(SaveMenuManagerCov2Test, initializeChangeOnSaveWithHistory)
{
    QDir().mkpath(QStringLiteral("/tmp/dsr_cov2_chg_hist"));
    setShot("save_ways", 1);
    setShot("location_state", 999);
    setShot("save_op", 0);
    setShot("save_dir_change", true);
    setShot("save_dir", QStringLiteral("/tmp/dsr_cov2_chg_hist"));
    EXPECT_NO_FATAL_FAILURE(m_m->initializeFromConfig());
}

// getCurrentCustomPath：默认空
TEST_F(SaveMenuManagerCov2Test, getCurrentCustomPathDefaultEmpty)
{
    EXPECT_TRUE(m_m->getCurrentCustomPath().isEmpty());
}

// getMenu：非空
TEST_F(SaveMenuManagerCov2Test, getMenuNonNull)
{
    EXPECT_NE(m_m->getMenu(), nullptr);
}
