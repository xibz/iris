struct ListByName;
struct ListEntry;

struct ListByName * genListByName(int size);
int addToList( struct ListByName * myList, char * name, void * entry );
int removeFromListByName( struct ListByName * myList, char * name );
int removeFromListByIndex( struct ListByName * myList, int i );
int binarySearchList( struct ListByName * myList, char * name, int start, int end, int * index );
void destroyListByName( struct ListByName * myList );
