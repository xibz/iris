#include <stdio.h>
#include <stdbool.h>
#include <gtk-3.0/gtk/gtk.h>
#include "Menubar.h"
#include "chatBox.h"
bool buttonPress(GtkWidget *, GdkEvent *);
void menuItemResponse(char*);

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *vbox;
  Menubar mBar;
  Chatbox cbox;
  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 450);
  gtk_window_set_title(GTK_WINDOW(window), "iRis");
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  initMenubar(&mBar);
  initChatbox(&cbox);
  gtk_box_pack_start(GTK_BOX(vbox), mBar.menuBar, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(vbox), cbox.table, TRUE, TRUE, 0);
  g_signal_connect_swapped(G_OBJECT(window), "destroy",
  G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(G_OBJECT(mBar.quit), "activate",
  G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_show_all(window);
  gtk_main();
  return 0;
}
