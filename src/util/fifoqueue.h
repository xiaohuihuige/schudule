#ifndef __NET_TASK_H__
#define __NET_TASK_H__

#include "net-common.h"

typedef struct
{
    void *task;
    struct list_head list;
} task_list;

#define net_task_list_del(head, type) \
    {                                 \
        list_del(&head->list);        \
        if ((type *)head->task)       \
        {                             \
            net_free(head->task)      \
        }                             \
        net_free(head)                \
    }

#define net_task_list_each_free(head, type)                             \
    {                                                                   \
        task_list *task_pos = NULL;                                     \
        task_list *temp_pos = NULL;                                      \
        list_for_each_entry_safe(task_pos, temp_pos, &((task_list *)head)->list, list) \
        {                                                               \
            net_task_list_del(task_pos, type);                          \
        }                                                               \
        net_free(head)                                                  \
    }

#define net_task_list_find(head, head_pos, type, find_task)             \
    {                                                                   \
        task_list *temp_pos = NULL;                                     \
        list_for_each_entry_safe(head_pos, temp_pos, &((task_list *)head)->list, list) \
        {                                                               \
            if (head_pos->task == find_task)                            \
                break;                                                  \
        }                                                               \
    }

#define net_task_list_find_del(head, type, del_task)       \
    {                                                      \
        task_list *del_pos = NULL;                         \
        net_task_list_find(head, del_pos, type, del_task); \
        if (del_pos && del_pos->task == del_task)          \
        {                                                  \
            net_task_list_del(del_pos, type)               \
        }                                                  \
    }


static inline task_list *createFifiQueue(void)
{
    task_list *head = (task_list *)calloc(1, sizeof(task_list));
    if (!head)
    {
        ERR("malloc failed");
        return NULL;
    }

    INIT_LIST_HEAD(&head->list);
    return head;
}

static inline task_list *enqueue(task_list *head, void *task)
{
    task_list *new_task = (task_list *)calloc(1, sizeof(task_list));
    if (!new_task)
    {
        ERR("malloc failed");
        return NULL;
    }

    new_task->task = task;

    list_add_tail(&new_task->list, &head->list);

    return new_task;
}

static inline task_list *dequeue(task_list *head)
{   
    if (list_empty(&head->list))
        return NULL;
        
    task_list *task_node = list_first_entry(&head->list, task_list, list);
    if (!task_node)
    {
        return NULL;
    }

    list_del(&task_node->list);

    return task_node;
}

#endif // !__NET_TASK_H__
