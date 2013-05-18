#include "connectWindow.h"
#include "../Client/client.h"
#include <gtk-3.0/gtk/gtk.h>
#define SERVERMAX 256
#define PORTMAX 10
void newConnection(GtkWidget *pWindow)
{
  /////MEHHHHH, use a table///////
  GtkWidget *popupWindow, *connect, *grid, *server, *port;
  server = gtk_text_view_new();
  popupWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  connect = gtk_button_new_with_label("Connect");
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(popupWindow), grid);
  gtk_grid_attach(GTK_GRID(grid), connect, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), server, 0, 1, 1, 2);
  gtk_window_set_title(GTK_WINDOW(popupWindow), "Connect to Server");
  gtk_container_set_border_width(GTK_CONTAINER(popupWindow), 10);
  gtk_window_set_resizable(GTK_WINDOW(popupWindow), FALSE);
  gtk_window_set_decorated(GTK_WINDOW(popupWindow), FALSE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(popupWindow), TRUE);
  gtk_window_set_skip_pager_hint(GTK_WINDOW(popupWindow), TRUE);
  gtk_widget_set_size_request(popupWindow, 400, 400);
  gtk_window_set_position(GTK_WINDOW(popupWindow), GTK_WIN_POS_CENTER);
  gtk_widget_set_events(popupWindow, GDK_FOCUS_CHANGE_MASK);
  g_signal_connect(G_OBJECT(popupWindow), "focus-out-event",
                    G_CALLBACK(on_popup_focus_out),
                    NULL);
  //g_signal_connect(connect, "clicked", G_CALLBACK(connectServer), NULL);
  GdkColor color;
  gdk_color_parse("#0066FF", &color);
  gtk_widget_modify_bg(GTK_WIDGET(popupWindow), GTK_STATE_NORMAL, &color);
  gtk_widget_show_all(popupWindow);
  gtk_widget_grab_focus(popupWindow);
}
gboolean on_popup_focus_out(GtkWidget *w, GdkEventFocus *event, gpointer data)
{
  gtk_widget_destroy(w);
  return TRUE;
}
