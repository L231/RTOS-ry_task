
#include "ry_timer.h"
#include "ry_list.h"
#include "ry_core.h"
#include "ry_mem.h"


static volatile ry_u32_t ryTick = 0x12251225;

/* ��ʱ�������� */
static ry_list_t ryDelayList;





/**
 * ��������ȡ��ǰʱ��
 *
 **/
ry_u32_t ry_get_tick(void)
{
	return ryTick;
}

/**
 * ��������ȡ��ʱ��������
 *
 **/
ry_list_t *ry_get_delay_list(void)
{
	return &ryDelayList;
}

/**
 * ���������������ʼ��
 *
 **/
void ry_delay_list_init(void)
{
	ry_list_init(&ryDelayList);
}

/**
 * ��������ʱ����ʼ��
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
	/* �ҵ�Ҫ�����������Ľڵ�λ�� */
	NodePos = ry_get_timer_pos(&timer->delay_list);
	ry_list_insert_before(NodePos, &timer->delay_list);
	RY_INTERRUPT_ON;
}

#if RY_MEM == 1
/**
 * ������������ʱ��
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
 * ������ɾ����ʱ��
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
 * ��������ȡ��ʱ���ڵ�����ʱ���������λ��
 *
 **/
ry_list_t *ry_get_timer_pos(ry_list_t *n)
{
	RY_NEW_IT_VARI;
	ry_list_t  *NodePos;
	ry_timer_t *Node = TASK_CONTAINER_OF(n, ry_timer_t, delay_list);
	RY_INTERRUPT_OFF;
	NodePos          = ryDelayList.next;
	/* �ҵ�Ҫ��������Ľڵ�λ�� */
	for(; NodePos != &ryDelayList; NodePos = NodePos->next)
	{
		ry_timer_t *t = TASK_CONTAINER_OF(NodePos, ry_timer_t, delay_list);
		/* ��ǰ��ʱ������ʱ��С����һ�������������ʱ */
		if(Node->delay - t->delay > RY_TICK_MAX_DIV_2)
		{
			break;
		}
	}
	RY_INTERRUPT_ON;
	return NodePos;
}





/**
 * ������������ʱ���Ӿ�������������������
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
		/* �ҵ�Ҫ�����������Ľڵ�λ�� */
		NodePos = ry_get_timer_pos(&Task->delay_list);
		/* �ȴӾ�������ɾ�� */
		ry_task_discard();
		ry_list_insert_before(NodePos, &Task->delay_list);
		Task->status = RY_SUSPEND;
	}
	RY_INTERRUPT_ON;
	
	/* ���µ�ǰ������ */
	ry_scheduler();
}

/**
 * ����������ʱ����
 *
 **/
void ry_timeout_handler(void *param)
{
	ry_task_t  *Task = param;
	
	ry_task_recover(&Task->list);
	Task->err = RY_ERR_TIMEOUT;
	
	/* ���µ�ǰ������ */
	ry_scheduler();
}


/**
 * ������Tick�жϴ�����
 *
 **/
void ry_tick_handler(void)
{
	RY_NEW_IT_VARI;
	ry_task_t  *Task;
	
	ryTick++;
	RY_INTERRUPT_OFF;
	Task = ry_get_task();
	/* ʱ��Ƭ������,ͬʱȷ�����ھ���̬��������˳�����̬�����ٴν������ */
	if(--Task->remaining_tick == 0 && Task->status == RY_READY)
	{
		ry_list_t *Prev = Task->list.prev;
		/* ����ʱ��Ƭ */
		Task->remaining_tick = Task->init_tick;
		/* ���뵽��ǰ���ȼ��ľ����б��ĩβ */
		ry_list_remove(&Task->list);
		ry_list_insert_before(Prev, &Task->list);
	}
	
	/* �����ʱ�Ƿ񵽴� */
	if(ryDelayList.next != &ryDelayList)
	{
		ry_timer_t *Timer = TASK_CONTAINER_OF(ryDelayList.next, ry_timer_t, delay_list);
		/* ��ʱʱ�䵽�ˣ��ӹ������������������� */
		if(Timer->delay - ryTick > RY_TICK_MAX_DIV_2)
		{
			RY_INTERRUPT_ON;
			/* ִ�г�ʱ������  */
			Timer->delay_func(Timer->delay_param);
			if(Timer->delay_type == RY_TIMER_CYCLE)
			{
				RY_INTERRUPT_OFF;
				ry_list_remove(&Timer->delay_list);
				Timer->delay = Timer->time + ryTick;
				/* �ҵ�Ҫ�����������Ľڵ�λ�ã������� */
				ry_list_insert_before(ry_get_timer_pos(&Timer->delay_list), &Timer->delay_list);
				RY_INTERRUPT_ON;
				return;
			}
			return;
		}
	}
	RY_INTERRUPT_ON;
}






