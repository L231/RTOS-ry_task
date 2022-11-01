#ifndef __RY_TYPE_H__
    #define    __RY_TYPE_H__

#include "ry_conf.h"






typedef signed   char             ry_int8_t;
typedef signed   short            ry_int16_t;
typedef signed   int              ry_int32_t;
typedef unsigned char             ry_u8_t;
typedef unsigned short            ry_u16_t;
typedef unsigned int              ry_u32_t;
typedef unsigned int              ry_cpu_t;

#define  ry_inline                static __inline

#define  ALIGN(n)                    __attribute__((aligned(n)))
/* ���϶��룬���ص������ܸ��� */
#define  RY_ALIGN(size, align)       (((size) + (align) - 1) & ~((align) - 1))
/* ���¶��룬���ص������ܸ�С */
#define  RY_ALIGN_DOWN(size, align)  ((size) & ~((align) - 1))


#define  RY_TRUE                  1
#define  RY_FALSE                 0
#define  RY_NULL                  0

#define  RY_TICK_MAX              0xFFFFFFFF
#define  RY_TICK_MAX_DIV_2        (RY_TICK_MAX >> 1)

#define  RY_R                     0  /* ֻ��     */
#define  RY_W                     1  /* ֻд     */
#define  RY_RW                    2  /* ��д     */
#define  RY_RC                    3  /* �ĺ󼴷� */

#define  RY_MAX(a, b)             (((a) > (b)) ? (a) : (b))
#define  RY_MIN(a, b)             (((a) < (b)) ? (a) : (b))


#define  _STRING(s)                  #s
#define  RY_STRING(s)                _STRING(s)
#define  _CONS(s1, s2)               s1##s2
#define  RY_CONS(s1, s2)             _CONS(s1, s2)

#define  TASK_CONTAINER_OF(ptr, type, member)  \
        ((type *)((char *)(ptr) - (ry_cpu_t)(&((type *)0)->member)))



/* �������� */
typedef struct ry_list_single ry_slist_t;
struct ry_list_single
{
    ry_slist_t          *next;
};

/* ˫������ */
typedef struct ry_list ry_list_t;
struct ry_list
{
    ry_list_t            *next;
    ry_list_t            *prev;
};


/* ��ʱ�����ƿ� */
typedef struct
{
    ry_u32_t              delay;
    ry_u8_t               delay_type;
    ry_list_t             delay_list;
    void                (*delay_func)(void *);
    void                 *delay_param;
    ry_u32_t              time;                    /* ��ʱʱ�� */
}ry_timer_t;


/* ������ƿ� */
typedef struct
{
    void                 *sp;                      /* ����ջָ�룬�������λ */
    char                 *name;
    void                (*func)(void *);
    void                 *param;
    ry_u8_t               status;                  /* ����״̬ */
    ry_u8_t               err;                     /* ������ */
    ry_u8_t               priority;                /* ���ȼ� */
    ry_u32_t              remaining_tick;          /* ʣ��ʱ��Ƭ */
    ry_u32_t              init_tick;               /* ��ʼ����ʱ��Ƭ��ֵ */
    
    void                 *stack_addr;              /* ����ջ���׵�ַ */
    ry_u32_t              stack_size;              /* ����ջ��С */
    
    ry_u32_t              delay;                   /* ��ʱʱ�� */
    ry_u8_t               delay_type;              /* ��ʱ���� */
    ry_list_t             delay_list;              /* ����ڵ㣬������ʱ������ */
    void                (*delay_func)(void *);     /* ��ʱ������ */
    void                 *delay_param;             /* ��ʱ�������β� */
    
#if RY_EVENT == 1
    ry_u8_t               event_mode;              /* �����¼�ģʽ */
    ry_u8_t               event_flag;              /* �����¼����� */
#endif
    ry_list_t             list;                    /* �ڵ㣬���ھ������������� */
}ry_task_t;


#if RY_MEMPOOL == 1
/* ��̬�ڴ�(�ڴ����ʽ)����ṹ�� */
typedef struct
{
    ry_u8_t              *free_list;
    ry_u8_t              *begin;
    ry_cpu_t              size;
    ry_cpu_t              block_size;
    ry_cpu_t              block_sum;
    ry_cpu_t              block_free_cnt;
    ry_u16_t              suspend_cnt;
    ry_list_t             suspend_list;
}ry_mempool_t;
#endif


/* ���� */
typedef struct
{
    ry_u8_t               valid;                   /* ��Ч��ֵ */
    ry_u16_t              type;                    /* �������� */
    ry_list_t             suspend_list;            /* �������� */
}ry_obj_t;

/* ����ͨ�ſ��ƿ� */
typedef struct
{
    ry_u8_t               valid;
    ry_u16_t              type;
    ry_list_t             suspend_list;
    ry_u8_t               max;                     /* ��ǰ����֧�ֵ�����ʼ��� */
    ry_int8_t             write;                   /* ������û��λ�������ʽ */
    ry_int8_t             read;
    ry_u8_t             **msg;
}ry_msg_t;

/* �ź��� */
typedef ry_obj_t ry_semaphore_t;
/* ������ */
typedef struct
{
    ry_u8_t               valid;
    ry_u16_t              type;
    ry_list_t             suspend_list;
    ry_u8_t               priority;                /* ���滥���������ߵ����ȼ� */
    ry_task_t            *task;                    /* ������������ */
}ry_mutex_t;

/* �¼� */
typedef ry_obj_t ry_event_t;
//typedef struct
//{
//    ry_u8_t               valid;
//    ry_u16_t              type;
//    ry_list_t             suspend_list;
//}ry_event_t;




typedef enum
{
    RY_OK,               /* �����޴��� */
    RY_ERR,              /* ���� */
    RY_ERR_REG,          /* ע����� */
    RY_ERR_STATUS,       /* ״̬���� */
    RY_ERR_START,        /* ry_task����ʧ�� */
    RY_ERR_TIMEOUT,      /* ����ȴ���ʱ */
    RY_ERR_IPC_SEM,      /* ����Ƿ�����IPC */
    RY_ERR_MEM,          /* �ڴ�����ʧ�� */
    
}ry_err_t;

typedef enum
{
//    RY_RUN,              /* ����̬ */
    RY_READY,            /* ����̬ */
    RY_SUSPEND,          /* ����̬ */
    RY_DIE,              /* ɾ����ʧ�˰� */
    
}ry_status_t;


/* �������� */
#define  RY_OBJ_MASK             0xF000      /* ������������ */
#define  RY_OBJ_MSG              0x0000      /* ���� */
#define  RY_OBJ_SEM              0x1000      /* �ź��� */
#define  RY_OBJ_MUTEX            0x2000      /* ������ */
#define  RY_OBJ_EVENT            0x3000      /* �¼� */

/* ���������������� */
#define  RY_OBJ_LIST_MASK        0x0F00
#define  RY_OBJ_LIST_FIFO        0x0000      /* ��FIFO��ʽ����������� */
#define  RY_OBJ_LIST_PRIO        0x0100      /* �����ȼ���ʽ */

/* �������ȼ��ļ̳л��� */
#define  RY_OBJ_PRIO_MASK        0x00F0
#define  RY_OBJ_PRIO_NORMAL      0x0000      /* �����ģ������ȼ��̳� */
#define  RY_OBJ_PRIO_INHERIT     0x0010      /* ���ȼ��̳� */

/* �������� */
#define  RY_MSG_FIFO            (RY_OBJ_MSG|RY_OBJ_LIST_FIFO)
#define  RY_MSG_RPIO            (RY_OBJ_MSG|RY_OBJ_LIST_PRIO)

/* �ź������� */
#define  RY_SEM_FIFO_NORMAL     (RY_OBJ_SEM|RY_OBJ_LIST_FIFO|RY_OBJ_PRIO_NORMAL)
//#define  RY_SEM_FIFO_INHERIT    (RY_OBJ_SEM|RY_OBJ_LIST_FIFO|RY_OBJ_PRIO_INHERIT)
#define  RY_SEM_PRIO_NORMAL     (RY_OBJ_SEM|RY_OBJ_LIST_PRIO|RY_OBJ_PRIO_NORMAL)
//#define  RY_SEM_PRIO_INHERIT    (RY_OBJ_SEM|RY_OBJ_LIST_PRIO|RY_OBJ_PRIO_INHERIT)

/* ���������� */
#define  RY_LOCK_FIFO_INHERIT   (RY_OBJ_MUTEX|RY_OBJ_LIST_FIFO|RY_OBJ_PRIO_INHERIT)
#define  RY_LOCK_PRIO_INHERIT   (RY_OBJ_MUTEX|RY_OBJ_LIST_PRIO|RY_OBJ_PRIO_INHERIT)

/* �¼����� */
#define  RY_EVENT_FIFO          (RY_OBJ_EVENT|RY_OBJ_LIST_FIFO)
#define  RY_ENENT_PRIO          (RY_OBJ_EVENT|RY_OBJ_LIST_PRIO)


/* ���䷢���ʼ������ȼ� */
#define  RY_MSG_SEND_NORMAL      0
#define  RY_MSG_SEND_PRIO        1

/* �¼��Ĳ���ģʽ */

#define  RY_ENENT_MASK           0xF0
#define  RY_ENENT_AND            0x00
#define  RY_ENENT_OR             0x10

#define  RY_ENENT_AND_R         (RY_ENENT_AND|RY_R)
#define  RY_ENENT_AND_RC        (RY_ENENT_AND|RY_RC)
#define  RY_ENENT_OR_R          (RY_ENENT_OR|RY_R)
#define  RY_ENENT_OR_RC         (RY_ENENT_OR|RY_RC)

/* ��ʱ������ */
#define  RY_TIMER_ONE            0      /* ���εģ�һ��������ʱ */
#define  RY_TIMER_CYCLE          1      /* �����Ե� */









#endif
