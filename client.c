#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include "color_print.h"

#define MSGSIZE 4096
#define FILESIZE 1048576
#define MAXSIZE 1052672 //MSGSIZE + FILESIZE

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
                            char ext[5], fname[50], tmp[MSGSIZE], file_content[MAXSIZE];
                            strcpy(tmp, msg);
                            removeSubstring(tmp, "FILE");
                            removeSubstring(tmp, "file");
                            strcpy(ext, strtok(tmp, " "));
                            strcpy(fname, strtok(NULL, " \n"));
                            if ( !access(fname, F_OK) )
                            {
                                if ( !(fp = fopen(fname, "r")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                                else {
                                    strncpy(file_content, msg, strlen(msg)-1);
                                    sprintf(file_content, "%s CONTENT:", file_content);
                                    printf("total len B:%ld\n", strlen(file_content));
                                    fread(file_content+strlen(file_content), 1, sizeof(char) * FILESIZE, fp);
                                    printf("total len A:%ld\n", strlen(file_content));
                                    write(sockfd, file_content, strlen(file_content));
                                    fclose(fp);
                                }
                            }
                        } else write(sockfd, msg, strlen(msg));
                    }
                    if ( strcasestr(msg, "EXIT") ) break; else bzero(msg, MSGSIZE);
                }
            } else {
                // reading from the server
                char msg[MAXSIZE];
                while ( 1 )
                {
                    bzero(msg, MAXSIZE);
                    if ( read(sockfd, msg, MAXSIZE) )
                    {
                        printf("recv len:%ld\n", strlen(msg));
                        if ( strcasestr(msg, "FILE") )
                        { // Sending files of size < 1MB & 40 characters or less in filename
                            FILE *fp;
                            char ext[5], fname[50], file_content[MAXSIZE];
                            removeSubstring(msg, "FILE ");
                            removeSubstring(msg, "file ");
                            strcpy(file_content, msg);
                            strcpy(ext, strtok(msg, " "));
                            removeSubstring(file_content, ext);
                            removeSubstring(file_content, " ");
                            printf("ext:%s|\n", ext);
                            strcpy(fname, strtok(NULL, " CONTENT:"));
                            removeSubstring(file_content, fname);
                            removeSubstring(file_content, " ");
                            printf("fname:%s|\n", fname);
                            removeSubstring(file_content, "CONTENT:");
                            printf("content len:%ld\n", strlen(file_content));
                            if ( !access(fname, F_OK) )
                            {
                                removeSubstring(fname, ".");
                                removeSubstring(fname, ext);
                                sprintf(fname, "%s_tmp.%s", fname, ext);
                                if ( !(fp = fopen(fname, "w")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                                else {
                                    fwrite(file_content, 1, strlen(file_content), fp);
                                    fclose(fp);
                                }
                            } else {
                                if ( !(fp = fopen(fname, "w")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                                else {
                                    fwrite(file_content, 1, strlen(file_content), fp);
                                    fclose(fp);
                                }
                            }
                        } else if ( !strcasecmp(msg, "VID") ) {
                            // Handle video feed here....
                        } else { // all the rest are messages
                            printf("%s%s%s", ANSI_COLOR_RED, msg, ANSI_COLOR_RESET);
                        }
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

