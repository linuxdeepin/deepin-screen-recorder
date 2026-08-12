// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PINSAVEMENUMANAGER_H
#define UT_PINSAVEMENUMANAGER_H
#include <gtest/gtest.h>
#include <QStandardPaths>
#include "ui/pinsavemenumanager.h"
#include "settings.h"

class TestPinSaveMenuManager : public testing::Test
{
public:
    void SetUp() override
    {
        Settings::release();
        m_mgr = new PinSaveMenuManager(nullptr);
    }
    void TearDown() override
    {
        if (m_mgr) {
            delete m_mgr;
            m_mgr = nullptr;
        }
        Settings::release();
    }
    PinSaveMenuManager *m_mgr;
};

TEST_F(TestPinSaveMenuManager, getMenuNotNull)
{
    EXPECT_NE(m_mgr->getMenu(), nullptr);
}

TEST_F(TestPinSaveMenuManager, getCurrentSaveOptionIsValid)
{
    int opt = m_mgr->getCurrentSaveOption();
    EXPECT_TRUE(opt == ASK || opt == DESKTOP || opt == PICTURES ||
                opt == FOLDER || opt == FOLDER_CHANGE);
}

TEST_F(TestPinSaveMenuManager, updateCustomPathNonexistentDoesNotCrash)
{
    m_mgr->updateCustomPath("/nonexistent/path/that/does/not/exist");
    SUCCEED();
}

TEST_F(TestPinSaveMenuManager, updateCustomPathValid)
{
    QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_mgr->updateCustomPath(tmpDir);
    SUCCEED();
}

#endif // UT_PINSAVEMENUMANAGER_H
