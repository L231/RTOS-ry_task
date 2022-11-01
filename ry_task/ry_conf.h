#ifndef __RY_CONF_H__
    #define    __RY_CONF_H__


/* 当前版本号 */
#define  RY_VERSION                              V0.15

/* 作者 */
//#define  RY_AUTHOR                               1225
#define  RY_AUTHOR   ".               _\r\n            \
_ _|_|\r\n          _|_|_|_\r\n         |_|   |_|\r\n"


/* 优先级个数 */
#define  RY_PRIORITY_NUM                        (32)

/* Tick周期，x(us)的周期 */
#define  RY_TICK_PERIOD                         (1000)

#define  RY_DEBUG                               (1)

#define  RY_UART_BAUDRATE                       (115200)





/**
 * 任务间通信
 **/
/* 邮箱 */
#define  RY_MSG                                 (1)
/* 信号量、互斥量 */
#define  RY_SEM                                 (1)
/* 事件 */
#define  RY_EVENT                               (1)

/* IPC邮箱数量 */
#define  RY_IPC_MSG_NUM                         (5)




/* 内存对齐的字节数 */
#define  RY_ALIGN_SIZE                          (4)

/* 内存管理使用了互斥量，增加判断 */
#if RY_SEM == 1
/* 动态内存管理使能 */
#define  RY_MEM                                 (1)
#if RY_MEM == 1
/* 动态内存管理分配的堆空间 4 * N 个字节 */
#define  RY_HEAP_SIZE                           (1024 * 2)
/* 静态内存管理内存池形式使能 */
#define  RY_MEMPOOL                             (1)
#endif
#endif





#if RY_DEBUG == 1
#define  RY_DBG_PRINTF(p, msg)         p msg
#else
#define  RY_DBG_PRINTF(p, msg)
#endif







#endif
