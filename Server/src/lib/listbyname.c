#include<listbyname.h>
#include<stdio.h>

#define NAME_SIZE 32

struct ListByName{
	int size;
	int numEntries;
	struct Entry * entriesArray;
};

struct Entry{
	char name[32];
	void * data;
};

struct ListByName * genListByName( int size ){
	struct ListByName * myList = malloc( sizeof(struct ListByName) );
	myList->size = size;
	myList->numEntries = 0;
	myList->entriesArray = malloc( sizeof( struct Entry ) * size );
	int i;
	for( i = 0; i < size; i++ )
		myList->entriesArray[i].data = NULL;
	return myList;
}


int addToList( struct ListByName * myList, char * name, void * entry ){
	int i;
	if( myList->numEntries == myList->size )
		return 0;
	if( binarySearchList( myList, name, 0, myList->numEntries-1, &i ) ){
		return -1;
	}
	else{
		int j;
		for( j = myList->numEntries-1; j >= i; j-- )
			myList->entriesArray[j+1] = myList->entriesArray[j];

		strcpy( myList->entriesArray[i].name, name );
		myList->entriesArray[i].data = entry;
		myList->numEntries++;
	}
	return 1;
}

int removeFromListByIndex( struct ListByName * myList, int i ){
	int j;
	free( myList->entriesArray[i].data );
	for( j = i; j < myList->numEntries; j++ )
		myList->entriesArray[j] = myList->entriesArray[j+1];
	return 1;
}

int binarySearchList( struct ListByName * myList, char * name, int start, int end, int * index ){
	//If the list is empty, end will be less than start
	if( end < start ){
		*index = 0;
		return 0;
	}

	int i = start + (start-end)/2;
	int comp = strcoll( myList->entriesArray[i].name, name );
	if( start == end ) {
		if( comp > 0 )
			*index =  (i-1 > 0) ? i-1 : 0;
		else if( comp < 0 )
			*index =  i+1;
		return 0;
	}
	else if( comp == 0 ){
		*index = i;
		return 1;
	} 
	else if( comp > 0 )
		return binarySearchList( myList, name, start, i-1, index );
	else if( comp < 0 )
		return binarySearchList( myList, name, i+1, end, index );
}

int removeFromListByName( struct ListByName * myList, char * name ){
	int i;
	if( (binarySearchList( myList, name, 0, myList->numEntries-1, &i) ) )
		removeFromListByIndex(myList, i);
	return;
}

void destroyListByName( struct ListByName * myList ){
	int i;
	for( i = 0; i < myList->numEntries; i++ ){
		free( myList->entriesArray[i].data );
	}
	free( myList );
}
