#include "chatBox.h"
#include <gtk-3.0/gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "../Client/client.h"
#include <pthread.h>
#include <string.h>
void initChatbox(Chatbox *cbox)
{
  pthread_mutex_init(&cbox->m, NULL);
  cbox->scrollbox = gtk_scrolled_window_new(NULL, NULL);
  cbox->userBox = gtk_scrolled_window_new(NULL, NULL);
  cbox->display = gtk_scrolled_window_new(NULL, NULL);
  //gtk_frame_set_shadow_type(GTK_FRAME(cbox->display), GTK_SHADOW_IN);
  //gtk_frame_set_shadow_type(GTK_FRAME(cbox->userBox), GTK_SHADOW_IN);
  cbox->table = gtk_table_new(2, 2, FALSE);
  gtk_table_set_row_spacing(GTK_TABLE(cbox->table), 0, 5);
  gtk_table_set_col_spacing(GTK_TABLE(cbox->table), 0, 5);

  cbox->text = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cbox->text), GTK_WRAP_CHAR);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(cbox->text), TRUE);

  cbox->userlist = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cbox->userlist), GTK_WRAP_CHAR);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(cbox->userlist), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(cbox->userlist), FALSE);

  cbox->dispText = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cbox->dispText), GTK_WRAP_CHAR);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(cbox->dispText), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(cbox->dispText), FALSE);
  gtk_container_add(GTK_CONTAINER(cbox->display), cbox->dispText);
  gtk_container_add(GTK_CONTAINER(cbox->scrollbox), cbox->text);
  gtk_container_add(GTK_CONTAINER(cbox->userBox), cbox->userlist);

  gtk_table_attach_defaults(GTK_TABLE(cbox->table), cbox->display,
                            0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(cbox->table), cbox->scrollbox,
                            0, 1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(cbox->table), cbox->userBox,
                            1, 2, 0, 2);
  gtk_widget_set_size_request(cbox->display, 200, 400);
  gtk_widget_set_hexpand(cbox->display, TRUE);
  gtk_widget_set_hexpand(cbox->text, TRUE);

  GtkTextBuffer *b = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cbox->userlist));
  gtk_text_buffer_set_text(b, " Userlist:\n", strlen(" Userlist:\n"));

  g_signal_connect(cbox->text, "key_press_event", G_CALLBACK(on_key_press), cbox);
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *pKey, Chatbox *cbox)
{
  if(pKey->type == GDK_KEY_PRESS)
  {
    pthread_t thread;
    switch(pKey->keyval)
    {
      case 65293:
        printf("ENTER\n");
        cbox->sockfd = 2556;
        parseInput(cbox);
        {
          GtkTextBuffer *b = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cbox->text));
          char buffer[2] = "";
          gtk_text_buffer_set_text(b, buffer, strlen(buffer));
        }
        return TRUE;
        break;
    }
  }
  return FALSE;
}
