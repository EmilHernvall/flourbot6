#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "util.h"
#include "hashtable.h"

void hashtable_init(hashtable* hashtable, int size, double loadfactor) 
{
	hashtable->data = (hashtable_bucket**)xmalloc(sizeof(hashtable_bucket*) * size);
	hashtable->loadfactor = loadfactor;
	hashtable->size = size;
	hashtable->count = 0;

	int i = 0;
	for (i = 0; i < size; i++) {
		hashtable->data[i] = NULL;
	}
}

int hashtable_count(hashtable* hashtable)
{
	return hashtable->count;
}

void hashtable_rehash(hashtable* ht, int newsize)
{
	hashtable newtable;
	hashtable_init(&newtable, newsize, ht->loadfactor);

	int i = 0;
	for (i = 0; i < ht->size; i++) {
		hashtable_bucket* bucket = ht->data[i];
		if (bucket == NULL) {
			continue;
		}

		int j = 0;
		for (j = 0; j < bucket->count; j++) {
			if (bucket->key[j] == NULL) {
				continue;
			}

//			printf("reinserting %s\n", bucket->key[j]);
			hashtable_insert(&newtable, bucket->key[j], (void*)bucket->data[j]);
			xfree(bucket->key[j]);
		}

		xfree(bucket->key);
		xfree(bucket->data);
		xfree(bucket);
	}

	xfree(ht->data);

	memcpy(ht, &newtable, sizeof(hashtable));
}

int hashtable_search(hashtable* hashtable, char* key) 
{
	assert(key != NULL);

	int bucket_pos = 0;
	int i = 0;
	int len = strlen(key);
	for (i = 0; i < len; i++) {
		bucket_pos += key[i] * pow(HASHTABLE_HASH_A, len - i);
	}

	bucket_pos = abs(HASHTABLE_COMPRESSION_A * bucket_pos + HASHTABLE_COMPRESSION_B) % hashtable->size;

	return bucket_pos;
}

void hashtable_insert(hashtable* hashtable, char* key, void* data) 
{
//	printf("Load factor: %0.2f\n", (double)hashtable->count/(double)hashtable->size);
	if ((double)hashtable->count/(double)hashtable->size > hashtable->loadfactor) {
		#if DEBUG
			printf("Rehashing from %d to %d.\n", hashtable->size, 2*hashtable->size);
		#endif
		hashtable_rehash(hashtable, 2*hashtable->size);
	}

	assert(key != NULL);

	int bucket_pos = hashtable_search(hashtable, key);

	hashtable_bucket* cur;
	
	if (hashtable->data[bucket_pos] == NULL) {
		cur = (hashtable_bucket*)xmalloc(sizeof(hashtable_bucket));

		hashtable->data[bucket_pos] = cur;

		cur->size = HASHTABLE_BUCKET_INITIALSIZE;
		cur->count = 0;

		cur->key = (char**)xmalloc(sizeof(char*) * HASHTABLE_BUCKET_INITIALSIZE);
		cur->data = (void**)xmalloc(sizeof(void*) * HASHTABLE_BUCKET_INITIALSIZE);

		int i = 0;
		for (i = 0; i < HASHTABLE_BUCKET_INITIALSIZE; i++) {
			cur->key[i] = NULL;
			cur->data[i] = NULL;
		}
	} else {
		cur = hashtable->data[bucket_pos];

		if (cur->count == cur->size - 1) {
			int newsize = 2 * cur->size;

			char** newkey = (char**)xmalloc(sizeof(char*) * newsize);
			void** newdata = (void**)xmalloc(sizeof(void*) * newsize);

			int i = 0;
			for (i = 0; i < newsize; i++) {
				if (i < cur->count) {
					newkey[i] = cur->key[i];
					newdata[i] = cur->data[i];
				} else {
					newkey[i] = NULL;
					newdata[i] = NULL;
				}
			}

			xfree(cur->key);
			xfree(cur->data);

			cur->key = newkey;
			cur->data = newdata;

			cur->size = newsize;
		}
	}

	char* nkey = strdup(key);

	int i = 0;
	for (i = 0; i < cur->size; i++) {
		if (cur->key[i] == NULL) {
			cur->key[cur->count] = nkey;
			cur->data[cur->count] = data;
			break;
		}
	}

	cur->count++;
	hashtable->count++;
}

void* hashtable_get(hashtable* hashtable, char* key) 
{
	if (key == NULL) {
		return NULL;
	}

	int bucket_pos = hashtable_search(hashtable, key);

	if (hashtable->data[bucket_pos] == NULL) {
		return NULL;
	}

	hashtable_bucket* cur = hashtable->data[bucket_pos];

	int i = 0;
	void* hit = NULL;
	for (i = 0; i < cur->count; i++) {
		if (cur->key[i] == NULL) {
			continue;
		}

		if (strcmp(cur->key[i], key) == 0) {
			hit = cur->data[i];
			break;
		}
	}

	return hit;
}

void* hashtable_delete(hashtable* hashtable, char* key) 
{
	assert(key != NULL);

	void* data;

	int bucket_pos = hashtable_search(hashtable, key);
	hashtable_bucket* cur = hashtable->data[bucket_pos];
//	printf("%s %d\n", key, bucket_pos);

	if (cur == NULL) {
		return NULL;
	}

	int i = 0, j = 0, pos = -1;
	for (i = 0; i < cur->size; i++) {
		if (cur->key[i] == NULL) {
			continue;
		}
		
		if (strcmp(cur->key[i], key) == 0) {
			pos = i;
			break;
		}
	}

	if (pos == -1) {
		return NULL;
	}

	if (cur->count == 1) {
		hashtable->data[bucket_pos] = NULL;
		hashtable->count--;

		xfree(cur->key[0]);
		data = cur->data[0];

		xfree(cur->key);
		xfree(cur->data);
		xfree(cur);

		return data;
	}

	// Shrink buffer
	if (floor((double)cur->size/2) > cur->count) {
	
		int newsize =  floor((double)cur->size/2);
	//	printf("Count: %d, Previous size: %d, New size: %d\n", cur->count, cur->size, newsize);

		char** newkey = (char**)xmalloc(sizeof(char*) * newsize);
		void** newdata = (void**)xmalloc(sizeof(void*) * newsize);

		memset(newkey, (int)NULL, sizeof(char*) * newsize);
		memset(newdata, (int)NULL, sizeof(void*) * newsize);

		for (i = 0; i < cur->size; i++) {
			if (cur->key[i] == NULL) {
				continue;
			}
		
			if (strcmp(cur->key[i], key) != 0) {
				newkey[j] = cur->key[i];
				newdata[j] = cur->data[i];
				j++;
			} else {
				xfree(cur->key[i]);
				data = cur->data[i];
			}
		}

		xfree(cur->key);
		xfree(cur->data);

		cur->key = newkey;
		cur->data = newdata;

		cur->size = newsize;
	}
	else {
		xfree(cur->key[pos]);
		data = cur->data[pos];

		cur->key[pos] = NULL;
		cur->data[pos] = NULL;
	}

	cur->count--;
	hashtable->count--;

	return data;
}

void hashtable_free(hashtable* ht, void (*cleanupfunc)(void*))
{
	int i = 0;
	for (i = 0; i < ht->size; i++) {
		hashtable_bucket* bucket = ht->data[i];
		if (bucket == NULL) {
			continue;
		}

		int j = 0;
		for (j = bucket->count; j >= 0; j--) {
			if (bucket->key[j] == NULL) {
				continue;
			}

//			printf("deleting %s\n", bucket->key[j]);

			void* data = hashtable_delete(ht, bucket->key[j]);
			if (cleanupfunc != NULL) {
				cleanupfunc(data);
			}
		}
	}

	xfree(ht->data);
}

#ifdef TEST

typedef struct _user {
	char* firstname;
	char* surname;
	char* pnr;
} user;

int main(int argc, char** argv) 
{
	hashtable mytable;
	hashtable_init(&mytable, 1001, 0.75);

	user users[] = { 
		{ "emil", "hernvall", "861110-5555" },
		{ "hannes", "hernvall", "whatever" },
		{ "lydia", "hernvall", "foobar" } };

	int i;
	for (i = 0; i < 3; i++) {
		hashtable_insert(&mytable, users[i].firstname, &users[i]);
	}

	printf("%d\n", hashtable_delete(&mytable, "erik"));

	for (i = 0; i < 3; i++) {
		user* u = hashtable_get(&mytable, users[i].firstname);
		printf("%s %s %s\n", u->firstname, u->surname, u->pnr);
	}

	return 0;
}
#endif

