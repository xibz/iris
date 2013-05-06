#include "About.h"
void showAbout(GtkWidget *w, gpointer window)
{
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "\n                          iRis\n\n"
            "                         2013\n"
            "                          1.1\n\n"
            "A chat and video messaging system.                \n\n"
            "                   Daniel Cauley\n"
            "                    Ben Powell\n"
            "                   Saju Varghese\n");
  gtk_window_set_title(GTK_WINDOW(dialog), "About");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}
