#include "PageTypeSelectionPopoverBase.h"

#include <memory>  // for unique_ptr
#include <string>  // for string

#include "control/pagetype/PageTypeHandler.h"  // for PageTypeInfo
#include "util/Color.h"                        // for Color
#include "util/raii/CairoWrappers.h"
#include "view/background/BackgroundView.h"  // for BackgroundView

PageTypeSelectionPopoverBase::PageTypeSelectionPopoverBase(PageTypeHandler* typesHandler, const Settings* settings,
                                                           const std::string_view& actionName):
        PageTypeSelectionMenuBase(typesHandler, settings, actionName) {}

auto PageTypeSelectionPopoverBase::createPreviewImage(const PageTypeInfo& pti) -> GtkWidget* {
    const int previewWidth = 100;
    const int previewHeight = 141;
    const double zoom = 0.5;

    xoj::util::CairoSurfaceSPtr surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, previewWidth, previewHeight),
                                        xoj::util::adopt);
    xoj::util::CairoSPtr crSPtr(cairo_create(surface.get()), xoj::util::adopt);
    auto cr = crSPtr.get();

    cairo_scale(cr, zoom, zoom);

    auto bgView = xoj::view::BackgroundView::createRuled(previewWidth / zoom, previewHeight / zoom, Colors::white,
                                                         pti.page, 2.0);
    bgView->draw(cr);

    cairo_identity_matrix(cr);

    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, previewWidth, 0);
    cairo_line_to(cr, previewWidth, previewHeight);
    cairo_line_to(cr, 0, previewHeight);
    cairo_line_to(cr, 0, 0);
    cairo_stroke(cr);


    // Todo(gtk4) Figure out how to get around without gtk_image_new_from_surface
    GtkWidget* preview = gtk_image_new_from_surface(surface.get());
    gtk_widget_set_tooltip_text(preview, pti.name.c_str());
    return preview;
}
