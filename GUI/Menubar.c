#include "Menubar.h"
#include <gtk-3.0/gtk/gtk.h>
void initMenubar(Menubar *mbar)
{
  mbar->menuBar = gtk_menu_bar_new();
  mbar->fileMenu = gtk_menu_new();
  mbar->helpMenu = gtk_menu_new();
  mbar->file = gtk_menu_item_new_with_label("File");
  mbar->quit = gtk_menu_item_new_with_label("Quit");
  mbar->help = gtk_menu_item_new_with_label("Help");
  mbar->about = gtk_menu_item_new_with_label("About");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mbar->file), mbar->fileMenu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mbar->help), mbar->helpMenu);
  gtk_menu_shell_append(GTK_MENU_SHELL(mbar->fileMenu), mbar->quit);
  gtk_menu_shell_append(GTK_MENU_SHELL(mbar->helpMenu), mbar->about);
  gtk_menu_shell_append(GTK_MENU_SHELL(mbar->menuBar), mbar->file);
  gtk_menu_shell_append(GTK_MENU_SHELL(mbar->menuBar), mbar->help);
}
