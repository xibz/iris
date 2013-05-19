#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <gtk-3.0/gtk/gtk.h>
#include "client.h"

#define NAMELEN 100
#define MAX_CHANNELS 10
#define MAX_USERS_PER_CHANNEL 10
#define MSGSIZE 1024
#define FILEBUF 900

#define NIPQUAD(addr) ((unsigned char *)&addr)[0], ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[2], ((unsigned char *)&addr)[3]

void readMsgs(Chatbox *cbox)
{
  char buffer[2048]="";
  GtkTextIter start, end;
  GtkTextBuffer *b = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cbox->dispText));
  GtkTextBuffer *textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cbox->text));
  gtk_text_buffer_get_bounds(textBuffer, &start, &end);
  int sockfd = cbox->sockfd;
  char msg[MSGSIZE];
  while ( 1 )
  {
      pthread_mutex_lock(&cbox->m);
      memset(msg, '\0', MSGSIZE);
      read(sockfd, msg, MSGSIZE);
      if(strlen(msg))
      {
        printf("msg_here:%s\n", msg);
        if ( !strncmp(msg, "QUITUSER", 8) )
        {
            printf("%s\n", msg);
            break;
        }
        if ( !strncmp(msg, "FILE", 4) )
        { // Sending files
            FILE *fp;
            char ext[5], fname[50];
            removeSubstring(msg, "FILE ");
            strcpy(ext, strtok(msg, " "));
            strcpy(fname, strtok(NULL, " "));
            int total_bytes = atoi(strtok(NULL, " \n"));
            sprintf(buffer, "Total bytes expecting: %5d\n", total_bytes);
            gtk_text_buffer_get_end_iter(b, &end);
            gtk_text_buffer_insert(b, &end, buffer, -1); 
            printf("Total bytes expecting: %5d\n", total_bytes);
            if ( !access(fname, F_OK) )
            {
                removeSubstring(fname, ".");
                removeSubstring(fname, ext);
                sprintf(fname, "%s_tmp.%s", fname, ext);
                if( remove(fname) != 0 ) perror("Error deleting file");
                else
                {
                  strcpy(buffer, "File successfully deleted\n");
                  gtk_text_buffer_get_end_iter(b, &end);
                  gtk_text_buffer_insert(b, &end, buffer, -1); 
                  puts("File successfully deleted");
                }
                if ( !(fp = fopen(fname, "wb")) )
                {
                  sprintf(buffer, "Error creating the file: %s\n", fname);
                  gtk_text_buffer_get_end_iter(b, &end);
                  gtk_text_buffer_insert(b, &end, buffer, -1); 
                  printf("Error creating the file: %s\n", fname);
                }
            } else {
                if ( !(fp = fopen(fname, "wb")) )
                {
                  sprintf(buffer, "Error creating the file: %s\n", fname);
                  gtk_text_buffer_get_end_iter(b, &end);
                  gtk_text_buffer_insert(b, &end, buffer, -1); 
                  printf("Error creating the file: %s\n", fname);
                }
            }
            if ( fp )
            {
                int nbr=0, total_bytes_r=0, total_bytes_s=0, i;
                while ( total_bytes_s < total_bytes )
                {
                    memset(msg, '\0', MSGSIZE);
                    nbr = read(sockfd, msg, MSGSIZE) - 5;
                    if ( nbr > 0 )
                    {
                        removeSubstring(msg, "FILE ");
                        sprintf(buffer, "MSGRECV:%d:%s\n", nbr, msg);
                        gtk_text_buffer_get_end_iter(b, &end);
                        gtk_text_buffer_insert(b, &end, buffer, -1); 
                        printf("MSGRECV:%d:%s\n", nbr, msg);
                        total_bytes_r = total_bytes_r + nbr;
                        for ( i=0; i<nbr; ++i ) total_bytes_s = total_bytes_s + fprintf(fp, "%c", msg[i]);
                        if ( !(total_bytes_s*100/total_bytes % 10) )
                        {
                          sprintf(buffer, "%f...", total_bytes_s*100./total_bytes);
                          gtk_text_buffer_get_end_iter(b, &end);
                          gtk_text_buffer_insert(b, &end, buffer, -1); 
                          printf("%f...", total_bytes_s*100./total_bytes);
                        }
                    } else {
                        break;
                    }
                }
                fclose(fp);
                sprintf(buffer, "\nTotal bytes recv: %4d | written: %4d\n", total_bytes_r, total_bytes_s);
                gtk_text_buffer_get_end_iter(b, &end);
                gtk_text_buffer_insert(b, &end, buffer, -1); 
                printf("\nTotal bytes recv: %4d | written: %4d\n", total_bytes_r, total_bytes_s);
            }
        } else if ( !strncmp(msg, "VID", 3) ) {
            // Handle video feed here....
        } else { // all the rest are messages //2 - OUPUT BUFFER
            printf("msg recv:%s", msg);
            if ( !strncmp(msg, "ADDUSER", 7) || !strncmp(msg, "JOINCHANNEL", 11) || !strncmp(msg, "LEAVECHANNEL", 12) || !strncmp(msg, "CREATECHANNEL", 13) ) msg[strchr(msg, '\n') - msg] = ' ';
            if ( !strncmp(msg, "USER JOINED", 11) || !strncmp(msg, "USER LEFT", 9) ) 
            {
                msg[strchr(msg, '\n') - msg] = ' ';
                msg[strchr(msg, '\n') - msg] = ' ';
            }
            parse_chan_user(msg);
            update_userlist(cbox, chan);
            strcpy(buffer, msg);
            gtk_text_buffer_get_end_iter(b, &end);
            gtk_text_buffer_insert(b, &end, buffer, -1); 
     	 }
    }
    pthread_mutex_unlock(&cbox->m);
  }
}

void update_userlist(Chatbox *cbox, struct chan_struct chan[])
{
  GtkTextIter start, end;
  GtkTextBuffer *b = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cbox->userlist));
  gtk_text_buffer_set_text(b, " Userlist:\n", strlen(" Userlist:\n"));
  int i, j;
  for(i = 0; i < MAX_CHANNELS; ++i)
  {
    for(j = 0; j < MAX_USERS_PER_CHANNEL; ++j)
    {
      gtk_text_buffer_get_end_iter(b, &end);
      char userName[NAMELEN+3];
      strcpy(userName, chan[i].users[j]);
      strcat(userName, "\n");
      gtk_text_buffer_insert(b, &end, userName, -1); 
    }
  }
}

void parseInput(Chatbox *cbox)
{
  pthread_mutex_lock(&cbox->m);
  char buffer[2048]="\n";
  int sockfd = cbox->sockfd;
  char chan_tmp[NAMELEN];
  GtkTextIter start, end;
  GtkTextBuffer *b = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cbox->dispText));
  GtkTextBuffer *textBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cbox->text));
  gtk_text_buffer_get_bounds(textBuffer, &start, &end);
  char *msg = gtk_text_buffer_get_text(textBuffer, &start, &end, FALSE);
  gtk_text_buffer_get_end_iter(b, &end);
  strcat(buffer, msg);
  gtk_text_buffer_insert(b, &end, buffer, 0); 
  //gtk_text_buffer_set_text(b, msg, strlen(msg));
  if ( !strncmp(msg, "EXIT", 4) ) 
  {
      write(sockfd, msg, strlen(msg));
  } 
  else 
  {
      char tmp[MSGSIZE], *cmd;
      memset(tmp, '\0', MSGSIZE);
      strcpy(tmp, msg);
      cmd = strtok(tmp, " ");
      printf("val:%d\n", !strncmp(msg, "FILE", 4));
      if ( !strncmp(msg, "ADDUSER", 7) )
      {
          if ( strtok(NULL, " \n") )
          {// all good
              msg[7] = '\n';
              sprintf(buffer, "\n*%s*\n", msg);
              gtk_text_buffer_get_end_iter(b, &end);
              gtk_text_buffer_insert(b, &end, buffer, -1); 
              printf("*%s*\n", msg);
              write(sockfd, msg, strlen(msg));
          } else {
              strcpy(buffer, "\nA username was not passed in this message\n");
              gtk_text_buffer_get_end_iter(b, &end);
              gtk_text_buffer_insert(b, &end, buffer, -1); 
          }
      } else if ( !strncmp(msg, "JOINCHANNEL", 11) || !strncmp(msg, "LEAVECHANNEL", 12) || !strncmp(msg, "CREATECHANNEL", 13) ) {
          strcpy(chan_tmp, strtok(NULL, " \n"));
          if ( chan_tmp )
          {// all good
              msg[strlen(cmd)] = '\n';
              write(sockfd, msg, strlen(msg));
          } else {
              strcpy(buffer, "\nNo channel was not passed in this message.\n");
              gtk_text_buffer_get_end_iter(b, &end);
              gtk_text_buffer_insert(b, &end, buffer, -1); 
              printf("No channel was not passed in this message.\n");
          }
      } else if ( !strncmp(msg, "BROADCAST", 9) ) {
          if ( strtok(NULL, " ") )
          {
              if ( strtok(NULL, " \n") )
              {// all good
                  msg[9] = '\n';
                  msg[strchr(msg, ' ') - msg] = '\n';
                  strcpy(buffer, msg);
                  gtk_text_buffer_get_end_iter(b, &end);
                  gtk_text_buffer_insert(b, &end, buffer, -1); 
                  printf("*%s*\n", msg);
                  write(sockfd, msg, strlen(msg));
              } else {
                  strcpy(buffer, "No message was not passed in this message.\n");
                  gtk_text_buffer_get_end_iter(b, &end);
                  gtk_text_buffer_insert(b, &end, buffer, -1); 
                  printf("No message was not passed in this message.\n");
              }
          } else {
              strcpy(buffer, "\nNo channel was not passed in this message.\n");
              gtk_text_buffer_get_end_iter(b, &end);
              gtk_text_buffer_insert(b, &end, buffer, -1); 
              printf("No channel was not passed in this message.\n");
          }
      } else if ( !strncmp(msg, "QUITUSER", 8) ) {
          write(sockfd, msg, strlen(msg));
      } else if ( !strncmp(msg, "FILE", 4) ) {
        FILE *fp;
        struct stat file_stat;
        char ext[5], fname[50], tmp[MSGSIZE], file_content[FILEBUF], send_msg[MSGSIZE];
        memset(tmp, '\0', MSGSIZE);
        strcpy(tmp, msg);
        removeSubstring(tmp, "FILE");
        strcpy(ext, strtok(tmp, " "));
        strcpy(fname, strtok(NULL, " \n"));
        fname[strlen(fname)]='\0';
        if ( !access(fname, F_OK) )
        {
            if ( !(fp = fopen(fname, "rb")) )
            {
              memset(buffer, '\0', 2048);
              sprintf(buffer, "Error reading the file: %s\n", fname);
              gtk_text_buffer_get_end_iter(b, &end);
              gtk_text_buffer_insert(b, &end, buffer, -1); 
            }
            else {
                stat(fname, &file_stat);
                printf("Total file size in bytes: %5lld\n", file_stat.st_size);
                strcpy(msg, strtok(msg, "\n"));
                sprintf(msg, "%s %lld", msg, file_stat.st_size);
                memset(send_msg, '\0', MSGSIZE);
                sprintf(send_msg, "BROADCAST\n%s\n%s", chan, msg);
                printf("OUT:%s\n", send_msg);
                write(sockfd, send_msg, strlen(send_msg));
                memset(msg, '\0', MSGSIZE);
                memset(file_content, '\0', FILEBUF);
                int nbr=0, total_bytes_r=0, total_bytes_s=0;
                while ( total_bytes_r < file_stat.st_size )
                {
                    memset(file_content, '\0', FILEBUF);
                    memset(send_msg, '\0', MSGSIZE);
                    printf("PREIN:%s\n", file_content);
                    nbr = fread(file_content, 1, FILEBUF, fp);
                    printf("Readin:%d\n", nbr);
                    sprintf(send_msg, "BROADCAST\n%s\nFILE %s", chan, file_content);
                    memset(tmp, '\0', MSGSIZE);
                    sprintf(tmp, "BROADCAST\n%s\nFILE ", chan);
                    send_msg[nbr+strlen(tmp)] = '\0';
                    printf("IN:%s\n", file_content);
                    printf("IN2:%s\n", send_msg);
                    total_bytes_r = total_bytes_r + nbr;
                    total_bytes_s = total_bytes_s + nbr;
                    printf("*\n*%ld*\n*", write(sockfd, send_msg, strlen(send_msg)));//send(sockfd, send_msg, strlen(send_msg), MSG_DONTROUTE);
                    if ( !(total_bytes_s*100/file_stat.st_size % 10) ) printf("%f...", total_bytes_s*100./file_stat.st_size);
                    sleep(1);
                }
                fclose(fp);
                printf("\nTotal bytes read: %4d | sent %4d\n", total_bytes_r, total_bytes_s);
            }
        }
    }
  }
  pthread_mutex_unlock(&cbox->m);
}

int runClient(Chatbox *cbox)
{
    signal(SIGCHLD, SIG_IGN);
    int sockfd, PORT;
    char ADDR[256] = "127.0.0.1", msg[MSGSIZE], str[INET_ADDRSTRLEN];
    struct sockaddr_in address;

    int i, j;
    for ( i=0; i<MAX_CHANNELS; ++i )
    {
      chan[i].inUse = 0;
      for ( j=0; j<MAX_USERS_PER_CHANNEL; ++j ) memset(chan[i].users[j], '\0', NAMELEN);
    }

    PORT = 7000;
    char buffer[2048] = "Connection established with server: ";
 
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) perror("Socket Error");
    else {
        address.sin_family      = AF_INET;
        address.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)gethostbyname(ADDR)->h_addr_list[0])));
        address.sin_port        = htons(PORT);

        if ( connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0 ) perror("Connection Error");
        else {
            cbox->sockfd = sockfd;
            inet_ntop(AF_INET, &(address.sin_addr), str, INET_ADDRSTRLEN);
            GtkTextBuffer *b = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cbox->dispText));
            strcat(buffer, str);
            gtk_text_buffer_set_text(b, buffer, strlen(buffer));
            close(sockfd); 
        }
    }
    return 1;
}

void parse_chan_user(char msg[MSGSIZE])
{
  char tmp[MSGSIZE], s[MSGSIZE];
  int i, j, k;
  strcpy(tmp, msg);
  if ( !strncmp(msg, "CREATECHANNEL", 13) )
  {
    strtok(tmp, " ");
    strtok(NULL, " ");
    strcpy(s, strtok(NULL, " "));
    if ( !strcmp(s, "SUCCESS") )
    {
      for ( i=0; i<MAX_CHANNELS; ++i )
      {
        if ( !chan[i].inUse )
        {
          chan[i].inUse = 1;
          strcpy(chan[i].name, strtok(NULL, " \n"));
          break;
        }
      }
    }    
  } else if ( !strncmp(msg, "JOINCHANNEL", 11) ) {
    strtok(tmp, " ");
    strtok(NULL, " ");
    strcpy(s, strtok(NULL, " "));
    if ( !strcmp(s, "SUCCESS") )
    {
      memset(s, '\0', MSGSIZE);
      strcpy(s, strtok(NULL, " "));
      for ( i=0; i<MAX_CHANNELS; ++i )
      {
        if ( !chan[i].inUse )
        {
          strcpy(chan[i].name, s);
          for ( j=0; j<MAX_USERS_PER_CHANNEL; ++j )
          {
            if ( !strlen(chan[i].users[j]) )
            {
              int nusers = atoi(strtok(NULL, " "));
              for ( k=0; k<nusers; ++k, ++j ) strcpy(chan[i].users[j], strtok(NULL, " \n"));
              break;
            }
          }
          break;
        }
      }
    }
  } else if ( !strncmp(msg, "USER JOINED", 11) ) {
    strtok(tmp, " ");
    strtok(NULL, " ");
    strcpy(s, strtok(NULL, " "));
    for ( i=0; i<MAX_CHANNELS; ++i )
    {
      if ( !strcmp(chan[i].name, s) )
      {
        for ( j=0; j<MAX_USERS_PER_CHANNEL; ++j )
        {
          if ( !strlen(chan[i].users[j]) )
          {
            strcpy(chan[i].users[j], strtok(NULL, " \n"));
            break;
          }
        }
        break;
      }
    }
  } else if ( !strncmp(msg, "LEAVECHANNEL", 12) ) {
    strtok(tmp, " ");
    strtok(NULL, " ");
    strcpy(s, strtok(NULL, " "));
    if ( !strcmp(s, "SUCCESS") )
    {
        memset(s, '\0', MSGSIZE);
        strcpy(s, strtok(NULL, " \n"));
      for ( i=0; i<MAX_CHANNELS; ++i )
      {
        if ( !strcmp(chan[i].name, s) )
        {
          for ( j=0; j<MAX_USERS_PER_CHANNEL; ++j ) if ( strlen(chan[i].users[j]) ) memset(chan[i].users[j], '\0', NAMELEN);
          chan[i].inUse = 0;
          break;
        }
      }
    }
  } else if ( !strncmp(msg, "USER LEFT", 9) ) {
    strtok(tmp, " ");
    strtok(NULL, " ");
    strcpy(s, strtok(NULL, " "));
    for ( i=0; i<MAX_CHANNELS; ++i )
    {
      if ( !strcmp(chan[i].name, s) )
      {
        memset(s, '\0', MSGSIZE);
        strcpy(s, strtok(NULL, " "));
        for ( j=0; j<MAX_USERS_PER_CHANNEL; ++j )
        {
          if ( !strcmp(chan[i].users[j], s) )
          {
            memset(chan[i].users[j], '\0', NAMELEN);
            break;
          }
        }
        break;
      }
    }
  }
}

void removeAllSubstring(char *s, const char *toremove)
{ // remove substring from the string
    while( (s = strstr(s, toremove)) ) memmove(s, (s + strlen(toremove)), (1 + strlen(s + strlen(toremove))));
}

void removeSubstring(char *s, const char *toremove)
{ // remove substring from the string
    if ( (s = strstr(s, toremove)) ) memmove(s, (s + strlen(toremove)), (1 + strlen(s + strlen(toremove))));
}
