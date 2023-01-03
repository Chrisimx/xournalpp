#include "PageTypeSelectionPopover.h"

#include <cassert>
#include <memory>  // for unique_ptr
#include <string>  // for string

#include <glib.h>
#include <gtk/gtkactionable.h>

#include "control/PageBackgroundChangeController.h"
#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo
#include "control/settings/Settings.h"
#include "gui/dialog/FormatDialog.h"
#include "gui/menus/StaticAssertActionNamespace.h"
#include "model/PageType.h"  // for PageType
#include "util/GListView.h"
#include "util/StringUtils.h"
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

PageTypeSelectionPopover::PageTypeSelectionPopover(Control* control, Settings* settings, GtkApplicationWindow* win):
        PageTypeSelectionPopoverBase(control->getPageTypes(), settings, PAGETYPE_SELECTION_ACTION_NAME),
        control(control),
        controller(control->getPageBackgroundChangeController()),
        settings(settings),
        selectedPageSize(getInitiallySelectedPaperSize()),
        selectedOrientation(selectedPageSize->orientation()),
        popover(createPopover()) {
    static_assert(is_action_namespace_match<decltype(win)>(G_ACTION_NAMESPACE));

    this->controller->setPaperSizeForNewPages(this->selectedPageSize);

    orientationAction.reset(createOrientationGAction(selectedOrientation), xoj::util::adopt);
    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(orientationAction.get()));
    g_simple_action_set_enabled(orientationAction.get(), selectedPageSize.has_value());
    g_signal_connect(G_OBJECT(orientationAction.get()), "change-state", G_CALLBACK(changedOrientationSelectionCallback),
                     this);

    g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(typeSelectionAction.get()));
}
auto PageTypeSelectionPopover::getInitiallySelectedPaperSize() -> std::optional<PaperSize> {
    if (settings) {
        PageTemplateSettings model;
        model.parse(settings->getPageTemplate());
        return model.isCopyLastPageSize() ? std::nullopt : std::optional(PaperSize(model));
    }
    return std::nullopt;
}
auto PageTypeSelectionPopover::createOrientationGAction(uint8_t orientation) -> GSimpleAction* {
    return g_simple_action_new_stateful(ORIENTATION_SELECTION_ACTION_NAME, G_VARIANT_TYPE_BOOLEAN,
                                        g_variant_new_boolean(orientation));
}
void PageTypeSelectionPopover::changedOrientationSelectionCallback(GSimpleAction* ga, GVariant* parameter,
                                                                   PageTypeSelectionPopover* self) {
    g_simple_action_set_state(ga, parameter);
    self->selectedOrientation = static_cast<GtkOrientation>(g_variant_get_boolean(parameter));
    if (self->selectedPageSize && (self->selectedPageSize->orientation() != self->selectedOrientation)) {
        self->selectedPageSize->swapWidthHeight();
        self->controller->setPaperSizeForNewPages(self->selectedPageSize);
    }
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
template <bool changeComboBoxSelection>
void PageTypeSelectionPopover::setSelectedPaperSize(const std::optional<PaperSize>& newPageSize) {
    if (newPageSize != selectedPageSize) {
        if constexpr (changeComboBoxSelection)
            gtk_combo_box_set_active(GTK_COMBO_BOX(pageFormatComboBox.get()),
                                     getComboBoxIndexForPaperSize(newPageSize));

        selectedPageSize = newPageSize;
        if (selectedPageSize)
            g_simple_action_set_state(orientationAction.get(), g_variant_new_boolean(selectedPageSize->orientation()));
        g_simple_action_set_enabled(orientationAction.get(), selectedPageSize.has_value());

        controller->setPaperSizeForNewPages(selectedPageSize);
    }
}
int PageTypeSelectionPopover::getComboBoxIndexForPaperSize(const std::optional<PaperSize>& paperSize) {
    if (!paperSize)
        return paperSizeMenuOptions.size() - 1;  // Return index of copy option

    for (int i = 0; i < paperSizeMenuOptions.size() - 2; i++) {
        auto& currentPaperSize = std::get<PaperFormatUtils::GtkPaperSizeUniquePtr_t>(paperSizeMenuOptions[i]);
        const PaperSize aDefaultPaperSize(currentPaperSize);
        if (paperSize->equalDimensions(aDefaultPaperSize))
            return i;
    }
    return paperSizeMenuOptions.size() - 2;  // Custom option is returned if no matching format is found
}
GtkWidget* PageTypeSelectionPopover::createOrientationButton(std::string_view actionName, GtkOrientation orientation,
                                                             std::string_view icon) {
    GtkWidget* button = gtk_toggle_button_new();
    gtk_container_add(GTK_CONTAINER(button), gtk_image_new_from_icon_name(icon.data(), GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_actionable_set_action_name(GTK_ACTIONABLE(button), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(button), g_variant_new_boolean(orientation));
    return button;
}
xoj::util::WidgetSPtr PageTypeSelectionPopover::createPopover() {
    xoj::util::WidgetSPtr popover(gtk_popover_new(), xoj::util::adopt);

    // Todo(cpp20): constexpr this
    std::string prefixedPageTypeActionName = G_ACTION_NAMESPACE;
    prefixedPageTypeActionName += PAGETYPE_SELECTION_ACTION_NAME;
    std::string prefixedOrientationActionName = std::string(G_ACTION_NAMESPACE) + ORIENTATION_SELECTION_ACTION_NAME;

    // Todo(gtk4): remove radioGroup (see other comments)
    GtkWidget* radioGroup = nullptr;

    GtkWidget* grid = createPreviewGrid(types->getPageTypes(), prefixedPageTypeActionName, radioGroup);

    GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_popover_set_child(GTK_POPOVER(popover.get()), GTK_WIDGET(box));

    gtk_box_append(box, grid);

    GtkBox* pageFormatBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
    gtk_widget_set_margin_start(GTK_WIDGET(pageFormatBox), 20);
    gtk_widget_set_margin_end(GTK_WIDGET(pageFormatBox), 20);
    gtk_widget_set_margin_top(GTK_WIDGET(pageFormatBox), 20);
    gtk_widget_set_margin_bottom(GTK_WIDGET(pageFormatBox), 6);

    GtkBox* orientationFormatBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(orientationFormatBox));
    gtk_style_context_add_class(context, "linked");

    GtkWidget* verticalOrientationBtn = createOrientationButton(prefixedOrientationActionName, GTK_ORIENTATION_VERTICAL,
                                                                "xopp-orientation-portrait");
    GtkWidget* horizontalOrientationBtn = createOrientationButton(
            prefixedOrientationActionName, GTK_ORIENTATION_HORIZONTAL, "xopp-orientation-landscape");

    gtk_box_append(orientationFormatBox, verticalOrientationBtn);
    gtk_box_append(orientationFormatBox, horizontalOrientationBtn);

    gtk_box_append(pageFormatBox, GTK_WIDGET(orientationFormatBox));

    PaperFormatUtils::loadDefaultPaperSizes(paperSizeMenuOptions);
    // Add Special options
    paperSizeMenuOptions.emplace_back("Custom");
    paperSizeMenuOptions.emplace_back("Copy current page");
    pageFormatComboBox = PaperFormatUtils::createPaperFormatDropDown(paperSizeMenuOptions);
    gtk_combo_box_set_active(GTK_COMBO_BOX(pageFormatComboBox.get()), getComboBoxIndexForPaperSize(selectedPageSize));
    g_signal_connect(pageFormatComboBox.get(), "changed", G_CALLBACK(changedPaperFormatTemplateCb), this);
    gtk_box_append(pageFormatBox, pageFormatComboBox.get());

    gtk_box_append(box, GTK_WIDGET(pageFormatBox));

    gtk_box_append(box, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Create a special entry for copying the current page's background
    // It has index == npos
    gtk_box_append(box, createEntryWithoutPreview(_("Copy current"), npos, prefixedPageTypeActionName, radioGroup));

    // The indices of special page types start after the normal page types'
    size_t n = types->getPageTypes().size();
    for (const auto& pageInfo: types->getSpecialPageTypes()) {
        gtk_box_append(box,
                       createEntryWithoutPreview(pageInfo->name.c_str(), n++, prefixedPageTypeActionName, radioGroup));
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
void PageTypeSelectionPopover::changedPaperFormatTemplateCb(GtkComboBox* widget, PageTypeSelectionPopover* self) {
    int selected = gtk_combo_box_get_active(widget);
    if (selected < self->paperSizeMenuOptions.size() - 2) {
        GtkOrientation orientation = static_cast<GtkOrientation>(
                g_variant_get_boolean(g_action_get_state(G_ACTION(self->orientationAction.get()))));

        PaperSize paperSize(std::get<PaperFormatUtils::GtkPaperSizeUniquePtr_t>(self->paperSizeMenuOptions[selected]));
        if (paperSize.orientation() != orientation)
            paperSize.swapWidthHeight();

        self->setSelectedPaperSize<false>(paperSize);
    } else if (selected == self->paperSizeMenuOptions.size() - 2) {  // Custom option
        std::unique_ptr<FormatDialog> dlg;
        if (self->selectedPageSize) {
            dlg = std::make_unique<FormatDialog>(self->control->getGladeSearchPath(), self->settings,
                                                 self->selectedPageSize->width, self->selectedPageSize->height);
        } else {
            PageTemplateSettings model;
            model.parse(self->settings->getPageTemplate());
            dlg = std::make_unique<FormatDialog>(self->control->getGladeSearchPath(), self->settings,
                                                 model.getPageWidth(), model.getPageHeight());
        }
        dlg->show(self->control->getGtkWindow());

        self->setSelectedPaperSize<true>(PaperSize(dlg->getWidth(), dlg->getHeight()));
    } else if (selected == self->paperSizeMenuOptions.size() - 1) {  // Copy current option
        self->setSelectedPaperSize<false>(std::nullopt);
    }
}

void PageTypeSelectionPopover::setSelectedPageType(const std::optional<PageType>& selected) {
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
