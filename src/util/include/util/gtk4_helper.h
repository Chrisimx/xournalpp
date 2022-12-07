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

#pragma once

#include <gtk/gtk.h>

/**** GtkBox ****/

void gtk_box_append(GtkBox* box, GtkWidget* child);
void gtk_box_remove(GtkBox* box, GtkWidget* child);


/**** GtkCheckButton ****/

void gtk_check_button_set_child(GtkCheckButton* button, GtkWidget* child);
void gtk_check_button_set_label(GtkCheckButton* button, const char* label);


/**** GtkButton ****/

void gtk_button_set_child(GtkButton* button, GtkWidget* child);
GtkWidget* gtk_button_get_child(GtkButton* button);


/**** GtkPopover ****/

GtkWidget* gtk_popover_new();
void gtk_popover_set_child(GtkPopover* popover, GtkWidget* child);


/**** GtkLabel ****/
void gtk_label_set_wrap(GtkLabel* label, gboolean wrap);
