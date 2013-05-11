#ifndef __CONNECTWINDOW__H
#define __CONECTWINDOW__H
#include <gtk-3.0/gtk/gtk.h>
void newConnection(GtkWidget *pWindow);
gboolean on_popup_focus_out(GtkWidget *, GdkEventFocus *, gpointer);
#endif
