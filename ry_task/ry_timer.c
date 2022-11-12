
#include "ry_timer.h"
#include "ry_list.h"
#include "ry_core.h"
#include "ry_mem.h"


static volatile ry_u32_t ryTick = 0x12251225;

/* 延时挂起链表 */
static ry_list_t ryDelayList;





/**
 * 描述：获取当前时间
 *
 **/
ry_u32_t ry_get_tick(void)
{
	return ryTick;
}

/**
 * 描述：获取延时挂起链表
 *
 **/
ry_list_t *ry_get_delay_list(void)
{
	return &ryDelayList;
}

/**
 * 描述：挂起链表初始化
 *
 **/
void ry_delay_list_init(void)
{
	ry_list_init(&ryDelayList);
}

/**
 * 描述：定时器初始化
 *
 **/
void ry_timer_reg(ry_timer_t *timer, void (*timeout)(void*),
	                 void *param, ry_u32_t time, ry_u8_t type)
{
	RY_NEW_IT_VARI;
	ry_list_t *NodePos;
	timer->time        = time;
	timer->delay       = time + ryTick;
	timer->delay_type  = type;
	timer->delay_func  = timeout;
	timer->delay_param = param;
	
	RY_INTERRUPT_OFF;
	/* 找到要插入挂起链表的节点位置 */
	NodePos = ry_get_timer_pos(&timer->delay_list);
	ry_list_insert_before(NodePos, &timer->delay_list);
	RY_INTERRUPT_ON;
}

#if RY_MEM == 1
/**
 * 描述：创建定时器
 *
 **/
ry_timer_t *ry_timer_create(void (*timeout)(void*), void *param,
														ry_u32_t time, ry_u8_t type)
{
	ry_timer_t *Timer;
	Timer    = (ry_timer_t *)ry_malloc(sizeof(ry_timer_t));
	if(Timer == RY_NULL)
		return RY_NULL;
	ry_timer_reg(Timer, timeout, param, time, type);
	return Timer;
}

/**
 * 描述：删除定时器
 *
 **/
void ry_timer_delete(ry_timer_t *timer)
{
	RY_NEW_IT_VARI;
	RY_INTERRUPT_OFF;
	ry_list_remove(&timer->delay_list);
	RY_INTERRUPT_ON;
	ry_free(timer);
}
#endif


/**
 * 描述：获取定时器节点在延时挂起链表的位置
 *
 **/
ry_list_t *ry_get_timer_pos(ry_list_t *n)
{
	RY_NEW_IT_VARI;
	ry_list_t  *NodePos;
	ry_timer_t *Node = TASK_CONTAINER_OF(n, ry_timer_t, delay_list);
	RY_INTERRUPT_OFF;
	NodePos          = ryDelayList.next;
	/* 找到要插入链表的节点位置 */
	for(; NodePos != &ryDelayList; NodePos = NodePos->next)
	{
		ry_timer_t *t = TASK_CONTAINER_OF(NodePos, ry_timer_t, delay_list);
		/* 当前定时器的延时，小于下一个挂起任务的延时 */
		if(Node->delay - t->delay > RY_TICK_MAX_DIV_2)
		{
			break;
		}
	}
	RY_INTERRUPT_ON;
	return NodePos;
}





/**
 * 描述：任务延时，从就绪链表移至挂起链表
 *
 **/
void ry_task_delay(ry_u32_t time)
{
	RY_NEW_IT_VARI;
	ry_task_t  *Task;
	
	RY_INTERRUPT_OFF;
	Task        = ry_get_task();
	Task->delay = ryTick + time;
	
	if(time > 0)
	{
		ry_list_t  *NodePos;
		/* 找到要插入挂起链表的节点位置 */
		NodePos = ry_get_timer_pos(&Task->delay_list);
		/* 先从就绪链表删除 */
		ry_task_discard();
		ry_list_insert_before(NodePos, &Task->delay_list);
		Task->status = RY_SUSPEND;
	}
	RY_INTERRUPT_ON;
	
	/* 更新当前的任务 */
	ry_scheduler();
}

/**
 * 描述：任务超时处理
 *
 **/
void ry_timeout_handler(void *param)
{
	ry_task_t  *Task = param;
	
	ry_task_recover(&Task->list);
	Task->err = RY_ERR_TIMEOUT;
	
	/* 更新当前的任务 */
	ry_scheduler();
}


/**
 * 描述：Tick中断处理函数
 *
 **/
void ry_tick_handler(void)
{
	RY_NEW_IT_VARI;
	ry_task_t  *Task;
	
	ryTick++;
	RY_INTERRUPT_OFF;
	Task = ry_get_task();
	/* 时间片已用完,同时确保处于就绪态，避免刚退出就绪态，又再次进入就绪 */
	if(--Task->remaining_tick == 0 && Task->status == RY_READY)
	{
		ry_list_t *Prev = Task->list.prev;
		/* 重载时间片 */
		Task->remaining_tick = Task->init_tick;
		/* 插入到当前优先级的就绪列表的末尾 */
		ry_list_remove(&Task->list);
		ry_list_insert_before(Prev, &Task->list);
	}
	
	/* 检测延时是否到达 */
	if(ryDelayList.next != &ryDelayList)
	{
		ry_timer_t *Timer = TASK_CONTAINER_OF(ryDelayList.next, ry_timer_t, delay_list);
		/* 延时时间到了，从挂起链表移至就绪链表 */
		if(Timer->delay - ryTick > RY_TICK_MAX_DIV_2)
		{
			RY_INTERRUPT_ON;
			/* 执行超时处理函数  */
			Timer->delay_func(Timer->delay_param);
			if(Timer->delay_type == RY_TIMER_CYCLE)
			{
				RY_INTERRUPT_OFF;
				ry_list_remove(&Timer->delay_list);
				Timer->delay = Timer->time + ryTick;
				/* 找到要插入挂起链表的节点位置，并插入 */
				ry_list_insert_before(ry_get_timer_pos(&Timer->delay_list), &Timer->delay_list);
				RY_INTERRUPT_ON;
				return;
			}
			return;
		}
	}
	RY_INTERRUPT_ON;
}






