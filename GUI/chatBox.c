#include "chatBox.h"
#include <gtk-3.0/gtk/gtk.h>
void initChatbox(Chatbox *cbox)
{
  cbox->table = gtk_table_new(1, 1, TRUE);
  cbox->text = gtk_text_view_new();
  gtk_text_view_set_wrap_mode((GtkTextView *)cbox->text, GTK_WRAP_WORD);
  gtk_text_view_set_editable((GtkTextView *)cbox->text, TRUE);
  gtk_table_attach_defaults(GTK_TABLE(cbox->table), cbox->text,
                            0, 1, 0, 1);
}
