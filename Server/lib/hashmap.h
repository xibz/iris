struct HashMap;
struct Entry;

//Insert an entry
int put( struct HashMap * p_hm, char * p_key, void * p_entry );

//Get an entry by it's key
void * get( struct HashMap * p_hm, char * p_key);

//Make a new hashmap
struct HashMap * genHashMap( int p_bucketSize );

//Get an array of all the entries, for manual memory management
void ** toArray( struct HashMap * p_hm, int * n );

//Destroy a hashmap
void deleteHashMap( struct HashMap * p_hm, int p_freeEntries );
