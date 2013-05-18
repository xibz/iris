#include "server.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int LOG = 0;

struct ServerInfo * genServerInfo(char * hostname, int port, char *lhostname, int lport, int lsocketDesc)
{
	struct ServerInfo * si = malloc(sizeof(struct ServerInfo));
	int i, j;
	strcpy( si->hostname, hostname );
	si->port = port;

	si->myLeader.socketDesc = lsocketDesc;
	strcpy(si->myLeader.hostname, lhostname);
	si->myLeader.port = lport;

	for( i = 0; i < MAX_CHANNELS * MAX_USERS_PER_CHANNEL ; i++ )
	{
		si->userList[i].inUse = 0;
		si->userList[i].index = i;
		pthread_mutex_init( &(si->userLocks[i]), NULL );
		for( j = 0; j < MAX_CHANNELS; j++ ) si->userList[i].channelIndexes[j] = -1;
	}

	for( i = 0; i < MAX_CHANNELS; i++ )
	{
		si->channelList[i].inUse = 0;
		pthread_mutex_init( &(si->channelLocks[i]), NULL );
		pthread_mutex_init( &(si->channelList[i].userListLock), NULL );
	}

	pthread_mutex_init( &(si->leaderLock), NULL );

	return si;
}

int sendLeader_addServer( struct ServerInfo * si )
{
	char msg[BUFF_SIZE], responseMsg[BUFF_SIZE], * tok;
	memset( msg, '\0', BUFF_SIZE );
	int charsRead;
	strcpy( msg, "ADDSERVER\n" );
	sprintf( msg, "%s%s\n", msg, si->hostname );
	sprintf( msg, "%s%d\n", msg, si->port );
	pthread_mutex_lock( &si->leaderLock );
	if(LOG)printf( "SENDING ADDSERVER: %s\n", msg );
	fflush(stdout);
	write( si->myLeader.socketDesc, msg, strlen(msg)+1 );
	memset( responseMsg, '\0', BUFF_SIZE );
	if ( (charsRead=read( si->myLeader.socketDesc, responseMsg, sizeof(responseMsg)) >0) )
	{
		pthread_mutex_unlock( &si->leaderLock );
		tok = strtok( responseMsg, "\r\n" );
		if ( strcmp( tok, "LEADER ADDSERVER RESPONSE" ) != 0 ) return 0;
		tok = strtok( NULL, "\r\n" );
		if ( strcmp( tok, "SUCCESS" ) != 0 ) return 0;
		else return 1;
	}
	pthread_mutex_unlock( &si->leaderLock );
	return 0;
}

int sendLeader_getServerList( struct ServerInfo * si, char * buffer ){
	char msg[BUFF_SIZE], responseMsg[BUFF_SIZE], * tok;
	memset( msg, '\0', BUFF_SIZE );
	int charsRead;
	strcpy( msg, "GETSERVERS\n" );
	pthread_mutex_lock( &si->leaderLock );
	if(LOG)printf( "Got the leader lock\n" );
	if(LOG)printf( "LEADER SOCKET before write: %d\n", si->myLeader.socketDesc );
	write( si->myLeader.socketDesc, msg, strlen(msg) );
	if(LOG)printf( "LEADER SOCKET after write: %d\n", si->myLeader.socketDesc );
	memset( responseMsg, '\0', BUFF_SIZE );
	if(LOG)printf( "I'm READING NOW\n" );
	if(LOG)printf( "LEADER SOCKET BEFORE READ: %d\n", si->myLeader.socketDesc );
	read( si->myLeader.socketDesc, responseMsg, BUFF_SIZE );
	if(LOG)printf( "LEADER SOCKET AFTER READ: %d\n", si->myLeader.socketDesc );
	//if ( (charsRead=read( si->myLeader.socketDesc, responseMsg, BUFF_SIZE )) >0 ) 
	{
		if(LOG)printf( "GOT A RESPONSE FROM THE LEADER\n" );
		pthread_mutex_unlock( &si->leaderLock );
		tok = strtok( responseMsg, "\r\n" );
		if ( strcmp( tok, "LEADER GETSERVERS RESPONSE" ) != 0 ) return 0;
		tok = strtok( NULL, "" ); //does this get the rest of the string?
		strcpy( buffer, tok );
		return 1;
	} /*else {
		if(LOG)printf ( "GOT A RESPONSE FROM THE LEADER\n" );
	}*/
	pthread_mutex_unlock( &si->leaderLock );
	return 0;
}

int handleClient_addUser( struct ServerInfo * si,char * msg, int socketDesc )
{
	char * tok, response[BUFF_SIZE], username[NAME_SIZE];
	memset( response, '\0', BUFF_SIZE );
	int i, ret = -1, j;
	strcpy( response, "ADDUSER RESPONSE\n" );
	if ( si->nusers >= MAX_CHANNELS * MAX_USERS_PER_CHANNEL ) strcat( response, "SERVER FULL\n" );
	else {
		tok = strtok( msg, "\n" );
		strcpy( username, tok );
		
		pthread_mutex_lock( &(si->userListLock) );
		//check to see if the username is unique
		for(i = 0; (i < MAX_CHANNELS * MAX_USERS_PER_CHANNEL) && ((!(si->userList[i].inUse)) || (strcmp(si->userList[i].name, username)!=0)); i++);
		if( i < MAX_CHANNELS * MAX_USERS_PER_CHANNEL )
		{
			pthread_mutex_unlock( &(si->userListLock) );
			strcat( response, "USERNAME TAKEN\n" );
		}
		else {
			for(i = 0; (i < MAX_CHANNELS * MAX_USERS_PER_CHANNEL) && (si->userList[i].inUse); i++ );
			si->userList[i].inUse = 1;
			strcpy( si->userList[i].name, username );
			si->userList[i].socketDesc = socketDesc;
			si->userList[i].index = i;
			//reset the users "connected channels" list
			for( j = 0; j < MAX_CHANNELS; j++ ) si->userList[i].channelIndexes[j] = -1;
			pthread_mutex_unlock( &(si->userListLock) );
			ret = i;
			si->nusers++;
			strcat( response, "SUCCESS\n" );
		}
	}
	write( socketDesc, response, strlen(response)+1 );
	return ret;
}

//the mutex at channelIndex should be locked at time of calling
//therefore, this function features nested mutex locks: channelLocks[i] -> userLocks[i]
int sendChannel_userJoinedChannel( struct ServerInfo * si, int userIndex, int channelIndex )
{
	char msg[BUFF_SIZE];
	memset( msg, '\0', BUFF_SIZE );
	int i;
	strcpy( msg, "USER JOINED\n" );
	sprintf( msg, "%s%s\n", msg, si->channelList[channelIndex].name );
	sprintf( msg, "%s%s\n", msg, si->userList[userIndex].name );
	for( i = 0; i < MAX_USERS_PER_CHANNEL; i++ )
	{
		if( (si->channelList[channelIndex].userList[i].inUse) && (si->channelList[channelIndex].userList[i].socketDesc != si->userList[userIndex].socketDesc) )
		{
			if( LOG ) printf( "I'M LOCKING USER MUTEX %d in user joined channel\n", si->channelList[channelIndex].userList[i].index  );
			pthread_mutex_lock( &si->userLocks[si->channelList[channelIndex].userList[i].index] );
			if( LOG )printf ("I LOCKED IT\n" );
			write( si->channelList[channelIndex].userList[i].socketDesc, msg, strlen(msg) );
			if( LOG )printf( "I'M UNLOCKING USER MUTEX %d\n", si->channelList[channelIndex].userList[i].index  );
			pthread_mutex_unlock( &si->userLocks[si->channelList[channelIndex].userList[i].index] );
			if( LOG )printf ("I LOCKED IT\n" );
		}
	}
	return 1;
}

//the mutex at channelIndex should be locked at time of calling
//therefore, this function features nested mutex locks: channelLocks[i] -> userLocks[i]
int sendChannel_userLeftChannel( struct ServerInfo * si, int userIndex, int channelIndex )
{
	char msg[BUFF_SIZE], otherUsers = 0;
	memset( msg, '\0', BUFF_SIZE );
	int i;
	strcpy( msg, "USER LEFT\n" );
	sprintf( msg, "%s%s\n", msg, si->channelList[channelIndex].name );
	sprintf( msg, "%s%s\n", msg, si->userList[userIndex].name );
	for( i = 0; i < MAX_USERS_PER_CHANNEL; i++ )
	{
		if( (si->channelList[channelIndex].userList[i].inUse) && (si->channelList[channelIndex].userList[i].socketDesc != si->userList[userIndex].socketDesc) )
		{
			if( LOG )printf( "I'M LOCKING USER MUTEX %d in user left channel\n", si->channelList[channelIndex].userList[i].index  );
			pthread_mutex_lock( &si->userLocks[si->channelList[channelIndex].userList[i].index] );
			if( LOG )printf( "I LOCKED IT\n" );
			write( si->channelList[channelIndex].userList[i].socketDesc, msg, strlen(msg) + 1 );
			if( LOG )printf( "I'M UNLOCKING USER MUTEX %d\n", si->channelList[channelIndex].userList[i].index  );
			pthread_mutex_unlock( &si->userLocks[si->channelList[channelIndex].userList[i].index] );
			if( LOG )printf( "I UNLOCKED IT\n" );
			otherUsers++;
		}
	}
	return otherUsers;
}

//the mutex at channelIndex should be locked at time of calling
//therefore, this function features nested mutex locks: channelLocks[i] -> userLocks[i]
int sendChannel_broadcastMsg( struct ServerInfo * si, int userIndex, int channelIndex, char * toBcast )
{
	//LOG = 1;
	//char msg[BUFF_SIZE];
	int i;
	//strcpy( msg, "USER BROADCAST\n" );
	//sprintf( msg, "%s%s\n", msg, si->channelList[channelIndex].name );
	//sprintf( msg, "%s%s\n", msg, si->userList[userIndex].name );
	//strcpy( msg, toBcast );
	for( i = 0; i < MAX_USERS_PER_CHANNEL; i++ )
	{
		if( LOG )printf( "Channel %d slot %d inUse = %d\n", channelIndex, i, si->channelList[channelIndex].userList[i].inUse);
		if( LOG )printf( "Their socket = %d vs my socket = %d\n", si->channelList[channelIndex].userList[i].socketDesc, si->userList[userIndex].socketDesc);
		if( (si->channelList[channelIndex].userList[i].inUse) && (si->channelList[channelIndex].userList[i].socketDesc != si->userList[userIndex].socketDesc) )
		{
			if( LOG )printf( "Got in\n" );
			if( LOG )printf( "Locking user %d in broadcastmsg\n", si->channelList[channelIndex].userList[i].index );
			pthread_mutex_lock( &si->userLocks[si->channelList[channelIndex].userList[i].index] );
			if( LOG )printf( "i locked it\n" );
			write( si->channelList[channelIndex].userList[i].socketDesc, toBcast, strlen(toBcast) );
			if( LOG )printf( "unlocking user %d\n", si->channelList[channelIndex].userList[i].index );
			pthread_mutex_unlock( &si->userLocks[si->channelList[channelIndex].userList[i].index] );
			if(LOG)printf( "i unlocked it\n" );
		}
	}
	//LOG = 0;
	return 1;
}

//this function features nested mutex locks: channelListLock -> channelLocks[i]
int handleClient_joinChannel( struct ServerInfo * si, char * msg, int userIndex )
{
	char * tok, serverListBuffer[BUFF_SIZE], response[BUFF_SIZE];
	memset( response, '\0', BUFF_SIZE );
	int i, j, ret = 0;
	strcpy( response, "JOINCHANNEL RESPONSE\n" );

	tok = strtok( msg, "\r\n" );
	pthread_mutex_lock( &si->channelListLock );
	if(LOG)printf( "Locking channel list\n" );
	for( i = 0; (i < MAX_CHANNELS)&&((!si->channelList[i].inUse) || (strcmp( si->channelList[i].name, tok)!=0)); i++ );
	if( i == MAX_CHANNELS )
	{
		pthread_mutex_unlock( &si->channelListLock );
		if(LOG)printf( "CHANNEL NOT FOUND\n" );
		strcat( response, "CHANNEL NOT FOUND\n" );
		if( sendLeader_getServerList( si, serverListBuffer ) ) {
			if(LOG)printf( "GOT SERVER LIST\n" );
			sprintf( response, "%s%s\n", response, serverListBuffer );
		}
		else {
			if(LOG)printf( "DID NOT GET SERVER LIST\n" );
			strcat( response, "ERROR GETTING SERVERLIST\n" );
		}
	} else {
		pthread_mutex_lock( &(si->channelLocks[i]) );
		if(LOG)printf( "Locking channel lock %d\n", i );
		pthread_mutex_unlock( &si->channelListLock );
		if(LOG)printf( "Unlocking channel list\n" );
		if( si->channelList[i].nusers >= MAX_USERS_PER_CHANNEL ) strcat( response, "CHANNEL FULL\n" );
		else {
			//see if we're already in the channel
			for( j = 0; (j < MAX_USERS_PER_CHANNEL) && (!(si->channelList[i].userList[j].inUse) || (strcmp(si->channelList[i].userList[j].name, si->userList[userIndex].name ) != 0 )); j++ );
			if( j < MAX_USERS_PER_CHANNEL )
			{
				strcat( response, "ALREADY IN CHANNEL\n" );
				pthread_mutex_unlock( &si->channelLocks[i] );
				if(LOG)printf( "Unlocking channel lock %d", i );
			} else {
				for( j = 0; (j < MAX_USERS_PER_CHANNEL) && (si->channelList[i].userList[j].inUse); j++ );
				si->channelList[i].userList[j] = si->userList[userIndex];
				si->channelList[i].nusers++;

				//add the channel to the users channelIndexes list
				for( j = 0; j < MAX_CHANNELS; j++ )
				{
					//the index being -1 means the element is 'empty'
					if( si->userList[userIndex].channelIndexes[j] == -1 )
					{
						if(LOG)printf( "Putting channel index %d into user index %d\n", j,userIndex);
						si->userList[userIndex].channelIndexes[j] = i;
						break;
					}
				}
				strcat( response, "SUCCESS\n" );
				pthread_mutex_unlock( &(si->channelLocks[i]) );
				if(LOG)printf( "Unlocking channe lock %d\n", i );
				sendChannel_userJoinedChannel( si, userIndex, i );
				if(LOG)printf( "sent broadcast\n" );
				ret = 1;
			}
		}
	}
	
	if(LOG)printf( "Locking user lock %d\n", userIndex );
	pthread_mutex_lock( &(si->userLocks[userIndex]) );
	if(LOG)printf( "I locked it\n" );
	write( (si->userList[userIndex].socketDesc), response, strlen(response) + 1 );
	if(LOG)printf ("Unlocking user lock %d\n", userIndex );
	pthread_mutex_unlock( &(si->userLocks[userIndex]) );
	if(LOG)printf ("I unlocked it\n" );
	return ret;
}

int handleClient_createChannel( struct ServerInfo * si, char * msg, int userIndex )
{
	char * tok, response[BUFF_SIZE];
	memset( response, '\0', BUFF_SIZE );
	int i, j, ret = 0;
	strcpy( response, "CREATECHANNEL RESPONSE\n" );

	if( si->nchannels >= MAX_CHANNELS ) strcat( response, "SERVER FULL\n" );
	else {
		tok = strtok( msg, "\r\n" );
		pthread_mutex_lock( &si->channelListLock );
		for( i = 0; (i < MAX_CHANNELS)&&((!si->channelList[i].inUse) || (strcmp( si->channelList[i].name, tok)!=0)); i++ );
		if( i != MAX_CHANNELS )
		{
			//channel already exists, just join it
			pthread_mutex_lock( &si->channelLocks[i] );
			pthread_mutex_unlock( &si->channelListLock );
			//check to see if we're already in it
			for( j = 0; (j < MAX_USERS_PER_CHANNEL) && (strcmp(si->channelList[i].userList[j].name, si->userList[userIndex].name ) != 0 ); j++ );
			if( j < MAX_USERS_PER_CHANNEL )
			{
				strcat( response, "ALREADY IN CHANNEL\n" );
				pthread_mutex_unlock( &si->channelLocks[i] );
			} else {
				for( j = 0; (j < MAX_USERS_PER_CHANNEL) && (si->channelList[i].userList[j].inUse); j++ );
				si->channelList[i].userList[j] = si->userList[userIndex];
				si->channelList[i].nusers++;
				//add the channel to the users channelIndexes list
				for( j = 0; j < MAX_CHANNELS; j++ )
				{
					//the index being -1 means the element is 'empty'
					if( si->userList[userIndex].channelIndexes[j] == -1 )
					{
						if(LOG)printf( "Putting channel index %d into user index %d\n", j, userIndex);
						si->userList[userIndex].channelIndexes[j] = i;
						break;
					}
				}
				pthread_mutex_unlock( &(si->channelLocks[i]) );
				strcat( response, "SUCCESS\n" );
				ret = 1;
			}
		} else {
			for( i = 0; (i < MAX_CHANNELS)&&(si->channelList[i].inUse); i++ );
			pthread_mutex_lock( &si->channelLocks[i] );
			si->channelList[i].inUse = 1;
			pthread_mutex_unlock( &si->channelListLock );
			si->channelList[i].userList[0] = si->userList[userIndex];
			si->channelList[i].nusers=1;
			si->channelList[i].index = i;
			for( j = 0; j < MAX_CHANNELS; j++ )
			{
				//the index being -1 means the element is 'empty'
				if( si->userList[userIndex].channelIndexes[j] == -1 )
				{
					if(LOG)printf( "Putting channel index %d into user index %d\n", j, userIndex);
					si->userList[userIndex].channelIndexes[j] = i;
					break;
				}
			}
			strcpy( si->channelList[i].name, tok );
			pthread_mutex_unlock( &(si->channelLocks[i]) );
			strcat( response, "SUCCESS\n" );
			si->nchannels++;
			ret = 1;
		}
	}
	if(LOG)printf( "I'm locking user %d\n", userIndex );
	pthread_mutex_lock( &(si->userLocks[userIndex]) );
	if(LOG)printf( "I locked it" );
	write( (si->userList[userIndex].socketDesc), response, strlen(response) + 1 );
	if(LOG)printf( "I'm unlocking user %d\n", userIndex );
	pthread_mutex_unlock( &(si->userLocks[userIndex]) );
	if(LOG)printf( "I unlocked it" );
	return ret;
}

int handleClient_quitUser( struct ServerInfo * si, char * msg, int userIndex )
{
	char response[BUFF_SIZE];
	memset( response, '\0', BUFF_SIZE );
	strcpy( response, "QUITUSER RESPONSE\n" );
	int i, j;
	//iterate through and disconnect from all the users channels
	for( i = 0; i < MAX_CHANNELS; i++ )
	{
		int cindex = si->userList[userIndex].channelIndexes[i];
		if( cindex != -1 )
		{
			if(LOG)printf( "Quitting channel at cindex %d\n", cindex );
			pthread_mutex_lock(&(si->channelLocks[cindex]));
			for( j = 0; (j < MAX_USERS_PER_CHANNEL) && (!(si->channelList[cindex].userList[j].inUse) || (strcmp( si->userList[userIndex].name, si->channelList[cindex].userList[j].name ) != 0)); j++  );
			if( j < MAX_USERS_PER_CHANNEL )
			{
				si->channelList[cindex].userList[j].inUse = 0;
				if( sendChannel_userLeftChannel( si, userIndex, cindex ) == 0 ) si->channelList[i].inUse = 0;
			}
			pthread_mutex_unlock( &(si->channelLocks[cindex]) );
		}
	}

	strcat( response, "SUCCESS\n" );
	if(LOG)printf( "I'm locking user %d\n", userIndex );
	pthread_mutex_lock( &si->userLocks[userIndex] );
	if(LOG)printf( "I locked it\n" );
	write( si->userList[userIndex].socketDesc, response, strlen(response)+1 );
	if(LOG)printf( "I'm unlocking user %d\n", userIndex );
	pthread_mutex_unlock( &si->userLocks[userIndex] );
	if(LOG)printf( "I unlocked it\n" );
	si->userList[userIndex].inUse = 0;
	return 1;
}

int handleClient_leaveChannel( struct ServerInfo * si, char * msg, int userIndex )
{
	char * tok, response[BUFF_SIZE];
	memset( response, '\0', BUFF_SIZE );
	int i, j, ret = 0;
	strcpy( response, "LEAVECHANNEL RESPONSE\n" );

	tok = strtok( msg, "\r\n" );
	pthread_mutex_lock( &si->channelListLock );
	for( i = 0; (i < MAX_CHANNELS)&&((!si->channelList[i].inUse) || (strcmp( si->channelList[i].name, tok)!=0)); i++ );
	if( i == MAX_CHANNELS )
	{
		pthread_mutex_unlock( &si->channelListLock );
		strcat( response, "CHANNEL NOT FOUND\n" );
	} else {
		pthread_mutex_lock( &(si->channelLocks[i]) );
		pthread_mutex_unlock( &si->channelListLock );
		for( j = 0; (j < MAX_USERS_PER_CHANNEL) && (si->channelList[i].userList[j].inUse) && (si->channelList[i].userList[j].index = userIndex); j++ );
		if( j == MAX_USERS_PER_CHANNEL ) strcat( response, "USER NOT IN CHANNEL\n" );
		else {
			printf( "User in channel %d at index %d is getting inUse reset\n", i, j );
			si->channelList[i].userList[j].inUse = 0;
			if ( sendChannel_userLeftChannel( si, userIndex, i ) == 0 ) si->channelList[i].inUse = 0;
			//remove the channel from the users channelIndexes list
			for(j = 0; j < MAX_CHANNELS; j++ )
			{
				if( si->userList[userIndex].channelIndexes[j] == i )
				{
					si->userList[userIndex].channelIndexes[j] = -1;
					break;
				}
			}
			strcat( response, "SUCCESS\n" );
			ret = 1;
		}
		pthread_mutex_unlock( &(si->channelLocks[i]) );
	}
	if(LOG)printf( "Locking user mutex %d\n", userIndex );
	pthread_mutex_lock( &(si->userLocks[userIndex]) );
	if(LOG)printf( "I locked it\n" );
	write( (si->userList[userIndex].socketDesc), response, strlen(response) + 1 );
	if(LOG)printf( "unLocking user mutex %d\n", userIndex );
	pthread_mutex_unlock( &(si->userLocks[userIndex]) );
	if(LOG)printf( "I unlocked it\n" );
	return ret;
}

int handleClient_channelBroadcast( struct ServerInfo * si, char * msg, int userIndex )
{
	printf( "MESSAGE TO BROADCAST: %s\n", msg );

	if(LOG)printf( "Got here 1\n" );
	int i, j, ret = 0;
	char * tok, * msgPtr;

	msgPtr = strchr( msg, '\n' ) + 1;
	tok = strtok( msg, "\r\n" );
	if(LOG)printf( "Got here 2\n" );
	pthread_mutex_lock( &si->channelListLock );
	for( i = 0; (i < MAX_CHANNELS) && ((!(si->channelList[i].inUse)) || (strcmp( si->channelList[i].name, tok ) != 0)); i++ );
	if( i == MAX_CHANNELS ) pthread_mutex_unlock( &si->channelListLock );
	else {
		if(LOG)printf( "Got here 3\n" );
		pthread_mutex_lock( &si->channelLocks[i] );
		if(LOG)printf( "locking on channel %d\n", i );
		pthread_mutex_unlock( &si->channelListLock );
		for( j = 0; (j < MAX_USERS_PER_CHANNEL) && (si->channelList[i].userList[j].inUse) && (si->channelList[i].userList[j].index = userIndex); j++ );
		if( j < MAX_USERS_PER_CHANNEL )
		{
			printf( "sendChannel_broadcastMsg" );
			sendChannel_broadcastMsg( si, userIndex, i, msgPtr );
			printf( "return sendChannel_broadcastMsg" );
			ret = 1;
		}
		pthread_mutex_unlock( &si->channelLocks[i] );
		if(LOG)printf( "unlocking on channel %d\n", i );
	}
	if(LOG)printf( "Got here 4\n" );
	return ret;
}

int msg_handleMsg( struct ServerInfo * si, char * msg, int msgType, int userIndex )
{
	int ret_val = 0;
	switch( msgType )
	{
		case ADDUSER:
			ret_val = handleClient_addUser( si, msg, userIndex );
			break;
		case JOINCHANNEL:
			ret_val = handleClient_joinChannel( si, msg, userIndex );
			break;
		case LEAVECHANNEL:
			ret_val = handleClient_leaveChannel( si, msg, userIndex );
			break;
		case CREATECHANNEL:
			ret_val = handleClient_createChannel( si, msg, userIndex );
			break;
		case BROADCAST:
			ret_val = handleClient_channelBroadcast( si, msg, userIndex );
			break;
		case QUITUSER:
			ret_val = handleClient_quitUser( si, msg, userIndex );
			break;
		case -1:
			if(LOG)printf( "got a weird message dude\n" );
			break;
	}
	return ret_val;
}

int msg_stripHeader( char * msg, char **msgPtr )
{
	char * tok;
	int msgType = -1;
	//change to '\n'
	*msgPtr = strchr( msg, '\n' ) + 1;
	tok = strtok( msg, "\n\r" );
	     if ( !strcmp( tok, "ADDUSER"       ) ) msgType = ADDUSER;
	else if ( !strcmp( tok, "JOINCHANNEL"   ) ) msgType = JOINCHANNEL;
	else if ( !strcmp( tok, "LEAVECHANNEL"  ) )	msgType = LEAVECHANNEL;
	else if ( !strcmp( tok, "CREATECHANNEL" ) )	msgType = CREATECHANNEL;
	else if ( !strcmp( tok, "BROADCAST"     ) )	msgType = BROADCAST;
	else if ( !strcmp( tok, "QUITUSER"      ) )	msgType = QUITUSER;
	return msgType;
}
