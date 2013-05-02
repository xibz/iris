#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define SET_RED(x)      ANSI_COLOR_RED      x ANSI_COLOR_RESET
#define SET_GREEN(x)    ANSI_COLOR_GREEN    x ANSI_COLOR_RESET
#define SET_YELLOW(x)   ANSI_COLOR_YELLOW   x ANSI_COLOR_RESET
#define SET_BLUE(x)     ANSI_COLOR_BLUE     x ANSI_COLOR_RESET
#define SET_MAGENTA(x)  ANSI_COLOR_MAGENTA  x ANSI_COLOR_RESET
#define SET_CYAN(x)     ANSI_COLOR_CYAN     x ANSI_COLOR_RESET

#define MSGSIZE 4096

#define NIPQUAD(addr) ((unsigned char *)&addr)[0], ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[2], ((unsigned char *)&addr)[3]

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
            while ( 1 )
            {
                printf("Input:\n");
                bzero(msg, MSGSIZE);
                if ( fgets(msg, MSGSIZE, stdin) )
                {
                    msg[strlen(msg)-1] = '\0';
                    //whatever you want to do with the msg
                    write(sockfd, msg, strlen(msg));
                }
                if ( strcasestr(msg, "EXIT") || strcasestr(msg, "exit") ) break; else bzero(msg, MSGSIZE);
            }
            close(sockfd); 
        }
    }
    return 1;
}
