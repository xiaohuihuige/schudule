#ifndef _fifo_queue_h_
#define _fifo_queue_h_
#ifdef __cplusplus
extern "C" {
#endif

#include "pthread.h"
#include "net-common.h"

typedef struct _queue_elmt_t
{
	void *  data;
	struct _queue_elmt_t * next;
} queue_elmt_t;

typedef struct 
{
	queue_elmt_t *first_elmt;
	queue_elmt_t *last_elmt;

	Mutex lock;
	Cond signal;
} Queue;

Queue *createFifoQueue(void);
void releaseFifoQueue(Queue *pQueue);

void fifoQueuePush(Queue *pQueue, void *data);
void *fifoQueuePop(Queue *pQueue);
void *fifoQueuePopUnblock(Queue *pQueue);

#ifdef __cplusplus
}
#endif
#endif

