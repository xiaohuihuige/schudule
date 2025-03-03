#include "net-common.h"
#include "kernel_list.h"

typedef struct kernel_list{
	int data;
	struct list_head list;
}kl_st, *kl_pt;

kl_pt kl_list_init(void)
{
	// 1.申请堆空间给头节点
	kl_pt h = malloc(sizeof(kl_st));
	if(h == NULL)
	{
		ERR("head malloc failed");
		return NULL;
	}

	INIT_LIST_HEAD(&h->list);
	
	return h;
}

void kl_list_add_tail(kl_pt head, int new_data)
{
	// 1.申请堆空间给新节点，清空并把数据给入
	kl_pt new_node = malloc(sizeof(kl_st));
	if(new_node == NULL)
	{
		ERR("new_node malloc failed");
		return;
	}
	memset(new_node, 0, sizeof(kl_st));
	new_node->data = new_data;
	
	// 2.调用内核链表函数添加函数
	list_add_tail(&new_node->list, &head->list);
}

// 删除指定数据
void kl_list_add_del(kl_pt head, int del_data)
{
	if(list_empty(&head->list))
	{
		DBG("链表为空");
		return ;
	}
    
	kl_pt pos;
	pos=head;

	while(1)
	{
		if(pos->data!=del_data)
		{
			pos=list_entry(pos->list.next, kl_st, list);
			if(pos == head)
			{
				DBG("没找到%d",del_data);
				return ;
			}
		}
		else 
		{
			list_del(&pos->list);
			break;
		}
		printf("%d == %d",pos->data,del_data);
	}
	printf("\n");

}

void kl_list_show(kl_pt head)
{
	kl_pt get_node;
	
	struct list_head *pos;
	list_for_each(pos, &head->list)
	{
		get_node = list_entry(pos, kl_st, list);
		printf("%d ", get_node->data);
	}
    printf("\n");
}

int main()
{	
	kl_pt head = kl_list_init();
	int cmd;
	while(1)
	{
		printf("Pls Input: ");
		scanf("%d", &cmd); while(getchar()!='\n');
		if(cmd > 0)	//添加
		{
			kl_list_add_tail(head, cmd);
		}
		else if(cmd < 0)	//删除
		{
			kl_list_add_del(head, -cmd);
		}
		if(cmd == 0)	//退出
		{
			exit(0);
		}
		kl_list_show(head);	//显示链表数据
	}
	return 0;	
}