#include <stdio.h>
#include <stdbool.h>
#include <gtk-3.0/gtk/gtk.h>
#include "Menubar.h"
#include "chatBox.h"
#include "About.h"
#include "../Client/client.h"
#include "connectWindow.h"
bool buttonPress(GtkWidget *, GdkEvent *);
void menuItemResponse(char*);

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *grid;
  Menubar mBar;
  Chatbox cbox;
  g_thread_init(NULL);
  gdk_threads_init();
  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 650);
  gtk_widget_set_size_request(window, 600, 650);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  gtk_window_set_title(GTK_WINDOW(window), "iRis");
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  initMenubar(&mBar);
  initChatbox(&cbox);

  gtk_box_pack_start(GTK_BOX(vbox), mBar.menuBar, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(vbox), cbox.table, TRUE, TRUE, 0);

  g_signal_connect(G_OBJECT(mBar.connect), "activate",
    G_CALLBACK(newConnection), (gpointer)window);
  g_signal_connect_swapped(G_OBJECT(window), "destroy",
  G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(G_OBJECT(mBar.quit), "activate",
  G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(G_OBJECT(mBar.about), "activate",
    G_CALLBACK(showAbout), (gpointer)window);

  gtk_widget_show_all(window);
  pthread_t thread[2];
  //thread client
  //Probably should call runClient on connect

  //pthread_create(&thread[0], NULL, (void *)&runClient, (void *)&cbox);
  runClient(&cbox);
  gdk_threads_enter();
  pthread_create(&thread[1], NULL, (void *)&readMsgs, (void *)&cbox);
  gdk_threads_leave();

  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave();
  //Meh, could have made this exit nicer
  exit(0);
  return 0;
}
