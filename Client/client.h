#ifndef __CLIENT__H
#define __CLIENT__H
#include <gtk-3.0/gtk/gtk.h>
#include "../GUI/chatBox.h"
#define NAMELEN 100
#define MAX_CHANNELS 10
#define MAX_USERS_PER_CHANNEL 10
#define MSGSIZE 1024
#define FILEBUF 900

struct chan_struct
{
  int inUse;
  char name[NAMELEN];
  char users[MAX_USERS_PER_CHANNEL][NAMELEN];
} chan[MAX_CHANNELS];
void parse_chan_user(char msg[MSGSIZE]);
void removeSubstring(char *, const char *);
void removeAllSubstring(char *, const char *);
int runClient(Chatbox *);
void parseInput(Chatbox *);
void readMsgs(Chatbox *);
void update_userlist(Chatbox *, struct chan_struct []);
#endif
