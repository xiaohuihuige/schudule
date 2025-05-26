#ifndef __LINUX_LIST_H__
#define __LINUX_LIST_H__

/*最好的算法只维护结构，不操作实际数据*/

#include "net-common.h"

#define container_of(ptr, type, member) \
  ((type*) ((uintptr_t) (ptr) - offsetof(type, member)))

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *new_head,
			      struct list_head *prev,
			      struct list_head *next)
{

	next->prev = new_head;
	new_head->next = next;
	new_head->prev = prev;
    prev->next = new_head;
}

static inline void list_add(struct list_head *new_head, struct list_head *head)
{
    __list_add(new_head, head, head->next);
}

static inline void list_add_tail(struct list_head *new_head, struct list_head *head)
{
    __list_add(new_head, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void __list_del_clearprev(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->prev = NULL;
}

static inline void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del_entry(entry);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline void list_replace(struct list_head *old,
				struct list_head *new_head)
{
	new_head->next = old->next;
	new_head->next->prev = new_head;
	new_head->prev = old->prev;
	new_head->prev->next = new_head;
}

static inline void list_replace_init(struct list_head *old,
				     struct list_head *new_head)
{
	list_replace(old, new_head);
	INIT_LIST_HEAD(old);
}

static inline void list_swap(struct list_head *entry1,
			     struct list_head *entry2)
{
	struct list_head *pos = entry2->prev;

	list_del(entry2);
	list_replace(entry1, entry2);
	if (pos == entry1)
		pos = entry2;
	list_add(entry1, pos);
}

static inline void list_del_init(struct list_head *entry)
{
	__list_del_entry(entry);
	INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del_entry(list);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
	__list_del_entry(list);
	list_add_tail(list, head);
}

static inline void list_bulk_move_tail(struct list_head *head,
				       struct list_head *first,
				       struct list_head *last)
{
	first->prev->next = last->next;
	last->next->prev = first->prev;

	head->prev->next = first;
	first->prev = head->prev;

	last->next = head;
	head->prev = last;
}

static inline int list_is_first(const struct list_head *list, const struct list_head *head)
{
	return list->prev == head;
}

static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
	return list->next == head;
}

static inline int list_is_head(const struct list_head *list, const struct list_head *head)
{
	return list == head;
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

#define list_first_entry_or_null(ptr, type, member) ({ \
	struct list_head *head__ = (ptr); \
	struct list_head *pos__ = READ_ONCE(head__->next); \
	pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
}) 

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

#define list_next_entry_circular(pos, head, member) \
	(list_is_last(&(pos)->member, head) ? \
	list_first_entry(head, typeof(*(pos)), member) : list_next_entry(pos, member))

//遍历
#define list_for_each(pos, head) \
	for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)

#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; !list_is_head(pos, (head)); pos = pos->prev)

#define list_for_each_reverse(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

#define list_for_each_continue(pos, head) \
	for (pos = pos->next; !list_is_head(pos, (head)); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; !list_is_head(pos, (head)); \
	     pos = n, n = pos->next)  

#define list_for_each_prev_safe(pos, n, head) \
            for (pos = (head)->prev, n = pos->prev; \
                    pos != (head); \
                    pos = n, n = pos->prev)

static inline size_t list_count_nodes(struct list_head *head)
{
	struct list_head *pos;
	size_t count = 0;

	list_for_each(pos, head)
		count++;

	return count;
}

#define list_entry_is_head(pos, head, member)				\
	list_is_head(&pos->member, (head))

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_last_entry(head, typeof(*pos), member);		\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = list_prev_entry(pos, member))    

#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))  

#define list_for_each_entry_continue(pos, head, member)  \
            for (pos = list_next_entry(pos, member); \
                    &pos->member != (head); \
                    pos = list_next_entry(pos, member))

#define list_for_each_entry_from(pos, head, member)  \
            for (; &pos->member != (head); \
                    pos = list_next_entry(pos, member))

#define list_for_each_entry_safe(pos, n, head, member) \
            for (pos = list_first_entry(head, typeof(*pos), member), n = list_next_entry(pos, member); \
                &pos->member != (head); pos = n, n = list_next_entry(n, member))

#define list_for_each_entry_safe_continue(pos, n, head, member)  \
            for (pos = list_next_entry(pos, member),  \
                    n = list_next_entry(pos, member); \
                    &pos->member != (head); \
                    pos = n, n = list_next_entry(n, member))

#define list_for_each_entry_safe_from(pos, n, head, member)  \
            for (n = list_next_entry(pos, member); \
                    &pos->member != (head); \
                    pos = n, n = list_next_entry(n, member))

#define list_safe_reset_next(pos, n, member) \
            n = list_next_entry(pos, member)	
																	
#endif // !__LINUX_LIST_H__
