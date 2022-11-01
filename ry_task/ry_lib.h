#ifndef __RY_LIB_H__
    #define    __RY_LIB_H__


#include "ry_type.h"





extern void ry_init(void);
extern void ry_start(void);

extern void ry_task_reg(ry_task_t  *task,
                        char       *name,
                        void      (*func)(void *),
                        void       *param,
                        ry_u8_t     priority,
                        ry_u32_t    tick,
                        void       *stack_addr,
                        ry_u32_t    stack_size);
#if RY_MEM == 1
extern ry_task_t *ry_task_create(char       *name,
                                 void      (*func)(void *),
                                 void       *param,
                                 ry_u8_t     priority,
                                 ry_u32_t    tick,
                                 ry_u32_t    stack_size);
extern void       ry_task_delete(ry_task_t *task);
#endif
extern ry_task_t *ry_get_task(void);
                                                                 
extern ry_u32_t    ry_get_tick(void);
extern void        ry_task_delay(ry_u32_t time);
extern void        ry_timer_reg(ry_timer_t *timer, void (*timeout)(void*),
                                void *param, ry_u32_t time, ry_u8_t type);
#if RY_MEM == 1
extern ry_timer_t *ry_timer_create(void (*timeout)(void*), void *param, ry_u32_t time, ry_u8_t type);
extern void        ry_timer_delete(ry_timer_t *timer);
#endif

#if RY_MEM == 1
extern void *ry_malloc(ry_u32_t size);
extern void  ry_free(void *mem);
#endif

#if RY_MEMPOOL == 1
extern void          ry_mempool_reg(ry_mempool_t *mem, void *start, ry_u32_t size, ry_u32_t block_size);
extern ry_mempool_t *ry_mempool_create(ry_u32_t block_num, ry_u32_t block_size);
extern void          ry_mempool_delete(ry_mempool_t *mem);
extern void         *ry_mempool_malloc(ry_mempool_t *mem, ry_int16_t time);
extern void          ry_mempool_free(void *block);
#endif


#if RY_MSG == 1
extern void      ry_msg_reg(ry_msg_t *msg, ry_u16_t type, ry_u8_t max, ry_u8_t **buf);
#if RY_MEM == 1
extern ry_msg_t *ry_msg_create(ry_u16_t type, ry_u8_t max);
extern void      ry_msg_delete(ry_msg_t *msg);
#endif
extern ry_u8_t   ry_msg_send(ry_msg_t *msg, ry_u8_t mode, ry_u8_t *buf);
extern ry_u8_t  *ry_msg_rec(ry_msg_t *msg, ry_u8_t mode, ry_int16_t time);
#endif

#if RY_SEM == 1
extern void    ry_semaphore_reg(ry_semaphore_t *semaphore, ry_u16_t type, ry_u8_t valid);
extern void    ry_mutex_reg(ry_mutex_t *mutex, ry_u16_t type);
extern void    ry_semaphore_send(ry_semaphore_t *semaphore);
extern ry_u8_t ry_mutex_release(ry_mutex_t *mutex);
extern ry_u8_t ry_sem_release(void *sem);
extern ry_u8_t ry_semaphore_rec(ry_semaphore_t *semaphore, ry_int16_t time);
extern ry_u8_t ry_mutex_rec(ry_mutex_t *mutex, ry_int16_t time);
extern ry_u8_t ry_sem_rec(void *sem, ry_int16_t time);
#endif

#if RY_EVENT == 1
extern void    ry_event_reg(ry_obj_t *event, ry_u16_t type);
extern void    ry_event_send(ry_event_t *event, ry_u8_t flag);
extern ry_u8_t ry_event_rec(ry_event_t *event, ry_u8_t mode, ry_u8_t flag, ry_int16_t time);
#endif


#endif
