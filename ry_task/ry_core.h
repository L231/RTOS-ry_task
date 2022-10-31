#ifndef __RY_CORE_H__
	#define	__RY_CORE_H__


#include "ry_type.h"


extern ry_u32_t ry_interrupt_off(void);
extern void ry_interrupt_on(ry_u32_t cmd);

#define  RY_NEW_IT_VARI       ry_u32_t cmd
#define  RY_INTERRUPT_OFF     cmd = ry_interrupt_off()
#define  RY_INTERRUPT_ON      ry_interrupt_on(cmd)


ry_inline ry_u8_t ry_get_ready_task_pos(ry_u32_t p)
{
		p = (p - 1) & ~p;
		p = (p & 0x55555555) + ((p >> 1) & 0x55555555);
		p = (p & 0x33333333) + ((p >> 2) & 0x33333333);
		p = (p & 0x0F0F0F0F) + ((p >> 4) & 0x0F0F0F0F);
		return ((p&0x00FF) + ((p&0xFF00) >> 8) + ((p&0xFF0000) >> 16) + ((p&0xFF000000) >> 24));
}
#define  RY_GET_READY_TASK_POS(flag)  ry_get_ready_task_pos(flag)


extern ry_task_t *ry_get_task(void);
extern void ry_scheduler(void);

extern void ry_task_discard(void);
extern ry_u8_t ry_task_recover(ry_list_t *list);

extern ry_u8_t ry_task_suspend(ry_obj_t *obj);
extern ry_u8_t ry_wait_obj(ry_obj_t *obj, ry_int16_t time);








#endif
