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
#include "color_print.h"
#include <math.h>

#define MSGSIZE 2048

#define NIPQUAD(addr) ((unsigned char *)&addr)[0], ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[2], ((unsigned char *)&addr)[3]

void removeSubstring(char *, const char *);
void removeAllSubstring(char *, const char *);

int main(int argc, char *argv[])
{
    int sockfd, PORT;
    char *ADDR, msg[MSGSIZE], str[INET_ADDRSTRLEN];
    struct sockaddr_in address;

    ADDR = argv[1];
    PORT = atoi(argv[2]);
 
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) perror(SET_RED("Socket Error"));
    else {
        address.sin_family      = AF_INET;
        address.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)gethostbyname(ADDR)->h_addr_list[0])));
        address.sin_port        = htons(PORT);

        if ( connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0 ) perror(SET_RED("Connection Error"));
        else {
            inet_ntop(AF_INET, &(address.sin_addr), str, INET_ADDRSTRLEN);
            printf(SET_GREEN("Connection established with Server: %s\n"), str);
            bzero(msg, MSGSIZE);
            sprintf(msg, "CLIENT");
            write(sockfd, msg, strlen(msg));
            if ( !fork() )
            {   // writing to the server
                while ( 1 )
                {
                    bzero(msg, MSGSIZE);
                    if ( fgets(msg, MSGSIZE, stdin) )
                    {
                        printf(SET_CYAN("Input:%s"), msg);
                        if ( strcasestr(msg, "FILE") )
                        {
                            FILE *fp;
                            struct stat file_stat;
                            char ext[5], fname[50], tmp[MSGSIZE], file_content[MSGSIZE];
                            strcpy(tmp, msg);
                            removeSubstring(tmp, "FILE");
                            removeSubstring(tmp, "file");
                            strcpy(ext, strtok(tmp, " "));
                            strcpy(fname, strtok(NULL, " \n"));
                            fname[strlen(fname)]='\0';
                            if ( !access(fname, F_OK) )
                            {
                                if ( !(fp = fopen(fname, "r")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                                else {
                                    stat(fname, &file_stat);
                                    int block_send = (int)ceil(file_stat.st_size/1024.0);
                                    strcpy(msg, strtok(msg, "\n"));
                                    sprintf(msg, "%s %d", msg, block_send);
                                    write(sockfd, msg, strlen(msg));
                                    bzero(file_content, MSGSIZE);
                                    while ( block_send )
                                    {
                                        fread(file_content, 1, 1024, fp);
                                        send(sockfd, file_content, strlen(file_content), 0); //write(sockfd, file_content, strlen(file_content));
                                        bzero(file_content, MSGSIZE);
                                        block_send = block_send - 1;
                                    }
                                    fclose(fp);
                                }
                            }
                        } else write(sockfd, msg, strlen(msg));
                    }
                    if ( strcasestr(msg, "EXIT") ) break; else bzero(msg, MSGSIZE);
                }
            } else {
                // reading from the server
                char msg[MSGSIZE];
                while ( 1 )
                {
                    bzero(msg, MSGSIZE);
                    read(sockfd, msg, MSGSIZE);                    
                    if ( strcasestr(msg, "FILE") )
                    { // Sending files of size < 1MB & 40 characters or less in filename
                        FILE *fp;
                        char ext[5], fname[50];
                        removeSubstring(msg, "FILE ");
                        removeSubstring(msg, "file ");
                        strcpy(ext, strtok(msg, " "));
                        strcpy(fname, strtok(NULL, " "));
                        int blocks = atoi(strtok(NULL, " \n"));
                        if ( !access(fname, F_OK) )
                        {
                            removeSubstring(fname, ".");
                            removeSubstring(fname, ext);
                            sprintf(fname, "%s_tmp.%s", fname, ext);
                            if ( !(fp = fopen(fname, "w")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                        } else {
                            if ( !(fp = fopen(fname, "w")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                        }
                        if ( fp )
                        {
                            while ( blocks )
                            {
                                bzero(msg, MSGSIZE);
                                recv(sockfd, msg, MSGSIZE, 0); //read(sockfd, msg, MSGSIZE);
                                fprintf(fp, "%s", msg);
                                blocks = blocks - 1;
                            }
                            fclose(fp);
                        }
                    } else if ( !strcasecmp(msg, "VID") ) {
                        // Handle video feed here....
                    } else { // all the rest are messages
                        printf("%s%s%s", ANSI_COLOR_RED, msg, ANSI_COLOR_RESET);
                    }
                }
            }
            close(sockfd); 
        }
    }
    return 1;
}

void removeAllSubstring(char *s, const char *toremove)
{ // remove substring from the string
    while( (s = strstr(s, toremove)) ) memmove(s, (s + strlen(toremove)), (1 + strlen(s + strlen(toremove))));
}

void removeSubstring(char *s, const char *toremove)
{ // remove substring from the string
    if ( (s = strstr(s, toremove)) ) memmove(s, (s + strlen(toremove)), (1 + strlen(s + strlen(toremove))));
}

