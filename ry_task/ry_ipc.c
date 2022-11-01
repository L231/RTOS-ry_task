
#include "ry_list.h"
#include "ry_core.h"
#include "ry_timer.h"
#include "ry_mem.h"





#if RY_MSG == 1


/**
 * �������½�����
 *
 **/
void ry_msg_reg(ry_msg_t *msg, ry_u16_t type, ry_u8_t max, ry_u8_t **buf)
{
    ry_u8_t Pos;
    msg->valid      = 0;
    msg->max        = max;
    msg->type       = type;
    msg->write      = 0;
    msg->read       = 0;
    msg->msg        = buf;
    for(Pos = 0; Pos < max; Pos++)
    {
        msg->msg[Pos] = RY_NULL;
    }
    /* ��ʼ���������� */
    ry_list_init(&msg->suspend_list);
}

#if RY_MEM == 1
/**
 * ��������������
 *
 **/
ry_msg_t *ry_msg_create(ry_u16_t type, ry_u8_t max)
{
    ry_u8_t Pos;
    ry_u8_t **MsgBuf;
    ry_msg_t *Msg;
    
    MsgBuf = (ry_u8_t **)ry_malloc(sizeof(ry_u8_t *) * max);
    Msg    = (ry_msg_t *)ry_malloc(sizeof(ry_msg_t));
    if(Msg == RY_NULL)
        return RY_NULL;
    ry_msg_reg(Msg, type, max, MsgBuf);
    return Msg;
}

/**
 * ������ɾ������
 *
 **/
void ry_msg_delete(ry_msg_t *msg)
{
    ry_u8_t Flag = 0;
    ry_task_t *Task;
    ry_list_t *NodePos = msg->suspend_list.next;
    RY_NEW_IT_VARI;
    
    msg->valid = 0;
    RY_INTERRUPT_OFF;
    /* �ָ���ȴ��ʼ������������ */
    for(; NodePos != &msg->suspend_list; NodePos = NodePos->next)
    {
        Task      = TASK_CONTAINER_OF(NodePos, ry_task_t, list);
        ry_task_recover(&Task->list);
        Task->err = RY_ERR_TIMEOUT;
        Flag = 1;
    }
    RY_INTERRUPT_ON;
    
    ry_free(msg);
    ry_free(msg->msg);
    if(Flag)
        ry_scheduler();
}
#endif


/**
 * ��������������
 *
 **/
ry_u8_t ry_msg_send(ry_msg_t *msg, ry_u8_t mode, ry_u8_t *buf)
{
    RY_NEW_IT_VARI;
    
    RY_INTERRUPT_OFF;
    /* �������� */
    if(msg->write - msg->read == -1)
    {
        RY_INTERRUPT_ON;
        return RY_ERR;
    }
    /* �Ƿ���Ҫ���͸����ȼ��ʼ� */
    if(mode == RY_MSG_SEND_NORMAL)
    {
        if(msg->read == 0)
            msg->read = msg->max;
        msg->msg[--msg->read] = buf;
    }
    else
    {
        msg->msg[msg->write++] = buf;
        if(msg->write == msg->max)
            msg->write = 0;
    }
    
    /* �ָ���ȴ��ʼ������������ */
    if(msg->suspend_list.next != &msg->suspend_list)
    {
        ry_task_recover(msg->suspend_list.next);
        RY_INTERRUPT_ON;
        ry_scheduler();
        return RY_OK;
    }
    msg->valid++;
    /* ��ǰ��������� */
    RY_INTERRUPT_ON;
    return RY_OK;
}

/**
 * ��������������
 *
 **/
ry_u8_t *ry_msg_rec(ry_msg_t *msg, ry_u8_t mode, ry_int16_t time)
{
    ry_u8_t *buf;
    RY_NEW_IT_VARI;
    
    if(msg->valid)
    {
        msg->valid--;
    }
    else if(ry_wait_obj((ry_obj_t *)msg, time) != RY_OK)
    {
        return RY_NULL;
    }
    RY_INTERRUPT_OFF;
    /* ���ʼ���ַ���ظ����� */
    buf = msg->msg[msg->read];
    /* �ĺ󼴷٣���ָ������ */
    if(mode == RY_RC)
    {
        msg->msg[msg->read] = RY_NULL;
        if(++msg->read == msg->max)
            msg->read = 0;
    }
    RY_INTERRUPT_ON;
    return buf;
}

#endif


#if RY_SEM == 1


/**
 * �������½��ź���
 *
 **/
void ry_semaphore_reg(ry_semaphore_t *semaphore, ry_u16_t type, ry_u8_t valid)
{
    semaphore->type   = type;
    semaphore->valid  = valid;
    ry_list_init(&semaphore->suspend_list);
}

/**
 * �������½�������
 *
 **/
void ry_mutex_reg(ry_mutex_t *mutex, ry_u16_t type)
{
    mutex->type   = type;
    mutex->valid  = 0;
    mutex->task   = RY_NULL;
    ry_list_init(&mutex->suspend_list);
}

/**
 * �������ͷ��ź���
 *
 **/
void ry_semaphore_send(ry_semaphore_t *semaphore)
{
    RY_NEW_IT_VARI;
    RY_INTERRUPT_OFF;
    /* �еȴ���������ָ�������Ϊ����̬ */
    if(semaphore->suspend_list.next != &semaphore->suspend_list)
    {
        ry_task_recover(semaphore->suspend_list.next);
        RY_INTERRUPT_ON;
        ry_scheduler();
        return;
    }
    if(semaphore->valid < 0xFF)
        semaphore->valid++;
        
    RY_INTERRUPT_ON;
}

/**
 * �������ͷŻ�����
 *
 **/
ry_u8_t ry_mutex_release(ry_mutex_t *mutex)
{
    ry_task_t *Task;
    RY_NEW_IT_VARI;
    
    RY_INTERRUPT_OFF;
    Task = ry_get_task();
    /* ��ǰ�����ǻ����������� */
    if(mutex->task != Task)
    {
        Task->err = RY_ERR_IPC_SEM;
        RY_INTERRUPT_ON;
        return RY_ERR;
    }
    /* ��ǰ������Ҫ�ͷŻ����� */
    if(--mutex->valid == 0)
    {
        /* �����������ߣ��ָ�ԭʼ���ȼ� */
        mutex->task->priority = mutex->priority;
        mutex->task = RY_NULL;
        /* ����������������ָ�������Ϊ����̬ */
        if(mutex->suspend_list.next != &mutex->suspend_list)
        {
            ry_task_t *t = TASK_CONTAINER_OF(mutex->suspend_list.next, ry_task_t, list);
            /* �˴�������'sem'��ʹ���� */
            mutex->valid++;
            mutex->task     = t;
            mutex->priority = t->priority;
            ry_task_recover(&t->list);
            RY_INTERRUPT_ON;
            ry_scheduler();
            return RY_OK;
        }
    }
    
    RY_INTERRUPT_ON;
    return RY_OK;
}


/**
 * �������ͷ��ź�����������
 *
 **/
ry_u8_t ry_sem_release(void *sem)
{
    switch(((ry_obj_t *)sem)->type & RY_OBJ_MASK)
    {
        case RY_OBJ_SEM :
            ry_semaphore_send((ry_semaphore_t *)sem);
            break;
        case RY_OBJ_MUTEX :
            return ry_mutex_release((ry_mutex_t *)sem);
//            break;
    }
    return RY_OK;
}


/**
 * ��������ȡ�ź���
 *
 **/
ry_u8_t ry_semaphore_rec(ry_semaphore_t *semaphore, ry_int16_t time)
{
    if(semaphore->valid)
    {
        semaphore->valid--;
    }
    else if(ry_wait_obj((ry_obj_t *)semaphore, time) != RY_OK)
    {
        return RY_ERR;
    }
    return RY_OK;
}

/**
 * ��������ȡ������
 *
 **/
ry_u8_t ry_mutex_rec(ry_mutex_t *mutex, ry_int16_t time)
{
    ry_task_t *Task;
    RY_NEW_IT_VARI;
    
    RY_INTERRUPT_OFF;
    Task = ry_get_task();
    /* ��ǰ�����ǻ����������� */
    if(mutex->task != Task)
    {
        if(mutex->valid > 0)
        {
            RY_INTERRUPT_ON;
            if(ry_wait_obj((ry_obj_t *)mutex, time) != RY_OK)
            {
                return RY_ERR;
            }
        }
        /* ���������У���ǰ�����û�����ʹ��Ȩ */
        else
        {
            mutex->valid++;
            mutex->task     = Task;
            /* �������ȼ����̳����ȼ����ٴλָ�ʱʹ�� */
            mutex->priority = Task->priority;
        }
    }
    else
    {
        mutex->valid++;
    }
    RY_INTERRUPT_ON;
    return RY_OK;
}

/**
 * ��������ȡ�ź�����������
 *
 **/
ry_u8_t ry_sem_rec(void *sem, ry_int16_t time)
{
    ry_u8_t Status;
    
    switch(((ry_obj_t *)sem)->type & RY_OBJ_MASK)
    {
        case RY_OBJ_SEM :
            Status = ry_semaphore_rec((ry_semaphore_t *)sem, time);
            break;
        case RY_OBJ_MUTEX :
            Status = ry_mutex_rec((ry_mutex_t *)sem, time);
            break;
    }
    return Status;
}

#endif


#if RY_EVENT == 1


/**
 * �������½��¼�
 *
 **/
void ry_event_reg(ry_obj_t *event, ry_u16_t type)
{
    event->type   = type;
    event->valid  = 0;
    ry_list_init(&event->suspend_list);
}

/**
 * �������˲��¼�
 *
 **/
static ry_u8_t _ry_event_check(ry_event_t *event, ry_task_t *task)
{
    ry_u8_t Status = RY_ERR;
    if((task->event_mode & RY_ENENT_MASK) == RY_ENENT_AND)
    {
        if((event->valid & task->event_flag) == task->event_flag)
            Status = RY_OK;
    }
    else if((task->event_mode & RY_ENENT_MASK) == RY_ENENT_OR)
    {
        if(event->valid & task->event_flag)
        {
            task->event_flag &= event->valid;
            Status = RY_OK;
        }
    }
    return Status;
}

/**
 * �������¼�����
 *
 **/
void ry_event_send(ry_event_t *event, ry_u8_t flag)
{
    ry_u8_t TaskUpdate;
    ry_list_t *List;
    RY_NEW_IT_VARI;
    
    TaskUpdate = RY_FALSE;
    RY_INTERRUPT_OFF;
    event->valid |= flag;
    
    /* �������������ָ��ȵ����¼������� */
    List = event->suspend_list.next;
    while(List != &event->suspend_list)
    {
        ry_list_t *Temp = List->next;
        ry_task_t *Task = TASK_CONTAINER_OF(List, ry_task_t, list);
        if(_ry_event_check(event, Task) == RY_OK)
        {
            /* ������������ */
            TaskUpdate = RY_TRUE;
            /* �ָ�����Ϊ����̬ */
            ry_task_recover(List);
//            List = Temp;
            if((Task->event_mode & RY_RC) == RY_RC)
            {
                event->valid &= ~Task->event_flag;
            }
        }
        List = Temp;
    }
    RY_INTERRUPT_ON;
    if(TaskUpdate == RY_TRUE)
        ry_scheduler();
}

/**
 * �������¼�����
 *
 **/
ry_u8_t ry_event_rec(ry_event_t *event, ry_u8_t mode, ry_u8_t flag, ry_int16_t time)
{
    ry_u8_t Flag;
    ry_task_t *Task;
    RY_NEW_IT_VARI;
    
    Flag   = RY_NULL;
    RY_INTERRUPT_OFF;
    
    /* ���浱ǰ�¼������� */
    Task = ry_get_task();
    Task->event_mode = mode;
    Task->event_flag = flag;
    /* ��ע���¼��Ѳ��� */
    if(_ry_event_check(event, Task) == RY_OK)
    {
        Flag = Task->event_flag;
        if((mode & RY_RC) == RY_RC)
            event->valid &= ~flag;
    }
    else
    {
        /* �ȴ��¼����� */
        RY_INTERRUPT_ON;
        if(ry_wait_obj((ry_obj_t *)event, time) == RY_OK)
        {
            Flag = Task->event_flag;
        }
    }
    RY_INTERRUPT_ON;
    return Flag;
}



#endif



