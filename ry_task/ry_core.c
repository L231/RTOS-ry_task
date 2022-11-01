
#include "ry_core.h"
#include "ry_timer.h"
#include "ry_mem.h"
#include "ry_list.h"

#include "ry_hw.h"


/* cpuʹ���� */
ry_u8_t  ryCPU;

/* ��ǰ���� */
ry_task_t *ryCurrentTask;

/* �������� */
static ry_list_t ryReadyList[RY_PRIORITY_NUM];

/* �������ȼ��飬������ı�־ */
static ry_u32_t ryReadyPriorityFlag;

ry_task_t *ry_get_new_task(void);


extern void ry_timeout_handler(void *param);

extern ry_u8_t *ry_stack_init(void *entry, void *param, ry_u8_t *stack_addr);




/* ����/����������ȼ���ı�־ */
#define  RY_GET_PRIORITY_MASK(p)             (ry_u32_t)((ry_u32_t)0x1 << (p)) 
#define  RY_SET_READY_PRIORITY_FLAG(prio)     ryReadyPriorityFlag |= RY_GET_PRIORITY_MASK(prio)
#define  RY_CLEAR_READY_PRIORITY_FLAG(prio)   ryReadyPriorityFlag &= ~RY_GET_PRIORITY_MASK(prio)



/**
 * ��������ȡ��ǰ����
 *
 **/
ry_task_t *ry_get_task(void)
{
    return ryCurrentTask;
}

/**
 * ���������������ʼ��
 *
 **/
void ry_ready_list_init(void)
{
    ry_u8_t Pos;
    for(Pos = 0; Pos < RY_PRIORITY_NUM; Pos++)
    {
        ry_list_init(&ryReadyList[Pos]);
    }
}


/**
 * ����������ע��
 *
 **/
void ry_task_reg(ry_task_t  *task,
                 char       *name,
                 void      (*func)(void *),
                 void       *param,
                 ry_u8_t     priority,
                 ry_u32_t    tick,
                 void       *stack_addr,
                 ry_u32_t    stack_size)
{
    RY_NEW_IT_VARI;
    task->name           = name;
    task->func           = func;
    task->param          = param;
    task->status         = RY_READY;
    task->priority       = priority;
    task->init_tick      = tick;
    task->remaining_tick = tick;
    task->delay_type     = RY_TIMER_ONE;
    task->delay_func     = ry_timeout_handler;
    task->delay_param    = task;
    /* ��������ջ */
    task->stack_addr     = stack_addr;
    task->stack_size     = stack_size;
    task->sp             = ry_stack_init(func, param,
                           (void *)((ry_u32_t)stack_addr+stack_size-4));
    ry_list_init(&task->delay_list);
    
    RY_INTERRUPT_OFF;
    RY_SET_READY_PRIORITY_FLAG(priority);
    ry_list_insert_after(ryReadyList[priority].prev, &task->list);
    RY_INTERRUPT_ON;
}

#if RY_MEM == 1
/**
 * ��������������
 *
 **/
ry_task_t *ry_task_create(char       *name,
                          void      (*func)(void *),
                          void       *param,
                          ry_u8_t     priority,
                          ry_u32_t    tick,
                          ry_u32_t    stack_size)
{
    ry_u8_t *stack_addr;
    ry_task_t *Task;
    stack_size  = RY_ALIGN((ry_u32_t)stack_size, RY_ALIGN_SIZE);
    stack_addr  = (ry_u8_t *)ry_malloc(stack_size);
    Task        = (ry_task_t *)ry_malloc(sizeof(ry_task_t));
    if(Task != RY_NULL)
    {
        ry_task_reg(Task, name, func, param, priority, tick, stack_addr, stack_size);
    }
    if(Task == RY_NULL || stack_addr == RY_NULL)
    {
        RY_DBG_PRINTF(RY_PRINTF, ("ry_task_create error!\r\n"));
    }
    return Task;
}


/**
 * ����������ɾ��
 *
 **/
void ry_task_delete(ry_task_t *task)
{
    RY_NEW_IT_VARI;
    task->status = RY_DIE;
    RY_INTERRUPT_OFF;
    ry_list_remove(&task->list);
    ry_list_remove(&task->delay_list);
    /* ��ǰ���ȼ������������������־ */
    if(ryReadyList[task->priority].next == &ryReadyList[task->priority])
    {
        RY_CLEAR_READY_PRIORITY_FLAG(task->priority);
    }
    ry_free(task->stack_addr);
    ry_free(task);
    RY_INTERRUPT_ON;
}
#endif



/**
 * ������ryϵͳ��ʼ��
 *
 **/
void ry_init(void)
{
extern void ry_idle_init(void);
#if RY_MEM == 1
extern void ry_sysheap_init(void);
#endif
    ryCPU               = 100;
    ryCurrentTask       = RY_NULL;
    ryReadyPriorityFlag = 0;
    ry_ready_list_init();
    ry_delay_list_init();
#if RY_MEM == 1
    ry_sysheap_init();
#endif
    ry_idle_init();
    ryCurrentTask = ry_get_new_task();
}

/**
 * ����������ryϵͳ
 *
 **/
void ry_start(void)
{
extern void ry_systick_cfg(void);
extern void ry_start_hook(void);
extern void ry_scheduler_start(void);
    ry_start_hook();
    ry_systick_cfg();
    ry_scheduler_start();
}


/**
 * ������������ǰ���񣬵�������
 *
 **/
void ry_task_discard(void)
{
    RY_NEW_IT_VARI;
    RY_INTERRUPT_OFF;
    ry_list_remove(&ryCurrentTask->list);
    ry_list_remove(&ryCurrentTask->delay_list);
    /* ��ǰ���ȼ������������������־ */
    if(ryReadyList[ryCurrentTask->priority].next == &ryReadyList[ryCurrentTask->priority])
    {
        RY_CLEAR_READY_PRIORITY_FLAG(ryCurrentTask->priority);
    }
    RY_INTERRUPT_ON;
}

/**
 * �������ָ�����ʱ���������
 *
 **/
ry_u8_t ry_task_recover(ry_list_t *list)
{
    ry_task_t  *Task = TASK_CONTAINER_OF(list, ry_task_t, list);
    RY_NEW_IT_VARI;
    
    /* ����δ���ڹ���̬ */
    if(Task->status != RY_SUSPEND)
        return RY_ERR_STATUS;
    RY_INTERRUPT_OFF;
    /* ����ʱ��������ɾ�� */
    ry_list_remove(&Task->delay_list);
    /* �ӹ�������ɾ�� */
    ry_list_remove(&Task->list);
//    /* ����ʱ��Ƭ */
//    Task->remaining_tick = Task->init_tick;
    RY_SET_READY_PRIORITY_FLAG(Task->priority);
    ry_list_insert_after(ryReadyList[Task->priority].prev, &Task->list);
    Task->err    = RY_OK;
    Task->status = RY_READY;
    RY_INTERRUPT_ON;
    return RY_OK;
}

/**
 * ��������ǰ����ȴ�ĳ������Ҫ���𣬹����Ҫ��ʱ��������
 *
 **/
ry_u8_t ry_task_suspend(ry_obj_t *obj)
{
    ry_list_t *NodePos;
    ry_list_t *SuspendList;
    RY_NEW_IT_VARI;
    
    /* ����δ���ھ���̬ */
    if(ryCurrentTask->status != RY_READY)
        return RY_ERR_STATUS;
    RY_INTERRUPT_OFF;
    ry_list_remove(&ryCurrentTask->list);
    ry_list_remove(&ryCurrentTask->delay_list);
    /* ��ǰ���ȼ������������������־ */
    if(ryReadyList[ryCurrentTask->priority].next == &ryReadyList[ryCurrentTask->priority])
    {
        RY_CLEAR_READY_PRIORITY_FLAG(ryCurrentTask->priority);
    }
    switch(obj->type & RY_OBJ_LIST_MASK)
    {
        case RY_OBJ_LIST_FIFO :
            /* �Ƚ��ȳ�����ô���ǲ��뵽��������ĩβ */
            ry_list_insert_before(&obj->suspend_list, &ryCurrentTask->list);
            break;
        case RY_OBJ_LIST_PRIO :
            /* �����ȼ��ȼ����ۺ��жϽڵ�λ�� */
            SuspendList = &obj->suspend_list;
            NodePos     = SuspendList->next;
            /* �ҵ�Ҫ��������Ľڵ�λ�� */
            for(; NodePos != SuspendList; NodePos = NodePos->next)
            {
                ry_task_t *t = TASK_CONTAINER_OF(NodePos, ry_task_t, list);
                /* ��ǰ�������ȼ����� */
                if(ryCurrentTask->priority < t->priority)
                    break;
            }
            ry_list_insert_before(NodePos, &ryCurrentTask->list);
            break;
    }
    ryCurrentTask->status = RY_SUSPEND;
    RY_INTERRUPT_ON;
    return RY_OK;
}

/**
 * �������ȴ�ĳ�������ͷ���Դ
 *
 **/
ry_u8_t ry_wait_obj(ry_obj_t *obj, ry_int16_t time)
{
    ry_task_t *Task;
    RY_NEW_IT_VARI;
    
    RY_INTERRUPT_OFF;
    Task      = ryCurrentTask;
    Task->err = RY_OK;
    /* ������ʣ����Դ */
    {
        ry_u16_t Delay;
        /* ���񲻵ȴ� */
        if(time == 0)
        {
            RY_INTERRUPT_ON;
            return RY_ERR;
        }
        Delay = (time > 0) ? time : 0;
        Task->delay = Delay + ry_get_tick();
        /* ִ�����ȼ��̳� */
        if((obj->type & RY_OBJ_PRIO_MASK) == RY_OBJ_PRIO_INHERIT)
        {
            ry_mutex_t *m = (ry_mutex_t *)obj;
            /* �����������ߵ����ȼ��ϵͣ���̳е�ǰ��������ȼ� */
            if(m->task->priority > Task->priority)
                m->task->priority = Task->priority;
        }
        /* �������� */
        ry_task_suspend(obj);
        
        /* ׼����ʱ���ȴ����� */
        if(Delay > 0)
        {
            ry_list_t  *NodePos;
            /* �ҵ�Ҫ�����������Ľڵ�λ�� */
            NodePos = ry_get_timer_pos(&Task->delay_list);
            ry_list_remove(&Task->delay_list);
            ry_list_insert_before(NodePos, &Task->delay_list);
            Task->status = RY_SUSPEND;
        }
        RY_INTERRUPT_ON;
        ry_scheduler();
        
        /* �ȴ���ʱ */
        if(Task->err != RY_OK)
            return RY_ERR;
    }
    RY_INTERRUPT_ON;
    return RY_OK;
}


/**
 * ��������ȡ�������������µ�����
 *
 **/
ry_task_t *ry_get_new_task(void)
{
    RY_NEW_IT_VARI;
    ry_list_t  *Node;
    
    RY_INTERRUPT_OFF;
    Node = ryReadyList[RY_GET_READY_TASK_POS(ryReadyPriorityFlag)].next;
    RY_INTERRUPT_ON;
    return TASK_CONTAINER_OF(Node, ry_task_t, list);
}




