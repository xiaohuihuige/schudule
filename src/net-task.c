#include "net-task.h"

task_list *net_task_list_init(void)
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

task_list *net_task_list_add_tail(task_list *head, void *task)
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

task_list *net_task_list_pop_head(task_list *head)
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
