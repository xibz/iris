#ifndef __CLIENT__H
#define __CLIENT__H
#include <gtk-3.0/gtk/gtk.h>
#include "../GUI/chatBox.h"
void removeSubstring(char *, const char *);
void removeAllSubstring(char *, const char *);
int runClient(Chatbox *);
void parseInput(Chatbox *);
void readMsgs(Chatbox *);
#endif
