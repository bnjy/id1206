#include <ucontext.h>

typedef struct green_t {
	ucontext_t *context;
	void *(*fun)(void*);
	void *arg;
	struct green_t *next;
	struct green_t *join;
	void *retval;
	int zombie; //indicates if the thread has terminated or not
} green_t;

int green_create(green_t *thread, void *(*fun)(void*), void *args);
int green_yield();
int green_join(green_t *thread, void** val);
