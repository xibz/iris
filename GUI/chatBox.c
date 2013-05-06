#include "chatBox.h"
#include <gtk-3.0/gtk/gtk.h>
void initChatbox(Chatbox *cbox)
{
  cbox->display = gtk_frame_new("Channel: 0");
  cbox->userBox = gtk_frame_new("User list:");
  gtk_frame_set_shadow_type(GTK_FRAME(cbox->display), GTK_SHADOW_IN);
  gtk_frame_set_shadow_type(GTK_FRAME(cbox->userBox), GTK_SHADOW_IN);
  cbox->table = gtk_table_new(2, 2, FALSE);
  cbox->text = gtk_text_view_new();
  gtk_text_view_set_wrap_mode((GtkTextView *)cbox->text, GTK_WRAP_WORD);
  gtk_text_view_set_editable((GtkTextView *)cbox->text, TRUE);
  gtk_table_attach_defaults(GTK_TABLE(cbox->table), cbox->display,
                            0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(cbox->table), cbox->text,
                            0, 1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(cbox->table), cbox->userBox,
                            1, 2, 0, 1);
  gtk_widget_set_size_request(cbox->display, 200, 400);
}
