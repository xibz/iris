#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//project includes
#include <threadpool.h>
#include "util.h"
#include "server.h"

#define NAME_SIZE 32
#define BUFF_SIZE 1024

#define USAGE "Usage: [-l] portno [leaderHostname leaderPort]\n"

void handleRequest( void * args );

int main( int argc, char * argv[] ){
	//Parse command line arguments
	int opt, port, leaderMode = 0, independentMode = 1, socketDesc, listenfd, connfd;
	char leaderHostname[ NAME_SIZE ], leaderPort[7], myHostname[ NAME_SIZE ];
	struct ServerInfo * si;
	while( ( opt=getopt( argc, argv, "l") ) != -1 ){
		switch( opt ){
			case 'l':
				leaderMode = 1;
				break;
		}
	}

	//if we're missing the port argument
	if( optind == argc ){
		printf( USAGE );
		exit( EXIT_FAILURE );
	}
	//if the port argument ain't an int
	if( (port=atoi(argv[optind])) == 0 ){
		printf( USAGE );
		exit( EXIT_FAILURE );
	}

	//If user supplied leader to connect to
	if( optind + 3 == argc ){
		if( leaderMode == 1 ){
			printf( "Ignoring leader arguments, as this server was specified as a leader\n" );
		}
		else{
			strcpy( leaderHostname, argv[optind+1] );
			strcpy( leaderPort, argv[optind+2] );
			independentMode = 0;
		}
	//If the user didn't supply a leader, there better not be any extra arguments
	} else if( optind+1 != argc ) {
		printf( USAGE );
		exit( EXIT_FAILURE );
	}

	gethostname( myHostname, sizeof( myHostname ) );
	si = genServerInfo( myHostname, port, leaderHostname, atoi(leaderPort), leaderMode, independentMode );

	if( !(leaderMode || independentMode) ){
		//TODO: connect to leader and send hellos
	}

	listenfd = getListenDesc( port );
	listen( listenfd, 20 );
	threadpool_t * pool = threadpool_create(20, 50, 0);

	void ** args;
	while( 1 ){
		connfd = accept( listenfd, (struct sockaddr*)NULL, NULL);
		args = malloc(sizeof(void *)*2);
		args[0] = (void *) si;
		args[1] = (void *) connfd;
		threadpool_add( pool, (void *) handleRequest, (void *) args, 0 );
	}

	destroyServerInfo( si );
}

void handleRequest( void * args ){
	void ** argsList = (void **) args;
	struct ServerInfo * si = (struct ServerInfo *) argsList[0];
	int connfd = (int) argsList[1], charsRead;
	char rcvBuff[BUFF_SIZE], *msgPtr, responseBuff[BUFF_SIZE];
	
	if( (charsRead = read( connfd, rcvBuff, sizeof(rcvBuff) ) ) > 0 ){
		int msgType = stripHeader( rcvBuff, &msgPtr );
		handleMessage( si, msgType, msgPtr, responseBuff, sizeof(responseBuff));
	}

	close(connfd);
	free( args );
	return;
}
