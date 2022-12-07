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

#include <memory>
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget

#include "gui/menus/popoverMenus/PageTypeSelectionPopoverBase.h"
#include "util/raii/GObjectSPtr.h"

class PageTemplateDialog;
class PageTypeHandler;
class PageTypeInfo;
class Settings;

class PageTypeSelectionPopoverGridOnly: public PageTypeSelectionPopoverBase {
public:
    PageTypeSelectionPopoverGridOnly(PageTypeHandler* typesHandler, const Settings* settings,
                                     PageTemplateDialog* parent);
    ~PageTypeSelectionPopoverGridOnly() override = default;

public:
    inline GtkWidget* getPopover() { return popover.get(); }

private:
    static GtkWidget* createPreviewGrid(const std::vector<std::unique_ptr<PageTypeInfo>>& pageTypes,
                                        const std::string_view& prefixedActionName);
    xoj::util::WidgetSPtr createPopover();
    void entrySelected(const PageTypeInfo* info) override;

private:
    PageTemplateDialog* parent;
    xoj::util::WidgetSPtr popover;

public:
    static constexpr auto G_ACTION_NAMESPACE = "popover";
    static constexpr auto SELECTION_ACTION_NAME = "select-page-type-template";
};
