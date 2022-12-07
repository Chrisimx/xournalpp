/*
 * Xournal++
 *
 * header for missing gtk4 functions (part of the gtk4 port)
 * will be removed later
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include "util/gtk4_helper.h"

#include <gtk/gtk.h>

namespace {
void set_child(GtkContainer* c, GtkWidget* child) {
    gtk_container_foreach(
            c, +[](GtkWidget* child, gpointer c) { gtk_container_remove(GTK_CONTAINER(c), child); }, c);
    gtk_container_add(c, child);
}
};  // namespace

/**** GtkBox ****/

void gtk_box_append(GtkBox* box, GtkWidget* child) {
    constexpr auto default_expand = false;
    gtk_box_pack_start(GTK_BOX(box), child, default_expand, true, 0);
}

void gtk_box_remove(GtkBox* box, GtkWidget* child) { gtk_container_remove(GTK_CONTAINER(box), child); }


/**** GtkCheckButton ****/

void gtk_check_button_set_child(GtkCheckButton* button, GtkWidget* child) { set_child(GTK_CONTAINER(button), child); }

void gtk_check_button_set_label(GtkCheckButton* button, const char* label) {
    gtk_check_button_set_child(GTK_CHECK_BUTTON(button), gtk_label_new(label));
}


/**** GtkButton ****/

void gtk_button_set_child(GtkButton* button, GtkWidget* child) { set_child(GTK_CONTAINER(button), child); }
GtkWidget* gtk_button_get_child(GtkButton* button) { return gtk_bin_get_child(GTK_BIN(button)); }

/**** GtkPopover ****/

GtkWidget* gtk_popover_new() { return gtk_popover_new(nullptr); }
void gtk_popover_set_child(GtkPopover* popover, GtkWidget* child) { set_child(GTK_CONTAINER(popover), child); }

/**** GtkLabel ****/
void gtk_label_set_wrap(GtkLabel* label, gboolean wrap) { gtk_label_set_line_wrap(label, wrap); }
