#ifndef __MENUBAR__H
#define __MENUBAR__H
#include <gtk-3.0/gtk/gtk.h>
typedef struct Menubar
{
  GtkWidget *menuBar;
  GtkWidget *fileMenu;
  GtkWidget *helpMenu;
  GtkWidget *file;
  GtkWidget *connect;
  GtkWidget *quit;
  GtkWidget *help;
  GtkWidget *about;
}Menubar;
void initMenubar(Menubar *);
#endif
