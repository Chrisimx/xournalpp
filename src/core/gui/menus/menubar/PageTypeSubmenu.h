/*
 * Xournal++
 *
 * Handles page selection menu
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gio/gio.h>  // for GMenu, GSimpleAction

#include "gui/menus/PageTypeSelectionMenuBase.h"
#include "util/raii/GObjectSPtr.h"

#include "AbstractSubmenu.h"

class PageTypeHandler;
class Settings;
class PageBackgroundChangeController;

class PageTypeSubmenu: public Submenu, public PageTypeSelectionMenuBase {
public:
    PageTypeSubmenu(PageTypeHandler* typesHandler, PageBackgroundChangeController* controller, const Settings* settings,
                    GtkApplicationWindow* win);
    ~PageTypeSubmenu() override = default;

    void setDisabled(bool disabled) override;
    void addToMenubar(MainWindow* win) override;

private:
    void entrySelected(const PageTypeInfo*) override;

    PageBackgroundChangeController* controller;

    xoj::util::GObjectSPtr<GMenu> generatedPageTypesSection;
    xoj::util::GObjectSPtr<GMenu> specialPageTypesSection;
    xoj::util::GObjectSPtr<GMenu> applyToAllPagesSection;
    xoj::util::GObjectSPtr<GSimpleAction> applyToAllPagesAction;

public:
    static constexpr auto G_ACTION_NAMESPACE = "win.";
    static constexpr auto SELECTION_ACTION_NAME = "menu.pick-page-type";
    static constexpr auto APPLY_ALL_ACTION_NAME = "menu.apply-current-page-type-to-all-pages";
    static constexpr auto SUBMENU_ID = "menuJournalPaperBackground";
};
