/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Tests/ActionManager/ActionManagerFixture.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <QMenu>
#include <QMenuBar>

namespace UnitTest
{
    TEST_F(ActionManagerFixture, RegisterMenu)
    {
        auto outcome = m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        EXPECT_TRUE(outcome.IsSuccess());
    }

    TEST_F(ActionManagerFixture, RegisterMenuTwice)
    {
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        auto outcome = m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        EXPECT_FALSE(outcome.IsSuccess());
    }

    TEST_F(ActionManagerFixture, RegisterMenuBar)
    {
        auto outcome = m_menuManagerInterface->RegisterMenuBar("o3de.menubar.test");
        EXPECT_TRUE(outcome.IsSuccess());
    }

    TEST_F(ActionManagerFixture, RegisterMenuBarTwice)
    {
        m_menuManagerInterface->RegisterMenuBar("o3de.menubar.test");
        auto outcome = m_menuManagerInterface->RegisterMenuBar("o3de.menubar.test");
        EXPECT_FALSE(outcome.IsSuccess());
    }

    TEST_F(ActionManagerFixture, AddActionToUnregisteredMenu)
    {
        m_actionManagerInterface->RegisterActionContext("", "o3de.context.test", {}, m_widget);
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test", {}, []{});

        auto outcome = m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test", 42);
        EXPECT_FALSE(outcome.IsSuccess());
    }

    TEST_F(ActionManagerFixture, AddActionToMenu)
    {
        m_actionManagerInterface->RegisterActionContext("", "o3de.context.test", {}, m_widget);
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test", {}, []{});
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});

        auto outcome = m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test", 42);
        EXPECT_TRUE(outcome.IsSuccess());
    }

    TEST_F(ActionManagerFixture, GetUnregisteredMenu)
    {
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        EXPECT_TRUE(menu == nullptr);
    }

    TEST_F(ActionManagerFixture, GetMenu)
    {
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});

        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        EXPECT_TRUE(menu != nullptr);
    }

    TEST_F(ActionManagerFixture, VerifyActionInMenu)
    {
        // Register menu, get it and verify it's empty.
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        EXPECT_EQ(menu->actions().size(), 0);

        // Register a new action and add it to the menu.
        m_actionManagerInterface->RegisterActionContext("", "o3de.context.test", {}, m_widget);
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test", {}, []{});
        m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test", 42);

        // Verify the action is now in the menu.
        EXPECT_EQ(menu->actions().size(), 1);
    }

    TEST_F(ActionManagerFixture, VerifyActionInMenuTwice)
    {
        // Register menu, get it and verify it's empty.
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        EXPECT_EQ(menu->actions().size(), 0);

        // Register a new action and add it to the menu twice.
        m_actionManagerInterface->RegisterActionContext("", "o3de.context.test", {}, m_widget);
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test", {}, []{});
        m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test", 42);
        m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test", 42);

        // Verify the action was added to the menu just once.
        // This is the correct behavior.
        EXPECT_EQ(menu->actions().size(), 1);
    }

    TEST_F(ActionManagerFixture, VerifyActionOrderInMenu)
    {
        // Register menu, get it and verify it's empty.
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        EXPECT_EQ(menu->actions().size(), 0);

        // Register a new action and add it to the menu.
        m_actionManagerInterface->RegisterActionContext("", "o3de.context.test", {}, m_widget);
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test1", {}, []{});
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test2", {}, []{});
        m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test2", 42);
        m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test1", 1);

        // Verify the actions are now in the menu.
        EXPECT_EQ(menu->actions().size(), 2);

        // Verify the order is correct.
        QAction* test1 = m_actionManagerInterface->GetAction("o3de.action.test1");
        QAction* test2 = m_actionManagerInterface->GetAction("o3de.action.test2");

        const auto& actions = menu->actions();
        EXPECT_EQ(actions[0], test1);
        EXPECT_EQ(actions[1], test2);
    }

    TEST_F(ActionManagerFixture, VerifyActionOrderInMenuWithCollision)
    {
        // Register menu, get it and verify it's empty.
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        EXPECT_EQ(menu->actions().size(), 0);

        // Register a new action and add it to the menu.
        m_actionManagerInterface->RegisterActionContext("", "o3de.context.test", {}, m_widget);
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test1", {}, []{});
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test2", {}, []{});
        m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test2", 42);
        m_menuManagerInterface->AddActionToMenu("o3de.menu.test", "o3de.action.test1", 42);

        // Verify the actions are now in the menu.
        EXPECT_EQ(menu->actions().size(), 2);

        // Verify the order is correct (when a collision happens, items should be in order of addition).
        QAction* test1 = m_actionManagerInterface->GetAction("o3de.action.test1");
        QAction* test2 = m_actionManagerInterface->GetAction("o3de.action.test2");

        const auto& actions = menu->actions();
        EXPECT_EQ(actions[0], test2);
        EXPECT_EQ(actions[1], test1);
    }

    TEST_F(ActionManagerFixture, VerifySeparatorInMenu)
    {
        // Register menu, get it and verify it's empty.
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        EXPECT_EQ(menu->actions().size(), 0);

        // Add a separator to the menu.
        m_menuManagerInterface->AddSeparatorToMenu("o3de.menu.test", 42);

        // Verify the separator is now in the menu.
        const auto& actions = menu->actions();

        EXPECT_EQ(actions.size(), 1);
        EXPECT_TRUE(actions[0]->isSeparator());
    }

    TEST_F(ActionManagerFixture, VerifySubMenuInMenu)
    {
        // Register menu and submenu.
        m_menuManagerInterface->RegisterMenu("o3de.menu.testMenu", {});
        m_menuManagerInterface->RegisterMenu("o3de.menu.testSubMenu", {});

        // Add the sub-menu to the menu.
        m_menuManagerInterface->AddSubMenuToMenu("o3de.menu.testMenu", "o3de.menu.testSubMenu", 42);

        // Verify the sub-menu is now in the menu.
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.testMenu");
        QMenu* submenu = m_menuManagerInterface->GetMenu("o3de.menu.testSubMenu");
        const auto& actions = menu->actions();

        EXPECT_EQ(actions.size(), 1);
        EXPECT_EQ(actions[0]->menu(), submenu);
    }

    TEST_F(ActionManagerFixture, AddNullWidgetInMenu)
    {
        // Register menu.
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});

        // Try to add a nullptr widget.
        auto outcome = m_menuManagerInterface->AddWidgetToMenu("o3de.menu.test", nullptr, 42);
        EXPECT_FALSE(outcome.IsSuccess());
    }

    TEST_F(ActionManagerFixture, VerifyWidgetInMenu)
    {
        // Register menu and create a QWidget.
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});
        QWidget* widget = new QWidget();

        // Add the widget to the menu.
        m_menuManagerInterface->AddWidgetToMenu("o3de.menu.test", widget, 42);

        // Verify the widget is now in the menu.
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        const auto& actions = menu->actions();

        EXPECT_EQ(actions.size(), 1);

        QWidgetAction* widgetAction = qobject_cast<QWidgetAction*>(actions[0]);
        EXPECT_TRUE(widgetAction != nullptr);
        EXPECT_TRUE(widgetAction->defaultWidget() == widget);
    }

    TEST_F(ActionManagerFixture, VerifyComplexMenu)
    {
        // Combine multiple actions, separators and submenus.
        m_menuManagerInterface->RegisterMenu("o3de.menu.testMenu", {});
        m_menuManagerInterface->RegisterMenu("o3de.menu.testSubMenu", {});

        m_actionManagerInterface->RegisterActionContext("", "o3de.context.test", {}, m_widget);
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test1", {}, []{});
        m_actionManagerInterface->RegisterAction("o3de.context.test", "o3de.action.test2", {}, []{});

        // Create a menu with this setup. Order of addition is intentionally scrambled to verify sortKeys.
        // - Test 1 Action
        // - Test 2 Action
        // - Separator
        // - SubMenu
        //   - Test 2 Action
        // 
        // Note: it is legal to add the same action to multiple different menus.
        m_menuManagerInterface->AddActionToMenu("o3de.menu.testMenu", "o3de.action.test2", 12);
        m_menuManagerInterface->AddActionToMenu("o3de.menu.testSubMenu", "o3de.action.test2", 1);
        m_menuManagerInterface->AddSubMenuToMenu("o3de.menu.testMenu", "o3de.menu.testSubMenu", 42);
        m_menuManagerInterface->AddActionToMenu("o3de.menu.testMenu", "o3de.action.test1", 11);
        m_menuManagerInterface->AddSeparatorToMenu("o3de.menu.testMenu", 18);

        // Verify the actions are now in the menu in the expected order.
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.testMenu");
        QMenu* submenu = m_menuManagerInterface->GetMenu("o3de.menu.testSubMenu");
        QAction* test1 = m_actionManagerInterface->GetAction("o3de.action.test1");
        QAction* test2 = m_actionManagerInterface->GetAction("o3de.action.test2");

        // Note: separators and sub-menus are still QActions in the context of the menu.
        EXPECT_EQ(menu->actions().size(), 4);

        // Verify the order is correct.
        const auto& actions = menu->actions();
        EXPECT_EQ(actions[0], test1);
        EXPECT_EQ(actions[1], test2);
        EXPECT_TRUE(actions[2]->isSeparator());
        EXPECT_EQ(actions[3]->menu(), submenu);

        const auto& subactions = submenu->actions();
        EXPECT_EQ(subactions[0], test2);
    }
    
    TEST_F(ActionManagerFixture, AddMenuToUnregisteredMenuBar)
    {
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});

        auto outcome = m_menuManagerInterface->AddMenuToMenuBar("o3de.menubar.test", "o3de.menu.test", 42);
        EXPECT_FALSE(outcome.IsSuccess());
    }
    
    TEST_F(ActionManagerFixture, AddMenuToMenuBar)
    {
        m_menuManagerInterface->RegisterMenuBar("o3de.menubar.test");
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});

        auto outcome = m_menuManagerInterface->AddMenuToMenuBar("o3de.menubar.test", "o3de.menu.test", 42);
        EXPECT_TRUE(outcome.IsSuccess());
    }

    TEST_F(ActionManagerFixture, GetMenuBar)
    {
        m_menuManagerInterface->RegisterMenuBar("o3de.menubar.test");

        QMenuBar* menuBar = m_menuManagerInterface->GetMenuBar("o3de.menubar.test");
        EXPECT_TRUE(menuBar != nullptr);
    }

    TEST_F(ActionManagerFixture, VerifyMenuInMenuBar)
    {
        // Register menu bar, get it and verify it's empty.
        m_menuManagerInterface->RegisterMenuBar("o3de.menubar.test");
        m_menuManagerInterface->RegisterMenu("o3de.menu.test", {});

        // Add the menu to the menu bar.
        m_menuManagerInterface->AddMenuToMenuBar("o3de.menubar.test", "o3de.menu.test", 42);

        // Verify the submenu is now in the menu.
        QMenuBar* menubar = m_menuManagerInterface->GetMenuBar("o3de.menubar.test");
        QMenu* menu = m_menuManagerInterface->GetMenu("o3de.menu.test");
        const auto& actions = menubar->actions();

        EXPECT_EQ(actions.size(), 1);
        EXPECT_EQ(actions[0]->menu(), menu);
    }

    TEST_F(ActionManagerFixture, VerifyComplexMenuBar)
    {
        // Register multiple menus.
        m_menuManagerInterface->RegisterMenuBar("o3de.menubar.test");
        m_menuManagerInterface->RegisterMenu("o3de.menu.testMenu1", {});
        m_menuManagerInterface->RegisterMenu("o3de.menu.testMenu2", {});
        m_menuManagerInterface->RegisterMenu("o3de.menu.testMenu3", {});

        // Create a menu bar with this setup. Order of addition is intentionally scrambled to verify sortKeys.
        // - Menu 1
        // - Menu 2
        // - Menu 3
        
        m_menuManagerInterface->AddMenuToMenuBar("o3de.menubar.test", "o3de.menu.testMenu2", 42);
        m_menuManagerInterface->AddMenuToMenuBar("o3de.menubar.test", "o3de.menu.testMenu3", 42);
        m_menuManagerInterface->AddMenuToMenuBar("o3de.menubar.test", "o3de.menu.testMenu1", 16);

        // Verify the menus are now in the menu bar in the expected order.
        QMenuBar* menubar = m_menuManagerInterface->GetMenuBar("o3de.menubar.test");
        QMenu* testMenu1 = m_menuManagerInterface->GetMenu("o3de.menu.testMenu1");
        QMenu* testMenu2 = m_menuManagerInterface->GetMenu("o3de.menu.testMenu2");
        QMenu* testMenu3 = m_menuManagerInterface->GetMenu("o3de.menu.testMenu3");

        // Note: menus are represented via a QAction with a submenu property in Qt.
        EXPECT_EQ(menubar->actions().size(), 3);

        // Verify the order is correct.
        const auto& actions = menubar->actions();
        EXPECT_EQ(actions[0]->menu(), testMenu1);
        EXPECT_EQ(actions[1]->menu(), testMenu2);
        EXPECT_EQ(actions[2]->menu(), testMenu3);
    }

} // namespace UnitTest
