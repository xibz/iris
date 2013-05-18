#include <pthread.h>

#define MSG_UNKNOWN 	-1
#define MSG_ADDSERVER 	0
#define MSG_RMVSERVER 	1
#define MSG_GETSERVERS 	2

#define NAME_SIZE 	100
#define BUFF_SIZE 	1024
#define MAX_SERVERS 10

struct Server{
	char hostname[NAME_SIZE];
	int port;
	int inUse;
};

struct LeaderInfo{
	struct Server serverList[MAX_SERVERS];
	pthread_mutex_t serverLocks[MAX_SERVERS];
	pthread_mutex_t serverListLock;
	int nservers;
};
struct LeaderInfo * leader_genLeaderInfo();
int msg_stripHeader( char * msg, char **msgPtr );
int msg_handleMsg( struct LeaderInfo * li, char * msg, int msgType, int connfd );
