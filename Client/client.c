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
#include "../color_print.h"
#include <math.h>

#define NAMELEN 20
#define MSGSIZE 2048
#define FILEBUF 16384

#define NIPQUAD(addr) ((unsigned char *)&addr)[0], ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[2], ((unsigned char *)&addr)[3]

void removeSubstring(char *, const char *);
void removeAllSubstring(char *, const char *);

int main(int argc, char *argv[])
{
     signal(SIGCHLD, SIG_IGN);
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
            memset(msg, '\0', MSGSIZE);
            sprintf(msg, "CLIENT");
            write(sockfd, msg, strlen(msg));
            if ( !fork() )
            {   // writing to the server
                while ( 1 )
                {
                    memset(msg, '\0', MSGSIZE);
                    printf(SET_CYAN("Input: "));
                    //1 -- INPUT BUFFER
                    if ( fgets(msg, MSGSIZE, stdin) )
                    {
                        if ( strcmp(msg, "EXIT") ) 
                        {
                            write(sockfd, msg, strlen(msg));
                            break;
                        }
                        if ( strcasestr(msg, "FILE") )
                        {
                            FILE *fp;
                            struct stat file_stat;
                            char ext[5], fname[50], tmp[MSGSIZE], file_content[FILEBUF];
                            strcpy(tmp, msg);
                            removeSubstring(tmp, "FILE");
                            strcpy(ext, strtok(tmp, " "));
                            strcpy(fname, strtok(NULL, " \n"));
                            fname[strlen(fname)]='\0';
                            if ( !access(fname, F_OK) )
                            {
                                if ( !(fp = fopen(fname, "rb")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                                else {
                                    stat(fname, &file_stat);
                                    printf(SET_RED("Total file size in bytes: %5lld\n"), file_stat.st_size);
                                    strcpy(msg, strtok(msg, "\n"));
                                    sprintf(msg, "%s %lld", msg, file_stat.st_size);
                                    write(sockfd, msg, strlen(msg));
                                    memset(msg, '\0', MSGSIZE);
                                    memset(file_content, '\0', FILEBUF);
                                    int nbr=0, total_bytes_r=0, total_bytes_s=0;
                                    while ( total_bytes_r < file_stat.st_size )
                                    {
                                        memset(file_content, '\0', FILEBUF);
                                        nbr = fread(file_content, 1, FILEBUF, fp);
                                        total_bytes_r = total_bytes_r + nbr;
                                        total_bytes_s = total_bytes_s + send(sockfd, file_content, nbr, MSG_DONTROUTE);
                                        if ( !(total_bytes_s*100/file_stat.st_size % 10) ) printf(SET_MAGENTA("%f..."), total_bytes_s*100./file_stat.st_size);
                                    }
                                    fclose(fp);
                                    printf(SET_RED("\nTotal bytes read: %4d | sent %4d\n"), total_bytes_r, total_bytes_s);
                                }
                            }
                        } else if ( strcasestr(msg, "ADDUSER") || strcasestr(msg, "CHANEXIT") ) {
                            char tmp[MSGSIZE];
                            memset(tmp, '\0', MSGSIZE);
                            strcpy(tmp, msg);
                            if ( !strcasecmp(strtok(tmp, " "), "CLIENT") )
                            {
                                if ( strcasestr(msg, "ADDUSER") )
                                {
                                    strtok(NULL, " ");
                                    if ( strtok(NULL, " ") )
                                    {
                                        if ( strtok(NULL, " ") )
                                        { // all good
                                            write(sockfd, msg, strlen(msg));
                                        } else {
                                            printf(SET_RED("No channel was not passed in this message.\n"));
                                        }
                                    } else {
                                        printf(SET_RED("An username was not passed in this message.\n"));
                                    }
                                } else {
                                    strtok(NULL, " ");
                                    if ( strtok(NULL, " ") )
                                    { // all good
                                        write(sockfd, msg, strlen(msg));
                                    } else {
                                        printf(SET_RED("No channel was not passed in this message.\n"));
                                    }
                                }
                            } else {
                                printf(SET_RED("'CLIENT' was not passed in this message.\n"));
                            }
                        } else write(sockfd, msg, strlen(msg));
                    }
                }
            } else {
                // reading from the server
                char msg[MSGSIZE];
                while ( 1 )
                {
                    memset(msg, '\0', MSGSIZE);
                    read(sockfd, msg, MSGSIZE);
                    if ( strcasestr(msg, "FILE") )
                    { // Sending files
                        FILE *fp;
                        char ext[5], fname[50];
                        removeSubstring(msg, "FILE ");
                        strcpy(ext, strtok(msg, " "));
                        strcpy(fname, strtok(NULL, " "));
                        int total_bytes = atoi(strtok(NULL, " \n"));
                        printf(SET_CYAN("Total bytes expecting: %5d\n"), total_bytes);
                        if ( !access(fname, F_OK) )
                        {
                            removeSubstring(fname, ".");
                            removeSubstring(fname, ext);
                            sprintf(fname, "%s_tmp.%s", fname, ext);
                            if( remove(fname) != 0 ) perror("Error deleting file"); else puts("File successfully deleted");
                            if ( !(fp = fopen(fname, "wb")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                        } else {
                            if ( !(fp = fopen(fname, "wb")) ) printf(SET_RED("Error creating the file: %s\n"), fname);
                        }
                        if ( fp )
                        {
                            int nbr=0, total_bytes_r=0, total_bytes_s=0, i;
                            char file_content[FILEBUF];
                            while ( total_bytes_s < total_bytes )
                            {
                                memset(file_content, '\0', FILEBUF);
                                nbr = recv(sockfd, file_content, FILEBUF, MSG_DONTROUTE); //read(sockfd, msg, MSGSIZE);
                                total_bytes_r = total_bytes_r + nbr;
                                for ( i=0; i<nbr; ++i ) total_bytes_s = total_bytes_s + fprintf(fp, "%c", file_content[i]);
                                if ( !(total_bytes_s*100/total_bytes % 10) ) printf(SET_RED("%f..."), total_bytes_s*100./total_bytes);
                            }
                            fclose(fp);
                            printf(SET_RED("\nTotal bytes recv: %4d | written: %4d\n"), total_bytes_r, total_bytes_s);
                        }
                    } else if ( !strcasecmp(msg, "VID") ) {
                        // Handle video feed here....
                    } else if ( strcasestr(msg, "IM") ) { // all the rest are messages //2 - OUPUT BUFFER
                        printf("%s%s%s", ANSI_COLOR_RED, msg, ANSI_COLOR_RESET);
                    }
                    if ( strcmp(msg, "EXIT") )
                    {
                        removeSubstring(msg, "EXIT ");
                        printf("%s\n", msg);
                        break;
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
