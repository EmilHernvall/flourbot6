#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "stack.h"

void stack_init(stack* mystack) 
{
	assert(mystack != NULL);

	mystack->size = 0;
	mystack->first = NULL;
	mystack->last = NULL;

	pthread_mutex_init(&mystack->mutex, NULL);
}

void stack_free(stack* mystack)
{
	assert(mystack != NULL);

	pthread_mutex_destroy(&mystack->mutex);
}

int stack_size(stack* mystack)
{
	assert(mystack != NULL);
	return mystack->size;
}

void stack_push(stack* mystack, void* data) 
{
	assert(mystack != NULL);
	assert(data != NULL);

	pthread_mutex_lock(&mystack->mutex);

	stack_item* myitem = (stack_item*)xmalloc(sizeof(stack_item));
	myitem->data = data;
	myitem->prev = NULL;
	myitem->next = NULL;

	if (mystack->first == NULL) {
		mystack->first = myitem;
	}

	if (mystack->last == NULL) {
		mystack->last = myitem;
	} else {
		stack_item* last = mystack->last;
		last->next = myitem;
		myitem->prev = last;
		mystack->last = myitem;
	}

	mystack->size++;

	pthread_mutex_unlock(&mystack->mutex);
}

void* stack_pop(stack* mystack) 
{
	assert(mystack != NULL);

	pthread_mutex_lock(&mystack->mutex);

	if (mystack->size == 0) {
		return NULL;
	}

	stack_item* myitem = mystack->last;

	if (myitem->prev != NULL) {
		stack_item* last = myitem->prev;
		mystack->last = last;
		last->next = NULL;
	}

	if (mystack->size == 1) {
		mystack->first = NULL;
		mystack->last = NULL;
	}

	void* data = myitem->data;
	xfree(myitem);
	myitem = NULL;

	mystack->size--;

	pthread_mutex_unlock(&mystack->mutex);

	return data;
}

void* stack_peek(stack* mystack) 
{
	assert(mystack != NULL);

	pthread_mutex_lock(&mystack->mutex);

	if (mystack->size == 0) {
		return NULL;
	}

	stack_item* myitem = mystack->first;

	if (myitem->next != NULL) {
		stack_item* first = myitem->next;
		mystack->first = first;
		first->prev = NULL;
	}

	if (mystack->size == 1) {
		mystack->first = NULL;
		mystack->last = NULL;
	}

	void* data = myitem->data;

	xfree(myitem);
	myitem = NULL;

	mystack->size--;

	pthread_mutex_unlock(&mystack->mutex);

	return data;
}

void stack_output(stack* mystack) 
{
	printf("Size: %d\n", mystack->size);

	printf("Looping forward\n");
	stack_item* j;
	for (j = mystack->first; j != NULL; j = j->next) {
		printf("\t%s\n", j->data);
	}

	printf("Looping backward\n");
	for (j = mystack->last; j != NULL; j = j->prev) {
		printf("\t%s\n", j->data);
	}

	printf("Popping\n");
	void* i;
	while (i = stack_pop(mystack)) {
		printf("\t%s\n", i);
	}

	printf("Size: %d\n", mystack->size);
}
