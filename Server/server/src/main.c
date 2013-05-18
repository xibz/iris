#include "server.h"
#include "util.h"
#include "../lib/threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

void handleRequest( void * args );

int main( int argc, char * argv[])
{
	int myPort, leaderSocket, userId, connfd, listenfd, charsRead, msgType;
	char leaderHostname[NAME_SIZE], leaderPort[10], myHostname[NAME_SIZE], rcvBuff[BUFF_SIZE], * msgPtr;
	struct ServerInfo * si;
	threadpool_t * pool;
	void ** args;
	if((argc!=4) || ((myPort=atoi(argv[1]))==0)){
		printf( "Usage: %s <listening port> <l. hostname> <l. port>\n", argv[0] );
		exit( EXIT_FAILURE );
	}
	strcpy( leaderHostname, argv[2] );
	strcpy( leaderPort, argv[3] );
	gethostname( myHostname, sizeof( myHostname ) );
	if( (leaderSocket = util_getConnection( leaderHostname, leaderPort ))<0 ){
		printf( "Could not establish connection to leader\n" );
		exit( EXIT_FAILURE );
	}

	si = genServerInfo( myHostname, myPort, leaderHostname, atoi(leaderPort), leaderSocket );
	if( !sendLeader_addServer( si ) ){
		printf( "Failed to register server with leader\n" );
		exit( EXIT_FAILURE );
	}
	listenfd = util_getListenDesc( myPort );
	listen( listenfd, 20 );

	pool = threadpool_create( MAX_USERS, 50, 0 );

	while ( 1 )
	{
		connfd = accept( listenfd, (struct sockaddr *)NULL, NULL);
		memset( rcvBuff, '\0', BUFF_SIZE );
		if( (charsRead = read( connfd, rcvBuff, sizeof(rcvBuff) )) > 0 )
		{
			printf( "rcvBuff = %s\n", rcvBuff );
			fflush(stdout);
			msgType = msg_stripHeader( rcvBuff, &msgPtr );
			if( msgType == ADDUSER ) userId = msg_handleMsg( si, msgPtr, msgType, connfd );
			else close( connfd );
			memset(rcvBuff, '\0', BUFF_SIZE);
		}
		if( msgType != ADDUSER ) continue;

		args = malloc(sizeof(void *)*3);
		args[0] = (void *) si;
		args[1] = (void *) connfd;
		args[2] = (void *) userId;
		threadpool_add( pool, (void *) handleRequest, (void *) args, 0 );
	}
}

void handleRequest( void * args )
{
	void ** argsList = (void **) args;
	struct ServerInfo * si = (struct ServerInfo *) argsList[0];
	int connfd = (int) argsList[1];
	int userId = (int) argsList[2];
	free( args );
	int charsRead, msgType = 0;
	char rcvBuff[BUFF_SIZE] = "junk", *msgPtr;

	while( msgType != QUITUSER )
	{
		//pthread_mutex_lock( &(si->userLocks[userId]) );
		memset( rcvBuff, '\0', BUFF_SIZE );
		if( ((charsRead = read( connfd, rcvBuff, sizeof( rcvBuff ) )) > 0) && (rcvBuff[0] != '\0') )
		{
			//pthread_mutex_unlock( &(si->userLocks[userId]) );
			printf( "\ncharsRead = %d\n", strlen(rcvBuff));
			printf( "rcvBuff = %s\n", rcvBuff );
			fflush(stdout);
			msgType = msg_stripHeader( rcvBuff, &msgPtr );
			//pthread_mutex_lock( &(si->userLocks[userId]) );
			msg_handleMsg( si, msgPtr, msgType, userId );
			//pthread_mutex_unlock( &(si->userLocks[userId]) );
		}
	}
	printf( "i'm leaving\n" );
	close( connfd );
}
