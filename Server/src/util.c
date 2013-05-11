#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int getListenDesc( int port ){
	int listenfd=0;

	//Used to describe the type of socket connection
	struct sockaddr_in serv_addr;
	memset( &serv_addr, '0', sizeof(serv_addr));
	
	//If we can't listen on the socket, exit out
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		printf("Error creating socket");
		exit( EXIT_FAILURE );
	}

	//Describe the socket connection
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	//If we can't bind on the socket, exit out
	if( bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0 ){
		printf("Error binding on port: %d\n", port);
		exit( EXIT_FAILURE );
	}

	return listenfd;
}

int getConnection( char * hostname, char * port ){
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, j;
	
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

