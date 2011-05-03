#ifndef HASHTABLE_H__
#define HASHTABLE_H__

#define HASHTABLE_HASH_A 41
#define HASHTABLE_COMPRESSION_A 77
#define HASHTABLE_COMPRESSION_B 42
#define HASHTABLE_BUCKET_INITIALSIZE 2

typedef struct _hashtable_bucket {
	char** key;
	void** data;
	int size;
	int count;
} hashtable_bucket;

typedef struct _hashtable {
	hashtable_bucket** data;
	double loadfactor;
	int size;
	int count;
} hashtable;

void hashtable_init(hashtable* hashtable, int size, double loadfactor);
int hashtable_count(hashtable* hashtable);
void hashtable_rehash(hashtable* hashtable, int newsize);
int hashtable_search(hashtable* hashtable, char* key);
void hashtable_insert(hashtable* hashtable, char* key, void* data);
void* hashtable_get(hashtable* hashtable, char* key);
void* hashtable_delete(hashtable* hashtable, char* key);
void hashtable_free(hashtable* hashtable, void (*cleanupfunc)(void*));

#endif
