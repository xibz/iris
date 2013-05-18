#include <pthread.h>

#define ADDUSER 		1
#define JOINCHANNEL 	2
#define CREATECHANNEL 	3
#define LEAVECHANNEL 	4
#define BROADCAST 		5
#define QUITUSER 		6

#define NAME_SIZE 				100
#define MAX_CHANNELS 			10
#define MAX_USERS_PER_CHANNEL 	10
#define BUFF_SIZE 				1024
#define MAX_USERS 				100

struct User
{
	int inUse;
	int index;
	int socketDesc;
	char name[NAME_SIZE];
	int channelIndexes[MAX_CHANNELS];
};

struct Channel
{
	int inUse;
	int index;
	char name[NAME_SIZE];
	struct User userList[MAX_USERS_PER_CHANNEL];
	int nusers;
	pthread_mutex_t userListLock;
};

struct Leader
{
	int socketDesc;
	int port;
	char hostname[NAME_SIZE];
};

struct ServerInfo
{
	char hostname[NAME_SIZE];
	int port;

	struct Leader myLeader;
	struct User userList[MAX_CHANNELS*MAX_USERS_PER_CHANNEL];
	pthread_mutex_t userLocks[MAX_CHANNELS*MAX_USERS_PER_CHANNEL];
	pthread_mutex_t userListLock;
	int nusers;

	struct Channel channelList[MAX_CHANNELS];
	pthread_mutex_t channelLocks[MAX_USERS_PER_CHANNEL];
	pthread_mutex_t channelListLock;
	int nchannels;

	pthread_mutex_t leaderLock;
};

struct ServerInfo * genServerInfo(char *, int, char *, int, int);
int sendLeader_addServer(struct ServerInfo *);
int msg_handleMsg(struct ServerInfo *, char *, int, int);
int msg_stripHeader(char *, char **);
