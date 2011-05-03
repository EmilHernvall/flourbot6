#ifndef __STACK_H_
#define __STACK_H_

#include <pthread.h>

typedef struct _item {
        struct _item* prev;
        void* data;
        struct _item* next;
} stack_item;

typedef struct _stack {
        stack_item* first;
        stack_item* last;
		pthread_mutex_t mutex;
        int size;
} stack;

void stack_init(stack* mystack);
void stack_free(stack* mystack);
int stack_size(stack* mystack);
void stack_push(stack* mystack, void* data);
void stack_output(stack* mystack);
void* stack_pop(stack* mystack);
void* stack_peek(stack* mystack);

#endif
