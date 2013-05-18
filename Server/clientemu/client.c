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
 
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) perror("Socket Error");
    else {
        address.sin_family      = AF_INET;
        address.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)gethostbyname(ADDR)->h_addr_list[0])));
        address.sin_port        = htons(PORT);

        if ( connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0 ) perror("Connection Error");
        else {
            inet_ntop(AF_INET, &(address.sin_addr), str, INET_ADDRSTRLEN);
            printf("Connection established with Server: %s\n", str);
            if ( !fork() )
            {   // writing to the server
                while ( 1 )
                {
                    memset(msg, '\0', MSGSIZE);
                    printf("Input: ");
                    //1 -- INPUT BUFFER
                    if ( fgets(msg, MSGSIZE, stdin) )
                    {
                        if ( !strncmp(msg, "EXIT", 4) ) 
                        {
                            write(sockfd, msg, strlen(msg));
                            break;
                        } else {
                            char tmp[MSGSIZE];
                            memset(tmp, '\0', MSGSIZE);
                            strcpy(tmp, msg);
                            strtok(tmp, " ");
                            if ( !strncmp(msg, "ADDUSER", 7) )
                            {
                                if ( strtok(NULL, " \n") )
                                {// all good
                                    msg[7] = '\n';
                                    printf("*%s*\n", msg);
                                    write(sockfd, msg, strlen(msg));
                                } else {
                                    printf("An username was not passed in this message.\n");
                                }
                            } else if ( !strncmp(msg, "JOINCHANNEL", 11) || !strncmp(msg, "LEAVECHANNEL", 12) || !strncmp(msg, "CREATECHANNEL", 13) ) {
                                if ( strtok(NULL, " \n") )
                                {// all good
                                    if ( !strncmp(msg, "JOINCHANNEL", 11)   ) msg[11] = '\n';
                                    if ( !strncmp(msg, "LEAVECHANNEL", 12)  ) msg[12] = '\n';
                                    if ( !strncmp(msg, "CREATECHANNEL", 13) ) msg[13] = '\n';
                                    write(sockfd, msg, strlen(msg));
                                } else {
                                    printf("No channel was not passed in this message.\n");
                                }
                            } else if ( !strncmp(msg, "BROADCAST", 9) ) {
                                if ( strtok(NULL, " ") )
                                {
                                    if ( strtok(NULL, " \n") )
                                    {// all good
                                        msg[9] = '\n';
                                        msg[strchr(msg, ' ') - msg] = '\n';
                                        printf("*%s*\n", msg);
                                        write(sockfd, msg, strlen(msg));
                                    } else {
                                        printf("No message was not passed in this message.\n");
                                    }
                                } else {
                                    printf("No channel was not passed in this message.\n");
                                }
                            } else if ( !strncmp(msg, "QUITUSER", 8) ) {
                                write(sockfd, msg, strlen(msg));
                            } else if ( !strncmp(msg, "FILE", 4) ) {
                                FILE *fp;
                                struct stat file_stat;
                                char ext[5], fname[50], tmp[MSGSIZE], file_content[FILEBUF];
                                removeSubstring(tmp, "FILE");
                                strcpy(ext, strtok(tmp, " "));
                                strcpy(fname, strtok(NULL, " \n"));
                                fname[strlen(fname)]='\0';
                                if ( !access(fname, F_OK) )
                                {
                                    if ( !(fp = fopen(fname, "rb")) ) printf("Error creating the file: %s\n", fname);
                                    else {
                                        stat(fname, &file_stat);
                                        printf("Total file size in bytes: %5lld\n", file_stat.st_size);
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
                                            if ( !(total_bytes_s*100/file_stat.st_size % 10) ) printf("%f...", total_bytes_s*100./file_stat.st_size);
                                        }
                                        fclose(fp);
                                        printf("\nTotal bytes read: %4d | sent %4d\n", total_bytes_r, total_bytes_s);
                                    }
                                }
                            }
                        }                            
                    }
                }
            } else {
                // reading from the server
                char msg[MSGSIZE];
                while ( 1 )
                {
                    memset(msg, '\0', MSGSIZE);
                    read(sockfd, msg, MSGSIZE);
                    if ( !strncmp(msg, "EXIT", 4) )
                    {
                        removeSubstring(msg, "EXIT ");
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
                        printf("Total bytes expecting: %5d\n", total_bytes);
                        if ( !access(fname, F_OK) )
                        {
                            removeSubstring(fname, ".");
                            removeSubstring(fname, ext);
                            sprintf(fname, "%s_tmp.%s", fname, ext);
                            if( remove(fname) != 0 ) perror("Error deleting file"); else puts("File successfully deleted");
                            if ( !(fp = fopen(fname, "wb")) ) printf("Error creating the file: %s\n", fname);
                        } else {
                            if ( !(fp = fopen(fname, "wb")) ) printf("Error creating the file: %s\n", fname);
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
                                if ( !(total_bytes_s*100/total_bytes % 10) ) printf("%f...", total_bytes_s*100./total_bytes);
                            }
                            fclose(fp);
                            printf("\nTotal bytes recv: %4d | written: %4d\n", total_bytes_r, total_bytes_s);
                        }
                    } else if ( !strncmp(msg, "VID", 3) ) {
                        // Handle video feed here....
                    } else if ( strlen(msg) > 1 ) { // all the rest are messages //2 - OUPUT BUFFER
                        printf("msg recv:%s", msg);
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
