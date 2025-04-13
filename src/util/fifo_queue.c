#include "fifo_queue.h"

void fifoQueuePush(Queue *pQueue, void *data)
{
    if(!pQueue|| !data)
        return;

    MUTEX_LOCK(&pQueue->lock);

    queue_elmt_t *pElem = (queue_elmt_t *)malloc(sizeof(queue_elmt_t));
    if(!pElem) {
        MUTEX_UNLOCK(&pQueue->lock);
        return;
    }

    pElem->data = data;
    pElem->next = NULL;

    if(!pQueue->first_elmt)
        pQueue->first_elmt = pElem;
    else
        pQueue->last_elmt->next = pElem;
    pQueue->last_elmt = pElem;

    COND_SIGNAL(&pQueue->signal);
    MUTEX_UNLOCK(&pQueue->lock);
    return;
}

void *fifoQueuePop(Queue *pQueue)
{
    if(!pQueue)
        return NULL;

    void *pData = NULL;

    MUTEX_LOCK(&pQueue->lock);

    while(!pQueue->first_elmt) {
        COND_WAIT(&pQueue->signal, &pQueue->lock);
    }

    queue_elmt_t *pElem = pQueue->first_elmt;
    pQueue->first_elmt = pElem->next;
    if(!pElem->next)
        pQueue->last_elmt = NULL;

    pData = pElem->data;

    FREE(pElem);

    MUTEX_UNLOCK(&pQueue->lock);

    return pData;
}

void *fifoQueuePopUnblock(Queue *pQueue)
{
    if (!pQueue)
        return NULL;

    void *pData = NULL;

    MUTEX_LOCK(&pQueue->lock);

    if(!pQueue->first_elmt) {
        MUTEX_UNLOCK(&pQueue->lock);
        return NULL;
    }

    queue_elmt_t *pElem = pQueue->first_elmt;
    pQueue->first_elmt = pElem->next;
    if(!pElem->next)
        pQueue->last_elmt = NULL;

    pData = pElem->data;

    FREE(pElem);

    MUTEX_UNLOCK(&pQueue->lock);

    return pData;
}

Queue *createFifoQueue(void)
{
    Queue *pQueue = (Queue *)malloc(sizeof(Queue));
    if (!pQueue)
        return NULL;

    pQueue->first_elmt = NULL;
    pQueue->last_elmt = NULL;

    MUTEX_INIT(&pQueue->lock);
    COND_INIT(&pQueue->signal);

    return pQueue;
}

void releaseFifoQueue(Queue *pQueue)
{
    if (!pQueue)
        return;

    while (1) {
        void *pData = fifoQueuePopUnblock(pQueue);
        if (!pData)
            break;
        FREE(pData);
    }

    COND_DESTROY(&pQueue->signal);
    MUTEX_DESTROY(&pQueue->lock);

    FREE(pQueue);
}