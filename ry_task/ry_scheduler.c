
#include "ry_type.h"



/* ������ */
ry_task_t *ryNewTask;

extern void ry_task_switch_first(ry_u32_t next);
extern void ry_task_switch(ry_u32_t next);

extern ry_task_t *ry_get_new_task(void);

extern ry_task_t *ryCurrentTask;


/**
 * �����������������
 *
 **/
void ry_scheduler_start(void)
{
	ry_task_switch_first((ry_u32_t)ry_get_new_task());
}

/**
 * �������������
 *
 **/
void ry_scheduler(void)
{
	ry_task_t  *Task = ry_get_new_task();
	if(ryCurrentTask == Task)
		return;
	ry_task_switch((ry_u32_t)Task);
}





