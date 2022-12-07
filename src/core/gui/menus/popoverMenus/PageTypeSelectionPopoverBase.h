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

#include <string_view>

#include <gtk/gtk.h>  // for GtkWidget

#include "gui/menus/PageTypeSelectionMenuBase.h"

class PageTypeHandler;
class PageTypeInfo;
class Settings;

/**
 * @brief Base class for popovers containing various page types to choose from
 */
class PageTypeSelectionPopoverBase: public PageTypeSelectionMenuBase {
public:
    PageTypeSelectionPopoverBase(PageTypeHandler* typesHandler, const Settings* settings,
                                 const std::string_view& actionName);
    ~PageTypeSelectionPopoverBase() override = default;

protected:
    /**
     * @brief Create a GtkImage containing a miniature of the given (standard) page type
     *      The returned widget is a floating ref.
     */
    static GtkWidget* createPreviewImage(const PageTypeInfo& pti);

protected:
    static constexpr auto PAGE_TYPES_PER_ROW = 3;
};
