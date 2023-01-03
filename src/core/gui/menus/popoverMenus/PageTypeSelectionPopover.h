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

#include "control/Control.h"
#include "control/settings/PageTemplateSettings.h"
#include "gui/menus/popoverMenus/PageTypeSelectionPopoverBase.h"
#include "model/PaperSize.h"
#include "util/PaperFormatUtils.h"
#include "util/raii/GObjectSPtr.h"

class PageBackgroundChangeController;
class PageTypeHandler;
class PageTypeInfo;
class Settings;

class PageTypeSelectionPopover: public PageTypeSelectionPopoverBase {
public:
    PageTypeSelectionPopover(Control* control, Settings* settings, GtkApplicationWindow* win);
    ~PageTypeSelectionPopover() override = default;

public:
    inline GtkWidget* getPopover() { return popover.get(); }

    /**
     * @brief Set the selected radio button.
     *      This specialization is a workaround for a GTK-3 bug, where an infinite loop starts when grouped radio
     * buttons are linked to a GAction, and the state of this action is changed.
     */
    void setSelectedPageType(const std::optional<PageType>& selected) override;

    /**
     * @brief Sets the selected paper size of the menu.
     * @tparam changeComboBoxSelection Whether the combo box selection will be changed to a fitting option
     */
    template <bool changeComboBoxSelection = true>
    void setSelectedPaperSize(const std::optional<PaperSize>& newPageSize);

private:
    /**
     * @brief Create a togglebutton containing a miniature of the given (standard) page type
     *      The toggle button is initialized to follow a GAction with the given action name and target value entryNb.
     *      The returned widget is a floating ref.
     * @param radioGroup previously added radio button so that they are grouped together. nullptr for the first button
     *
     * Todo(gtk4): in gtk4 the radio buttons will be grouped simply because they share the same GAction:
     *           the parameter radioGroup can then be removed
     */
    static GtkWidget* createEntryWithPreview(const PageTypeInfo* pti, size_t entryNb,
                                             const std::string_view& prefixedActionName, GtkWidget*& radioGroup);
    /**
     * @brief Create a grid containing a miniature for each one the given (standard) page types
     *      The returned widget is a floating ref.
     *
     * @param radioGroup Address to which the last added radio button address will be recorded, for grouping purposes..
     *
     * Todo(gtk4): in gtk4 the radio buttons will be grouped simply because they share the same GAction:
     *           the parameter radioGroup can then be removed
     */
    static GtkWidget* createPreviewGrid(const std::vector<std::unique_ptr<PageTypeInfo>>& pageTypes,
                                        const std::string_view& prefixedActionName, GtkWidget*& radioGroup);
    xoj::util::WidgetSPtr createPopover();
    void entrySelected(const PageTypeInfo* info) override;

    static GSimpleAction* createOrientationGAction(uint8_t orientation);
    static void changedOrientationSelectionCallback(GSimpleAction* ga, GVariant* parameter,
                                                   PageTypeSelectionPopover* self);
    static void changedPaperFormatTemplateCb(GtkComboBox* widget, PageTypeSelectionPopover* dlg);
    std::optional<PaperSize> getInitiallySelectedPaperSize();
    int getComboBoxIndexForPaperSize(const std::optional<PaperSize>& paperSize);

    GtkWidget* createOrientationButton(std::string_view actionName, GtkOrientation orientation, std::string_view icon);
private:
    Control* control;
    PageBackgroundChangeController* controller;
    Settings* settings;

    std::optional<PaperSize> selectedPageSize;

    GtkOrientation selectedOrientation;
    xoj::util::GObjectSPtr<GSimpleAction> orientationAction;

    PaperFormatUtils::PaperFormatMenuOptionVector_t paperSizeMenuOptions;

    xoj::util::WidgetSPtr pageFormatComboBox;
    /**
     * @brief Pointer to one of the radio button, so that the radio button group is readily accessible.
     *
     * Todo(gtk4): remove this
     */
    xoj::util::WidgetSPtr radioButtonGroup;

    xoj::util::WidgetSPtr applyToCurrentPageButton;
    xoj::util::WidgetSPtr popover;

public:
    static constexpr auto G_ACTION_NAMESPACE = "win.";
    static constexpr auto PAGETYPE_SELECTION_ACTION_NAME = "select-page-type-of-new-page";
    static constexpr auto ORIENTATION_SELECTION_ACTION_NAME = "select-page-orientation-of-new-page";
};
