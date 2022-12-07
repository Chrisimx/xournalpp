#include "PageTypeSelectionPopover.h"

#include <cassert>
#include <memory>  // for unique_ptr
#include <string>  // for string

#include <glib.h>
#include <gtk/gtkactionable.h>

#include "control/PageBackgroundChangeController.h"
#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo
#include "gui/menus/StaticAssertActionNamespace.h"
#include "model/PageType.h"  // for PageType
#include "util/GListView.h"
#include "util/Util.h"
#include "util/gtk4_helper.h"          // for gtk_box_append
#include "util/i18n.h"                 // for _
#include "util/raii/CLibrariesSPtr.h"  // for adopt, ref


namespace {
GtkWidget* createEntryWithoutPreview(const char* label, size_t entryNb, const std::string_view& prefixedActionName,
                                     GtkWidget*& radioGroup) {
    // Todo(gtk4): GtkWidget* button = gtk_check_button_new();
    //      In GTK 4, the check buttons linked to the same GAction are drawn as radio buttons.
    //      The parameter radioGroup will then be useless
    GtkWidget* button = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radioGroup));
    radioGroup = button;

    gtk_check_button_set_label(GTK_CHECK_BUTTON(button), label);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), prefixedActionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(button), g_variant_new_uint64(entryNb));
    return button;
}
};  // namespace

PageTypeSelectionPopover::PageTypeSelectionPopover(PageTypeHandler* typesHandler,
                                                   PageBackgroundChangeController* controller, const Settings* settings,
                                                   GtkApplicationWindow* win):
        PageTypeSelectionPopoverBase(typesHandler, settings, SELECTION_ACTION_NAME),
        controller(controller),
        popover(createPopover()) {
    static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));

    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(typeSelectionAction.get()));
}

GtkWidget* PageTypeSelectionPopover::createEntryWithPreview(const PageTypeInfo* pti, size_t entryNb,
                                                            const std::string_view& prefixedActionName,
                                                            GtkWidget*& radioGroup) {
    // Todo(gtk4): GtkWidget* button = gtk_check_button_new();
    //      In GTK 4, the check buttons linked to the same GAction are drawn as radio buttons.
    //      The parameter radioGroup will then be useless
    GtkWidget* button = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(radioGroup));
    radioGroup = button;

    // // Use to restore labels in the menu
    // GtkWidget* label = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    // gtk_box_append(GTK_BOX(label), createPreviewImage(*pti));
    // gtk_box_append(GTK_BOX(label), gtk_label_new(pti->name.c_str()));
    // gtk_button_set_child(GTK_BUTTON(button), label);

    gtk_button_set_child(GTK_BUTTON(button), createPreviewImage(*pti));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), prefixedActionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(button), g_variant_new_uint64(entryNb));
    return button;
}

GtkWidget* PageTypeSelectionPopover::createPreviewGrid(const std::vector<std::unique_ptr<PageTypeInfo>>& pageTypes,
                                                       const std::string_view& prefixedActionName,
                                                       GtkWidget*& radioGroup) {
    GtkGrid* grid = GTK_GRID(gtk_grid_new());

    gtk_grid_set_column_homogeneous(grid, true);
    gtk_grid_set_column_spacing(grid, 10);
    gtk_grid_set_row_spacing(grid, 10);
    gtk_widget_set_margin_top(GTK_WIDGET(grid), 10);
    gtk_widget_set_margin_start(GTK_WIDGET(grid), 10);
    gtk_widget_set_margin_end(GTK_WIDGET(grid), 10);

    size_t n = 0;
    int gridX = 0;
    int gridY = 0;

    for (const auto& pageInfo: pageTypes) {
        // Special page types do not get a preview
        assert(!pageInfo->page.isSpecial());
        auto* entry = createEntryWithPreview(pageInfo.get(), n++, prefixedActionName, radioGroup);
        if (gridX >= PageTypeSelectionPopoverBase::PAGE_TYPES_PER_ROW) {
            gridX = 0;
            gridY++;
        }
        gtk_grid_attach(grid, entry, gridX, gridY, 1, 1);
        gridX++;
    }
    return GTK_WIDGET(grid);
}

xoj::util::WidgetSPtr PageTypeSelectionPopover::createPopover() {
    xoj::util::WidgetSPtr popover(gtk_popover_new(), xoj::util::adopt);

    // Todo(cpp20): constexpr this
    std::string prefixedActionName = G_ACTION_NAMESPACE;
    prefixedActionName += SELECTION_ACTION_NAME;

    // Todo(gtk4): remove radioGroup (see other comments)
    GtkWidget* radioGroup = nullptr;

    GtkWidget* grid = createPreviewGrid(types->getPageTypes(), prefixedActionName, radioGroup);

    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_popover_set_child(GTK_POPOVER(popover.get()), GTK_WIDGET(box));

    gtk_box_append(box, grid);
    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Create a special entry for copying the current page's background
    // It has index == npos
    gtk_box_append(box, createEntryWithoutPreview(_("Copy current"), npos, prefixedActionName, radioGroup));

    // The indices of special page types start after the normal page types'
    size_t n = types->getPageTypes().size();
    for (const auto& pageInfo: types->getSpecialPageTypes()) {
        gtk_box_append(box, createEntryWithoutPreview(pageInfo->name.c_str(), n++, prefixedActionName, radioGroup));
    }

    // Todo(gtk4): remove radioGroup (see other comments)
    radioButtonGroup.reset(radioGroup, xoj::util::ref);

    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    this->applyToCurrentPageButton.reset(gtk_button_new_with_label(_("Apply to current page")), xoj::util::adopt);
    // We cannot "Apply to current page" if no page type is selected...
    gtk_widget_set_sensitive(this->applyToCurrentPageButton.get(), this->selectedPT.has_value());
    g_signal_connect(this->applyToCurrentPageButton.get(), "clicked",
                     G_CALLBACK(+[](GtkWidget*, const PageTypeSelectionPopover* self) {
                         if (self->selectedPT) {
                             self->controller->changeCurrentPageBackground(self->selectedPT.value());
                         }
                     }),
                     this);
    gtk_box_append(box,
                   this->applyToCurrentPageButton.get());  // increases the ref-count of this->applyToCurrentPageButton

    GtkWidget* button = gtk_button_new_with_label(_("Apply to all pages"));
    g_signal_connect(button, "clicked", G_CALLBACK(+[](GtkWidget*, const PageTypeSelectionPopover* self) {
                         if (self->selectedPT) {
                             self->controller->applyBackgroundToAllPages(self->selectedPT.value());
                         } else {
                             self->controller->applyCurrentPageBackgroundToAll();
                         }
                     }),
                     this);
    gtk_box_append(box, button);  // box takes over button's floating ref

    gtk_widget_show_all(GTK_WIDGET(box));

    return popover;
}

void PageTypeSelectionPopover::setSelected(const std::optional<PageType>& selected) {
    // Todo(gtk4): remove this specialization entirely
    if (this->selectedPT != selected) {
        auto group = GSListView<GtkRadioButton>(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radioButtonGroup.get())));
        size_t index = findIndex(this->types, selected);

        for (auto& button: group) {
            GVariant* target = gtk_actionable_get_action_target_value(GTK_ACTIONABLE(&button));
            assert(g_variant_type_equal(g_variant_get_type(target), G_VARIANT_TYPE_UINT64));
            if (g_variant_get_uint64(target) == index) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(&button), true);
                break;
            }
        }
        this->selectedPT = selected;
    }
}

void PageTypeSelectionPopover::entrySelected(const PageTypeInfo*) {
    // We cannot "Apply to current page" if no page type is selected...
    gtk_widget_set_sensitive(this->applyToCurrentPageButton.get(), this->selectedPT.has_value());

    this->controller->setPageTypeForNewPages(this->selectedPT);
}
