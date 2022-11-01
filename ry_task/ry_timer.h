#ifndef __RY_TIMER_H__
    #define    __RY_TIMER_H__

#include "ry_type.h"






extern ry_u32_t ry_get_tick(void);
extern ry_list_t *ry_get_delay_list(void);
extern void ry_delay_list_init(void);
extern void ry_task_delay(ry_u32_t time);

extern ry_list_t *ry_get_timer_pos(ry_list_t *n);












#endif
