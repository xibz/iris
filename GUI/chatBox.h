#ifndef __CHATBOX__H
#define __CHATBOX__H
#include <gtk-3.0/gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pthread.h>
typedef struct Chatbox
{
  GtkWidget *table;
  GtkWidget *text;
  GtkWidget *dispText;
  GtkWidget *display;
  GtkWidget *userBox;
  GtkWidget *scrollbox;
  GtkWidget *displayBox;
  GtkWidget *userlist;
  int sockfd;
  pthread_mutex_t m;
}Chatbox;
void initChatbox(Chatbox *);
gboolean on_key_press(GtkWidget *, GdkEventKey *, Chatbox *);
#endif
