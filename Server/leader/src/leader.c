#include <pthread.h>
#include <string.h>
#include "leader.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LOG 0

//"Constructor" for LeaderInfo
//returns a leader object with an "empty" serverList, and initialized mutexes
struct LeaderInfo * leader_genLeaderInfo(){
	struct LeaderInfo * li = malloc(sizeof( struct LeaderInfo ));
	int i;
	for( i = 0; i < MAX_SERVERS; i++ ){
		li->serverList[i].inUse = 0;
		pthread_mutex_init( &(li->serverLocks[i]), NULL );
	}
	pthread_mutex_init( &(li->serverListLock), NULL );
	li->nservers = 0;
	return li;
}

//////////////////////////////////////////////////////////////////////////////////
//																				//
//							Operations on the Leader							//
//																				//
//////////////////////////////////////////////////////////////////////////////////

int leader_addServer( struct LeaderInfo * li, char * hostname, int port ){
	int i, serverId = -1;
	if( li->nservers < MAX_SERVERS ){
		pthread_mutex_lock( &(li->serverListLock) );
		for( i = 0; (i < MAX_SERVERS) && (li->serverList[i].inUse); i++ );
		strcpy( li->serverList[i].hostname, hostname );
		li->serverList[i].port = port;
		li->serverList[i].inUse = 1;
		li->nservers++;
		pthread_mutex_unlock( &(li->serverListLock) );
		serverId = i;
	}
	return serverId;
}

//maybe delete by id;
int leader_removeServer( struct LeaderInfo * li, char * hostname, int port ){
	int i, success = 0;
	if( li->nservers > 0 ){
		pthread_mutex_lock( &(li->serverListLock) );
		for( i = 0; (i < MAX_SERVERS) && ((!li->serverList[i].inUse) || (strcmp(li->serverList[i].hostname, hostname)!=0) || (li->serverList[i].port != port)); i++ );
		if( i < MAX_SERVERS ){
			li->serverList[i].inUse = 0;
			success = 1;
			li->nservers--;
		}
		pthread_mutex_unlock( &(li->serverListLock) );
	}
	return success;
}

//todo, check buffer bounds?
int leader_getServerList( struct LeaderInfo * li, char * buffer, int bufferSize ){
	if(LOG)printf( "I'm getting the server list\n" );
	int i, nservers = 0;
	buffer[0] = '\0';
	pthread_mutex_lock( &(li->serverListLock) );
	for( i=0; i < MAX_SERVERS; i++ )
		if( li->serverList[i].inUse ){
			sprintf( buffer, "%s%s:%d\n", buffer, li->serverList[i].hostname, li->serverList[i].port );
			if(LOG)printf( "buffer = %s\n", buffer );
			nservers++;
		}
	pthread_mutex_unlock( &(li->serverListLock) );
	return nservers;
}

//////////////////////////////////////////////////////////////////////////////////
//																				//
//							    Message Handling								//
//																				//
//////////////////////////////////////////////////////////////////////////////////

int msg_handleUnknown( connfd ){
	char response[BUFF_SIZE];
	memset( response, '\0', BUFF_SIZE );
	strcpy( response, "LEADER UNKNOWN RESPONSE\n");
	write( connfd, response, strlen(response) );
	return 1;
}

int msg_handleRmvServer( struct LeaderInfo * li, char * msg, int connfd ){
	char * tok, hostname[NAME_SIZE], response[BUFF_SIZE];
	memset( response, '\0', BUFF_SIZE );
	int port, ret;
	tok = strtok( msg, "\n\r" );
	strcpy( hostname, tok );
	tok = strtok( NULL, "\n\r" );
	port = atoi(tok);
	strcpy( response, "LEADER RMVSERVER RESPONSE\n" );
	if( (ret = leader_removeServer( li, hostname, port ) ) )
		strcat( response, "SUCCESS\n" );
	else
		strcat( response, "FAILURE: NO SUCH SERVER FOUND\n" );
	write( connfd, response, strlen(response ) );
	return ret; // returning ret
}

int msg_handleGetServers( struct LeaderInfo * li, int connfd ){
	if(LOG)printf( "Got a server list request\n" );
	int writereturn;
	int nservers = 0;
	char response[BUFF_SIZE], serverList[BUFF_SIZE];
	memset( response, '\0', BUFF_SIZE );
	strcpy( response, "LEADER GETSERVERS RESPONSE\n" );
	nservers = leader_getServerList( li, serverList, sizeof(serverList) );
	if(LOG)printf( "got the serverlist\n" );
	sprintf( response, "%s%d\n%s", response, nservers, serverList );
	if(LOG)printf( "writing to socket: %d\n" );
	writereturn = write( connfd, response, strlen(response) );
	if(LOG)printf( "wrote the serverlist: %d chars, and %d servers\n", writereturn, nservers  );
	return nservers;
}

int msg_handleAddServer( struct LeaderInfo * li, char * msg, int connfd ){
	char * tok, hostname[NAME_SIZE], response[BUFF_SIZE];
	memset( response, '\0', BUFF_SIZE );
	int port, ret;
	tok = strtok( msg, "\n\r" );
	strcpy( hostname, tok );
	tok = strtok( NULL, "\n\r" );
	port = atoi(tok);
	strcpy( response, "LEADER ADDSERVER RESPONSE\n" );
	if( (ret = leader_addServer( li, hostname, port)) >= 0 )
		strcat( response, "SUCCESS\n" );
	else
		strcat( response, "FAILURE: NUM SERVERS MAXED\n" );
	write( connfd, response, strlen(response)+1 );
	return ret;
}

int msg_stripHeader( char * msg, char **msgPtr ){
	char * tok;
	int msgType;
	//change to '\n'
	*msgPtr = strchr( msg, '\n' ) + 1;
	tok = strtok( msg, "\n\r" );
	if( strcmp( tok, "ADDSERVER" ) == 0 )
		msgType = MSG_ADDSERVER;
	else if( strcmp( tok, "RMVSERVER" ) == 0 )
		msgType = MSG_RMVSERVER;
	else if( strcmp( tok, "GETSERVERS" ) == 0 )
		msgType = MSG_GETSERVERS;
	else
		msgType = MSG_UNKNOWN;
	return msgType;
}

int msg_handleMsg( struct LeaderInfo * li, char * msg, int msgType, int connfd ){
	int ret_val = 0;
	switch( msgType ){
		case MSG_ADDSERVER:
			ret_val = msg_handleAddServer( li, msg, connfd );
			break;
		case MSG_RMVSERVER:
			ret_val = msg_handleRmvServer( li, msg, connfd );
			break;
		case MSG_GETSERVERS:
			ret_val = msg_handleGetServers( li, connfd );
			break;
		case MSG_UNKNOWN:
			ret_val = msg_handleUnknown( connfd ); //msg_handleUnknown( li, msg, connfd );
			break;
	}
	return ret_val;
}
