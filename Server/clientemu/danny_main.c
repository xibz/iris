#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>


#define BUFF_SIZE 1024

#define ADDUSER_MSG "ADDUSER\nGiRd0t\n"
#define JOINCHANNEL_MSG "JOINCHANNEL\nAOL\n"
#define CREATECHANNEL_MSG "CREATECHANNEL\nAOL\n"
#define LEAVECHANNEL_MSG "LEAVECHANNEL\nAOL\n"
#define BROADCAST_MSG "BROADCAST\nAOL\nomg sup fools\n"
#define QUIT_MSG "QUITUSER\n "
#define WEIRD_MSG "THIS IS A BOW TIE IN STRING FORM\n"

int util_getConnection( char * hostname, char * port );

int main( int argc, char * argv[] ){
	char msg[BUFF_SIZE] = "ADDUSER\nGiRdOt\n";
	char response[BUFF_SIZE];
	char c;
	int socketDesc = util_getConnection( argv[1], argv[2] );
	while( 1 ){
		printf("Input: ");
		scanf( "%c", &c );
		switch( c ){
			case 'a':
				strcpy( msg, ADDUSER_MSG );
				write( socketDesc, msg, sizeof( msg ) );
				read( socketDesc, response, sizeof( response ) );
				printf( "response:\n%s\n\n", response );
				break;
			case 'j':
				strcpy( msg, JOINCHANNEL_MSG );
				write( socketDesc, msg, sizeof( msg ) );
				read( socketDesc, response, sizeof( response ) );
				printf( "response:\n%s\n\n", response );
				break;
			case 'c':
				strcpy( msg, CREATECHANNEL_MSG );
				write( socketDesc, msg, sizeof( msg ) );
				read( socketDesc, response, sizeof( response ) );
				printf( "response:\n%s\n\n", response );
				break;
			case 'l':
				strcpy( msg, LEAVECHANNEL_MSG );
				write( socketDesc, msg, sizeof( msg ) );
				read( socketDesc, response, sizeof( response ) );
				printf( "response:\n%s\n\n", response );
				break;
			case 'b':
				strcpy( msg, BROADCAST_MSG );
				write( socketDesc, msg, sizeof( msg ) );
				break;
			case 'r':
				read( socketDesc, response, sizeof( response ) );
				printf( "response:\n%s\n\n", response );
				break;
			case 'q':
				strcpy( msg, QUIT_MSG );
				write( socketDesc, msg, sizeof( msg ) );
				read( socketDesc, response, sizeof( response ) );
				printf( "response:\n%s\n\n", response );
				break;
				
			case 'k':
				strcpy( msg, WEIRD_MSG );
				write( socketDesc, msg, sizeof( msg ) );
				break;
			default:
				continue;
		}
	}
	return 0;
}

int util_getConnection( char * hostname, char *port ){
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	
	memset( &hints, 0, sizeof(struct addrinfo) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	s = getaddrinfo(hostname, port, &hints, &result);
	if( s!= 0 ){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for( rp = result; rp != NULL; rp = rp->ai_next ){
		sfd = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol );
		if( sfd == -1 )
			continue;
		if( connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1 )
			break;
		close(sfd);
	}

	if( rp == NULL ){
		fprintf( stderr, "Could not connect\n" );
		return -1;
	}
	
	//Now we know we're connected
	return sfd;
}

