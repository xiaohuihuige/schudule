#ifndef __NET_TASK_H__
#define __NET_TASK_H__

#include "net-common.h"

typedef struct
{
    void *task;
    struct list_head list;
} FifoQueue;

#define deleteFifoQueueTask(head, type)     \
    do {                                    \
        if (head) {                         \
            list_del(&head->list);          \
            if ((type *)head->task)         \
            {                               \
                net_free(head->task);       \
            }                               \
            FREE(head);                      \
        }                                   \
    } while (0);


#define destroyFifoQueue(head, type)                                                        \
    do {                                                                                    \
        if (head) {                                                                         \
            FifoQueue *task_pos = NULL;                                                         \
            FifoQueue *temp_pos = NULL;                                                         \
            list_for_each_entry_safe(task_pos, temp_pos, &((FifoQueue *)head)->list, list)      \
            {                                                                                   \
                deleteFifoQueueTask(task_pos, type);                                            \
            }                                                                                   \
            FREE(head);                                                                         \
        }                                                                                       \
    } while (0);


#define destroyFifoQueueTask(head, type)                                                        \
    do {                                                                                        \
        if (head) {                                                                             \
            FifoQueue *task_pos = NULL;                                                         \
            FifoQueue *temp_pos = NULL;                                                         \
            list_for_each_entry_safe(task_pos, temp_pos, &((FifoQueue *)head)->list, list)      \
            {                                                                                   \
                list_del(&task_pos->list);                                                       \
                FREE(task_pos);                                                                 \
            }                                                                                   \
        }                                                                                       \
    } while (0);

#define findFifoQueueTask(head, head_pos, type, find_task)                  \
    do {                                                                    \
        if (head) {                                                         \
            FifoQueue *temp_pos = NULL;                                     \
            list_for_each_entry_safe(head_pos, temp_pos, &((FifoQueue *)head)->list, list) \
            {                                                               \
                if (head_pos->task == find_task)                            \
                    break;                                                  \
            }                                                               \
        }                                                                   \
    } while (0);

#define FindDeleteFifoQueueTask(head, type, del_task)                       \
    do {                                                                    \
        if (head) {                                                         \
            FifoQueue *del_pos = NULL;                                      \
            findFifoQueueTask(head, del_pos, type, del_task);               \
            if (del_pos && del_pos->task == del_task)                       \
            {                                                               \
                list_del(&del_pos->list);                                   \
                FREE(del_pos);                                              \
            }                                                               \
        }                                                                   \
    } while (0);


static inline FifoQueue *createFifiQueue(void)
{
    FifoQueue *head = (FifoQueue *)calloc(1, sizeof(FifoQueue));
    if (!head)
    {
        ERR("malloc failed");
        return NULL;
    }

    INIT_LIST_HEAD(&head->list);
    return head;
}

static inline FifoQueue *enqueue(FifoQueue *head, void *task)
{
    FifoQueue *new_task = (FifoQueue *)calloc(1, sizeof(FifoQueue));
    if (!new_task)
    {
        ERR("malloc failed");
        return NULL;
    }

    new_task->task = task;

    list_add_tail(&new_task->list, &head->list);

    return new_task;
}

static inline FifoQueue *dequeue(FifoQueue *head)
{   
    if (list_empty(&head->list))
        return NULL;
        
    FifoQueue *task_node = list_first_entry(&head->list, FifoQueue, list);
    if (!task_node)
    {
        return NULL;
    }

    list_del(&task_node->list);

    return task_node;
}

#endif // !__NET_TASK_H__
