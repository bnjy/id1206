#ifndef _QUEUE
#define _QUEUE

#include "green.h"

struct queue_t{
	struct green_t *head;
	struct green_t *tail;
	int length;
};

struct queue_t *init_queue(void);
void queue_insert(struct queue_t *queue, struct green_t *thread);
struct green_t *queue_remove(struct queue_t *queue);

#endif
