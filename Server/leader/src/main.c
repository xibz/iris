#include "leader.h"
#include "../lib/threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include <unistd.h>
#include <sys/socket.h>

void handleRequest( void * args );

int main( int argc, char * argv[] )
{
	char rcvBuff[BUFF_SIZE], *msgPtr;
	int port, listenfd, connfd, charsRead, msgType, serverId;
	struct LeaderInfo * li;
	threadpool_t * pool;
	void ** args;

	if( (argc != 2) || ((port=atoi(argv[1]))==0) ){
		printf( "Usage: %s <listening port>\n", argv[0] );
		exit( EXIT_FAILURE );
	}

	li = leader_genLeaderInfo();
	listenfd = util_getListenDesc( port );
	listen( listenfd, 20 );

	pool = threadpool_create(MAX_SERVERS, 50, 0);

	while( 1 )
	{
		connfd = accept( listenfd, (struct sockaddr *)NULL, NULL);
		memset( rcvBuff, '\0', BUFF_SIZE );
		if ( (charsRead = read( connfd, rcvBuff, sizeof(rcvBuff) )) > 0 )
		{
			printf( "rcvBuff = %s\n", rcvBuff );
			fflush(stdout);
			msgType = msg_stripHeader( rcvBuff, &msgPtr );
			if ( msgType == MSG_ADDSERVER ) serverId = msg_handleMsg( li, msgPtr, msgType, connfd );
			else close( connfd );
		}
		if( msgType != MSG_ADDSERVER ) continue;

		args = malloc(sizeof(void *)*3);
		args[0] = (void *) li;
		args[1] = (void *) connfd;
		args[2] = (void *) serverId;
		threadpool_add( pool, (void *) handleRequest, (void *) args, 0 );
	}
}

void handleRequest( void * args )
{
	void ** argsList = (void **) args;
	struct LeaderInfo * li = (struct LeaderInfo *) argsList[0];
	int connfd = (int) argsList[1];
	int serverId = (int) argsList[2];
	free( args );
	int charsRead, msgType = 0;
	char rcvBuff[BUFF_SIZE] = "junk", *msgPtr;

	while( msgType != MSG_RMVSERVER )
	{
		memset( rcvBuff, '\0', BUFF_SIZE );
		printf( "leader read socket: %d\n", connfd );
		while( ((charsRead = read( connfd, rcvBuff, sizeof( rcvBuff ) )) > 0) && (rcvBuff[0] != '\0') )
		{
			printf( "rcvBuff = %s\n", rcvBuff );
			fflush(stdout);
			pthread_mutex_lock( &(li->serverLocks[serverId]) );
			msgType = msg_stripHeader( rcvBuff, &msgPtr );
			msg_handleMsg( li, msgPtr, msgType, connfd );
			pthread_mutex_unlock( &(li->serverLocks[serverId]) );
		}
	}
	printf( "i'm leaving\n" );
	close( connfd );
}
