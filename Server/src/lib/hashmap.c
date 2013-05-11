#include<stdio.h>
#include<stdint.h>
#include<stddef.h>
#include<stdlib.h>
#include<string.h>

#define PROBING_CONSTANT1 0.5
#define PROBING_CONSTANT2 0.5

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t SuperFastHash (const char * data, int len) {
	uint32_t hash = len, tmp;
	int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}


struct Entry{
	void * f_entry;
	char f_key[32];
};

struct HashMap{
	struct Entry ** f_entryBucket;
	int f_size;
};

int put( struct HashMap * p_hm, char * p_key, void * p_entry ){
	uint32_t l_hashValue = SuperFastHash( p_key, strlen( p_key ) );
	uint32_t l_i, l_targetIndex;

	for( l_i = 0; l_i < p_hm->f_size; l_i++ ){
		l_targetIndex = (uint32_t)(l_hashValue + ((double)(l_i)*PROBING_CONSTANT1) + ((double)(l_i)*(double)(l_i)*PROBING_CONSTANT2)) % p_hm->f_size;
		if( p_hm->f_entryBucket[ l_targetIndex ] == NULL )
			break;
	}

	if( l_i >= p_hm->f_size )
		return 0;


	struct Entry * newEntry = (struct Entry *) malloc( sizeof( struct Entry ) );
	strcpy( newEntry->f_key, p_key );
	newEntry->f_entry = p_entry;
	p_hm->f_entryBucket[ l_targetIndex ] = newEntry;
	return 1;
}

void * get( struct HashMap * p_hm, char * p_key){
	uint32_t l_hashValue = SuperFastHash( p_key, strlen(p_key) );
	uint32_t l_i, l_targetIndex;

	for( l_i = 0; l_i < p_hm->f_size; l_i++ ){
		l_targetIndex = (uint32_t)((l_hashValue) + ((double)(l_i)*PROBING_CONSTANT1) + ((double)(l_i)*(double)(l_i)*PROBING_CONSTANT2)) % p_hm->f_size;
		if( (p_hm->f_entryBucket[ l_targetIndex ] != NULL) && (strcmp( p_hm->f_entryBucket[l_targetIndex]->f_key, p_key ) == 0 ) )
			break;
	}

	if( l_i >= p_hm->f_size )
		return NULL;

	return p_hm->f_entryBucket[ l_targetIndex ]->f_entry;
}

void deleteFrom( struct HashMap * p_hm, char * p_key ){
	uint32_t l_hashValue = SuperFastHash( p_key, strlen(p_key) );
	uint32_t l_i, l_targetIndex;

	for( l_i = 0; l_i < p_hm->f_size; l_i++ ){
		l_targetIndex = (uint32_t)((l_hashValue) + ((double)(l_i)*PROBING_CONSTANT1) + ((double)(l_i)*(double)(l_i)*PROBING_CONSTANT2)) % p_hm->f_size;
		if( (p_hm->f_entryBucket[ l_targetIndex ] != NULL) && (strcmp( p_hm->f_entryBucket[l_targetIndex]->f_key, p_key ) == 0 ) )
			break;
	}

	if( l_i >= p_hm->f_size )
		return;

	free( p_hm->f_entryBucket[ l_targetIndex ]->f_entry );
	free( p_hm->f_entryBucket[ l_targetIndex ] );
}

void ** toArray( struct HashMap * p_hm, int * n ){
	void ** ret = (void **) malloc(sizeof(void *) * p_hm->f_size);
	int i;
	*n = p_hm->f_size;
	for( i = 0; i < *n; i++ ){
		if( p_hm->f_entryBucket[i] == NULL )
			ret[i] = NULL;
		else
			ret[i] = p_hm->f_entryBucket[i]->f_entry;
	}
	return ret;
}

struct HashMap * genHashMap( int p_bucketSize ){
	if( p_bucketSize <= 0 )
		return NULL;

	struct HashMap * l_hm = ( struct HashMap * ) malloc(sizeof( struct HashMap ) );
	l_hm->f_entryBucket = ( struct Entry ** ) malloc(sizeof( struct Entry * ) * p_bucketSize);
	l_hm->f_size = p_bucketSize;

	int i;
	for( i = 0; i < l_hm->f_size; i++ )
		l_hm->f_entryBucket[i] = NULL;

	return l_hm;
}

void deleteHashMap( struct HashMap * p_hm, int p_freeEntries ){
	int l_i;
	if( p_freeEntries ){
		for( l_i = 0; l_i < p_hm->f_size; l_i++ ){
			if( p_hm->f_entryBucket[l_i] != NULL ){
				free( p_hm->f_entryBucket[l_i] );
			}
		}
	}
	free( p_hm->f_entryBucket );
	free( p_hm );
	return ;
}
