// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PINSAVEMENUMANAGER_H
#define UT_PINSAVEMENUMANAGER_H
#include <gtest/gtest.h>
#include <QStandardPaths>
#include <QAction>
#include <DMenu>
#include "ui/pinsavemenumanager.h"
#include "settings.h"

class TestPinSaveMenuManager : public testing::Test
{
public:
    void SetUp() override
    {
        Settings::release();
        Settings *s = Settings::instance();
        s->setSavePath(QString());
        s->setSaveOption(qMakePair(ASK, 0));
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
    QAction *askEveryTimeAction() const
    {
        QList<QAction*> acts = m_mgr->getMenu()->actions();
        return acts.isEmpty() ? nullptr : acts.first();
    }
    QAction *specifiedLocationMenuAction() const
    {
        QList<QAction*> acts = m_mgr->getMenu()->actions();
        return acts.size() > 1 ? acts.at(1) : nullptr;
    }
    DMenu *specifiedLocationSubMenu() const
    {
        QAction *ma = specifiedLocationMenuAction();
        return ma ? ma->menu() : nullptr;
    }
    QList<QAction*> locationActions() const
    {
        DMenu *sub = specifiedLocationSubMenu();
        return sub ? sub->actions() : QList<QAction*>();
    }
    void invokeSaveOptionSlot(QAction *action)
    {
        QMetaObject::invokeMethod(m_mgr, "onSaveOptionTriggered",
                                  Qt::DirectConnection, Q_ARG(QAction*, action));
    }
    void invokeLocationSlot(QAction *action)
    {
        QMetaObject::invokeMethod(m_mgr, "onLocationActionTriggered",
                                  Qt::DirectConnection, Q_ARG(QAction*, action));
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

TEST_F(TestPinSaveMenuManager, saveOptionTriggeredAskEveryTime)
{
    QAction *ask = askEveryTimeAction();
    ASSERT_NE(ask, nullptr);
    ask->setChecked(true);
    invokeSaveOptionSlot(ask);
    EXPECT_EQ(m_mgr->getCurrentSaveOption(), ASK);
}

TEST_F(TestPinSaveMenuManager, saveOptionTriggeredSpecifiedLocationWithDesktop)
{
    QList<QAction*> locActs = locationActions();
    ASSERT_GE(locActs.size(), 1);
    locActs.at(0)->setChecked(true);
    QAction *menuAction = specifiedLocationMenuAction();
    ASSERT_NE(menuAction, nullptr);
    invokeSaveOptionSlot(menuAction);
    EXPECT_EQ(m_mgr->getCurrentSaveOption(), DESKTOP);
}

TEST_F(TestPinSaveMenuManager, saveOptionTriggeredSpecifiedLocationNoChild)
{
    QAction *menuAction = specifiedLocationMenuAction();
    ASSERT_NE(menuAction, nullptr);
    menuAction->setChecked(true);
    invokeSaveOptionSlot(menuAction);
    EXPECT_EQ(m_mgr->getCurrentSaveOption(), ASK);
}

TEST_F(TestPinSaveMenuManager, saveOptionTriggeredUnknownAction)
{
    QAction unknown("unknown", nullptr);
    invokeSaveOptionSlot(&unknown);
    SUCCEED();
}

TEST_F(TestPinSaveMenuManager, locationActionTriggeredDesktop)
{
    QList<QAction*> locActs = locationActions();
    ASSERT_GE(locActs.size(), 1);
    invokeLocationSlot(locActs.at(0));
    EXPECT_EQ(m_mgr->getCurrentSaveOption(), DESKTOP);
}

TEST_F(TestPinSaveMenuManager, locationActionTriggeredPictures)
{
    QList<QAction*> locActs = locationActions();
    ASSERT_GE(locActs.size(), 2);
    invokeLocationSlot(locActs.at(1));
    EXPECT_EQ(m_mgr->getCurrentSaveOption(), PICTURES);
}

TEST_F(TestPinSaveMenuManager, locationActionTriggeredFolderChange)
{
    QList<QAction*> locActs = locationActions();
    ASSERT_GE(locActs.size(), 3);
    invokeLocationSlot(locActs.at(2));
    EXPECT_EQ(m_mgr->getCurrentSaveOption(), FOLDER_CHANGE);
}

TEST_F(TestPinSaveMenuManager, locationActionTriggeredUnknown)
{
    QAction unknown("unknown", nullptr);
    invokeLocationSlot(&unknown);
    SUCCEED();
}

#endif // UT_PINSAVEMENUMANAGER_H
