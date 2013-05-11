#define CLIENT_ADDUSER 0

struct ServerInfo;
struct ServerInfo * genServerInfo( char * myHostname,
									int myPort,
									char * leaderHostname,
									int leaderPort,
									int leaderMode,
									int independentMode );
void destroyServerInfo( struct ServerInfo * si );
int stripHeader( char * rcvBuff, char ** msgPtr );
void handleMessage( struct ServerInfo * si, int msgType, char * msgPtr, char * responseBuff, int responseBuffSize );
