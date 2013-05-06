#ifndef __CHATBOX__H
#define __CHATBOX__H
#include <gtk-3.0/gtk/gtk.h>
typedef struct Chatbox
{
  GtkWidget *table;
  GtkWidget *text;
  GtkWidget *display;
}Chatbox;
void initChatbox(Chatbox *);
#endif
