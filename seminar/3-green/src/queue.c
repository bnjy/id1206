#include "green.h"
#include "queue.h"
#include <stdlib.h>

struct queue_t *init_queue(){
	
	struct queue_t *queue = (struct queue_t*)(malloc(sizeof(struct queue_t)));
	queue->head = NULL;
	queue->tail = NULL;
	return queue;
}

void queue_insert(struct queue_t *queue, green_t *new){
	/* If queue is empty, set both head and tail to single thread */
	if(queue->tail == NULL){
		queue->head = new;
		queue->tail = new;
	}else{ // else place thread in front of the queue
		queue->tail->next = new;
		queue->tail = new;
	}
}

struct green_t *queue_remove(struct queue_t *queue){
	/* Empty queue, return null. */
	if(queue->head == NULL){
		return NULL;
	}
	
	green_t *thread = queue->head;
	
	queue->head = queue->head->next;
	
	if(queue->head == NULL){
		queue->tail = NULL;
	}
	
	return thread;
	
	/*
	else{ // else init new thread "thread" and point to head.
		green_t *thread = queue->head;
		if(queue->head == queue->tail){ // if only one element in queue, dereference tail
			queue->tail = NULL;
		}
	queue->head = queue->head->next;
	thread->next = NULL;
	return thread;
	}
	*/
}


