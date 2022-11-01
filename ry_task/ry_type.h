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
/* 向上对齐，返回的数可能更大 */
#define  RY_ALIGN(size, align)       (((size) + (align) - 1) & ~((align) - 1))
/* 向下对齐，返回的数可能更小 */
#define  RY_ALIGN_DOWN(size, align)  ((size) & ~((align) - 1))


#define  RY_TRUE                  1
#define  RY_FALSE                 0
#define  RY_NULL                  0

#define  RY_TICK_MAX              0xFFFFFFFF
#define  RY_TICK_MAX_DIV_2        (RY_TICK_MAX >> 1)

#define  RY_R                     0  /* 只读     */
#define  RY_W                     1  /* 只写     */
#define  RY_RW                    2  /* 读写     */
#define  RY_RC                    3  /* 阅后即焚 */

#define  RY_MAX(a, b)             (((a) > (b)) ? (a) : (b))
#define  RY_MIN(a, b)             (((a) < (b)) ? (a) : (b))


#define  _STRING(s)                  #s
#define  RY_STRING(s)                _STRING(s)
#define  _CONS(s1, s2)               s1##s2
#define  RY_CONS(s1, s2)             _CONS(s1, s2)

#define  TASK_CONTAINER_OF(ptr, type, member)  \
        ((type *)((char *)(ptr) - (ry_cpu_t)(&((type *)0)->member)))



/* 单向链表 */
typedef struct ry_list_single ry_slist_t;
struct ry_list_single
{
    ry_slist_t          *next;
};

/* 双向链表 */
typedef struct ry_list ry_list_t;
struct ry_list
{
    ry_list_t            *next;
    ry_list_t            *prev;
};


/* 定时器控制块 */
typedef struct
{
    ry_u32_t              delay;
    ry_u8_t               delay_type;
    ry_list_t             delay_list;
    void                (*delay_func)(void *);
    void                 *delay_param;
    ry_u32_t              time;                    /* 延时时长 */
}ry_timer_t;


/* 任务控制块 */
typedef struct
{
    void                 *sp;                      /* 任务栈指针，必须放首位 */
    char                 *name;
    void                (*func)(void *);
    void                 *param;
    ry_u8_t               status;                  /* 任务状态 */
    ry_u8_t               err;                     /* 错误码 */
    ry_u8_t               priority;                /* 优先级 */
    ry_u32_t              remaining_tick;          /* 剩余时间片 */
    ry_u32_t              init_tick;               /* 初始化的时间片数值 */
    
    void                 *stack_addr;              /* 任务栈的首地址 */
    ry_u32_t              stack_size;              /* 任务栈大小 */
    
    ry_u32_t              delay;                   /* 延时时间 */
    ry_u8_t               delay_type;              /* 延时类型 */
    ry_list_t             delay_list;              /* 链表节点，挂于延时链表上 */
    void                (*delay_func)(void *);     /* 超时处理函数 */
    void                 *delay_param;             /* 超时处理函数形参 */
    
#if RY_EVENT == 1
    ry_u8_t               event_mode;              /* 缓存事件模式 */
    ry_u8_t               event_flag;              /* 缓存事件本体 */
#endif
    ry_list_t             list;                    /* 节点，挂于就绪、挂起链表 */
}ry_task_t;


#if RY_MEMPOOL == 1
/* 静态内存(内存池形式)分配结构体 */
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


/* 对象 */
typedef struct
{
    ry_u8_t               valid;                   /* 有效数值 */
    ry_u16_t              type;                    /* 对象类型 */
    ry_list_t             suspend_list;            /* 挂起链表 */
}ry_obj_t;

/* 任务通信控制块 */
typedef struct
{
    ry_u8_t               valid;
    ry_u16_t              type;
    ry_list_t             suspend_list;
    ry_u8_t               max;                     /* 当前邮箱支持的最大邮件数 */
    ry_int8_t             write;                   /* 邮箱采用环形缓冲区方式 */
    ry_int8_t             read;
    ry_u8_t             **msg;
}ry_msg_t;

/* 信号量 */
typedef ry_obj_t ry_semaphore_t;
/* 互斥量 */
typedef struct
{
    ry_u8_t               valid;
    ry_u16_t              type;
    ry_list_t             suspend_list;
    ry_u8_t               priority;                /* 缓存互斥量所有者的优先级 */
    ry_task_t            *task;                    /* 互斥量所有者 */
}ry_mutex_t;

/* 事件 */
typedef ry_obj_t ry_event_t;
//typedef struct
//{
//    ry_u8_t               valid;
//    ry_u16_t              type;
//    ry_list_t             suspend_list;
//}ry_event_t;




typedef enum
{
    RY_OK,               /* 正常无错误 */
    RY_ERR,              /* 错误 */
    RY_ERR_REG,          /* 注册出错 */
    RY_ERR_STATUS,       /* 状态错误 */
    RY_ERR_START,        /* ry_task启动失败 */
    RY_ERR_TIMEOUT,      /* 任务等待超时 */
    RY_ERR_IPC_SEM,      /* 任务非法操作IPC */
    RY_ERR_MEM,          /* 内存申请失败 */
    
}ry_err_t;

typedef enum
{
//    RY_RUN,              /* 运行态 */
    RY_READY,            /* 就绪态 */
    RY_SUSPEND,          /* 挂起态 */
    RY_DIE,              /* 删除消失了啊 */
    
}ry_status_t;


/* 对象类型 */
#define  RY_OBJ_MASK             0xF000      /* 对象类型掩码 */
#define  RY_OBJ_MSG              0x0000      /* 邮箱 */
#define  RY_OBJ_SEM              0x1000      /* 信号量 */
#define  RY_OBJ_MUTEX            0x2000      /* 互斥量 */
#define  RY_OBJ_EVENT            0x3000      /* 事件 */

/* 对象挂起链表的类型 */
#define  RY_OBJ_LIST_MASK        0x0F00
#define  RY_OBJ_LIST_FIFO        0x0000      /* 以FIFO方式插入挂起链表 */
#define  RY_OBJ_LIST_PRIO        0x0100      /* 以优先级方式 */

/* 任务优先级的继承机制 */
#define  RY_OBJ_PRIO_MASK        0x00F0
#define  RY_OBJ_PRIO_NORMAL      0x0000      /* 正常的，无优先级继承 */
#define  RY_OBJ_PRIO_INHERIT     0x0010      /* 优先级继承 */

/* 邮箱类型 */
#define  RY_MSG_FIFO            (RY_OBJ_MSG|RY_OBJ_LIST_FIFO)
#define  RY_MSG_RPIO            (RY_OBJ_MSG|RY_OBJ_LIST_PRIO)

/* 信号量类型 */
#define  RY_SEM_FIFO_NORMAL     (RY_OBJ_SEM|RY_OBJ_LIST_FIFO|RY_OBJ_PRIO_NORMAL)
//#define  RY_SEM_FIFO_INHERIT    (RY_OBJ_SEM|RY_OBJ_LIST_FIFO|RY_OBJ_PRIO_INHERIT)
#define  RY_SEM_PRIO_NORMAL     (RY_OBJ_SEM|RY_OBJ_LIST_PRIO|RY_OBJ_PRIO_NORMAL)
//#define  RY_SEM_PRIO_INHERIT    (RY_OBJ_SEM|RY_OBJ_LIST_PRIO|RY_OBJ_PRIO_INHERIT)

/* 互斥量类型 */
#define  RY_LOCK_FIFO_INHERIT   (RY_OBJ_MUTEX|RY_OBJ_LIST_FIFO|RY_OBJ_PRIO_INHERIT)
#define  RY_LOCK_PRIO_INHERIT   (RY_OBJ_MUTEX|RY_OBJ_LIST_PRIO|RY_OBJ_PRIO_INHERIT)

/* 事件类型 */
#define  RY_EVENT_FIFO          (RY_OBJ_EVENT|RY_OBJ_LIST_FIFO)
#define  RY_ENENT_PRIO          (RY_OBJ_EVENT|RY_OBJ_LIST_PRIO)


/* 邮箱发送邮件的优先级 */
#define  RY_MSG_SEND_NORMAL      0
#define  RY_MSG_SEND_PRIO        1

/* 事件的操作模式 */

#define  RY_ENENT_MASK           0xF0
#define  RY_ENENT_AND            0x00
#define  RY_ENENT_OR             0x10

#define  RY_ENENT_AND_R         (RY_ENENT_AND|RY_R)
#define  RY_ENENT_AND_RC        (RY_ENENT_AND|RY_RC)
#define  RY_ENENT_OR_R          (RY_ENENT_OR|RY_R)
#define  RY_ENENT_OR_RC         (RY_ENENT_OR|RY_RC)

/* 定时器类型 */
#define  RY_TIMER_ONE            0      /* 单次的，一般用于延时 */
#define  RY_TIMER_CYCLE          1      /* 周期性的 */









#endif
