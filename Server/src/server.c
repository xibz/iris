#include "server.h"
#include <listbyname.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

#define NUM_SUPPORTED_CHANNELS 100
#define NUM_SUPPORTED_USERS 32
#define NUM_USERS_PER_CHANNEL 64
#define HASHMAP_SIZE 1024
#define NAME_SIZE 32

struct User{
	char name[NAME_SIZE];
	int sockid;
};

struct Channel{
	char name[NAME_SIZE];
	struct ListByName * userList;
};

struct ServerInfo{
	struct ListByName * channelList;
	char myHostname[ NAME_SIZE ];
	int myPort;
	char leaderHostname[ NAME_SIZE ];
	int leaderPort;
	int leaderMode;
	int independentMode;
	int leadersocket;
};

struct ServerInfo * genServerInfo( char * myHostname,
									int myPort,
									char * leaderHostname,
									int leaderPort,
									int leaderMode,
									int independentMode ){
	struct ServerInfo * si;
	si = malloc( sizeof( struct ServerInfo ) );
	strcpy( si->myHostname, myHostname );
	si->myPort = myPort;
	strcpy( si->leaderHostname, leaderHostname );
	si->leaderPort = leaderPort;
	si->leaderMode = leaderMode;
	si->independentMode = independentMode;
	si->channelList = genListByName( NUM_SUPPORTED_CHANNELS );
	return si;
}

void destroyServerInfo( struct ServerInfo * si ){
	destroyListByName( si->channelList );
	free( si );
}

struct Channel * genChannel( char * name ){
	struct Channel * myChannel = malloc(sizeof(struct Channel));
	myChannel->userList = genListByName( NUM_SUPPORTED_USERS );
	strcpy( myChannel->name, name );
	return myChannel;
}

void destroyChannel( struct Channel * myChannel ){
	destroyListByName( myChannel->userList );
	free( myChannel );
}

int stripHeader( char * rcvBuff, char ** msgPtr ){
	int msgType;
	char * tok;
	*msgPtr = strchr( rcvBuff, '\n' ) + 1;
	tok = strtok( rcvBuff, "\n\r" );
	if( strcmp( tok, "CLIENT ADDUSER" ) == 0 ){
		return CLIENT_ADDUSER;
	} else {
		return -1;
	}
}

/*
CLIENT ADDUSER
USERNAME
CHANNEL
*/

void handleMessage( struct ServerInfo * si, int msgType, char * msgPtr, char * responseBuff, int responseBuffSize ){
	switch( msgType ){
		case CLIENT_ADDUSER:
			printf( "HANDLING A CLIENT ADDUSER\n" );
			break;
	}
}
