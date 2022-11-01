
#include "ry_core.h"
#include "ry_timer.h"
#include "ry_mem.h"
#include "ry_list.h"

#include "ry_hw.h"


/* cpu使用率 */
ry_u8_t  ryCPU;

/* 当前任务 */
ry_task_t *ryCurrentTask;

/* 就绪链表 */
static ry_list_t ryReadyList[RY_PRIORITY_NUM];

/* 就绪优先级组，有任务的标志 */
static ry_u32_t ryReadyPriorityFlag;

ry_task_t *ry_get_new_task(void);


extern void ry_timeout_handler(void *param);

extern ry_u8_t *ry_stack_init(void *entry, void *param, ry_u8_t *stack_addr);




/* 设置/清除就绪优先级组的标志 */
#define  RY_GET_PRIORITY_MASK(p)             (ry_u32_t)((ry_u32_t)0x1 << (p)) 
#define  RY_SET_READY_PRIORITY_FLAG(prio)     ryReadyPriorityFlag |= RY_GET_PRIORITY_MASK(prio)
#define  RY_CLEAR_READY_PRIORITY_FLAG(prio)   ryReadyPriorityFlag &= ~RY_GET_PRIORITY_MASK(prio)



/**
 * 描述：获取当前任务
 *
 **/
ry_task_t *ry_get_task(void)
{
    return ryCurrentTask;
}

/**
 * 描述：就绪链表初始化
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
 * 描述：任务注册
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
    /* 配置任务栈 */
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
 * 描述：创建任务
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
 * 描述：任务删除
 *
 **/
void ry_task_delete(ry_task_t *task)
{
    RY_NEW_IT_VARI;
    task->status = RY_DIE;
    RY_INTERRUPT_OFF;
    ry_list_remove(&task->list);
    ry_list_remove(&task->delay_list);
    /* 当前优先级就绪链表无任务，清标志 */
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
 * 描述：ry系统初始化
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
 * 描述：启动ry系统
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
 * 描述：丢弃当前任务，但不挂起
 *
 **/
void ry_task_discard(void)
{
    RY_NEW_IT_VARI;
    RY_INTERRUPT_OFF;
    ry_list_remove(&ryCurrentTask->list);
    ry_list_remove(&ryCurrentTask->delay_list);
    /* 当前优先级就绪链表无任务，清标志 */
    if(ryReadyList[ryCurrentTask->priority].next == &ryReadyList[ryCurrentTask->priority])
    {
        RY_CLEAR_READY_PRIORITY_FLAG(ryCurrentTask->priority);
    }
    RY_INTERRUPT_ON;
}

/**
 * 描述：恢复非延时挂起的任务
 *
 **/
ry_u8_t ry_task_recover(ry_list_t *list)
{
    ry_task_t  *Task = TASK_CONTAINER_OF(list, ry_task_t, list);
    RY_NEW_IT_VARI;
    
    /* 任务未处于挂起态 */
    if(Task->status != RY_SUSPEND)
        return RY_ERR_STATUS;
    RY_INTERRUPT_OFF;
    /* 从延时挂起链表删除 */
    ry_list_remove(&Task->delay_list);
    /* 从挂起链表删除 */
    ry_list_remove(&Task->list);
//    /* 重载时间片 */
//    Task->remaining_tick = Task->init_tick;
    RY_SET_READY_PRIORITY_FLAG(Task->priority);
    ry_list_insert_after(ryReadyList[Task->priority].prev, &Task->list);
    Task->err    = RY_OK;
    Task->status = RY_READY;
    RY_INTERRUPT_ON;
    return RY_OK;
}

/**
 * 描述：当前任务等待某对象，需要挂起，挂起后要及时调度任务
 *
 **/
ry_u8_t ry_task_suspend(ry_obj_t *obj)
{
    ry_list_t *NodePos;
    ry_list_t *SuspendList;
    RY_NEW_IT_VARI;
    
    /* 任务未处于就绪态 */
    if(ryCurrentTask->status != RY_READY)
        return RY_ERR_STATUS;
    RY_INTERRUPT_OFF;
    ry_list_remove(&ryCurrentTask->list);
    ry_list_remove(&ryCurrentTask->delay_list);
    /* 当前优先级就绪链表无任务，清标志 */
    if(ryReadyList[ryCurrentTask->priority].next == &ryReadyList[ryCurrentTask->priority])
    {
        RY_CLEAR_READY_PRIORITY_FLAG(ryCurrentTask->priority);
    }
    switch(obj->type & RY_OBJ_LIST_MASK)
    {
        case RY_OBJ_LIST_FIFO :
            /* 先进先出，那么总是插入到挂起链表末尾 */
            ry_list_insert_before(&obj->suspend_list, &ryCurrentTask->list);
            break;
        case RY_OBJ_LIST_PRIO :
            /* 以优先级等级，综合判断节点位置 */
            SuspendList = &obj->suspend_list;
            NodePos     = SuspendList->next;
            /* 找到要插入链表的节点位置 */
            for(; NodePos != SuspendList; NodePos = NodePos->next)
            {
                ry_task_t *t = TASK_CONTAINER_OF(NodePos, ry_task_t, list);
                /* 当前任务优先级更高 */
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
 * 描述：等待某个对象释放资源
 *
 **/
ry_u8_t ry_wait_obj(ry_obj_t *obj, ry_int16_t time)
{
    ry_task_t *Task;
    RY_NEW_IT_VARI;
    
    RY_INTERRUPT_OFF;
    Task      = ryCurrentTask;
    Task->err = RY_OK;
    /* 对象无剩余资源 */
    {
        ry_u16_t Delay;
        /* 任务不等待 */
        if(time == 0)
        {
            RY_INTERRUPT_ON;
            return RY_ERR;
        }
        Delay = (time > 0) ? time : 0;
        Task->delay = Delay + ry_get_tick();
        /* 执行优先级继承 */
        if((obj->type & RY_OBJ_PRIO_MASK) == RY_OBJ_PRIO_INHERIT)
        {
            ry_mutex_t *m = (ry_mutex_t *)obj;
            /* 互斥量所有者的优先级较低，则继承当前任务的优先级 */
            if(m->task->priority > Task->priority)
                m->task->priority = Task->priority;
        }
        /* 挂起任务 */
        ry_task_suspend(obj);
        
        /* 准备延时，等待对象 */
        if(Delay > 0)
        {
            ry_list_t  *NodePos;
            /* 找到要插入挂起链表的节点位置 */
            NodePos = ry_get_timer_pos(&Task->delay_list);
            ry_list_remove(&Task->delay_list);
            ry_list_insert_before(NodePos, &Task->delay_list);
            Task->status = RY_SUSPEND;
        }
        RY_INTERRUPT_ON;
        ry_scheduler();
        
        /* 等待超时 */
        if(Task->err != RY_OK)
            return RY_ERR;
    }
    RY_INTERRUPT_ON;
    return RY_OK;
}


/**
 * 描述：获取就绪链表中最新的任务
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




