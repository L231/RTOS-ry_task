
#include "ry_type.h"



/* ������ */
ry_task_t *ryNewTask;

extern void ry_task_switch_first(ry_cpu_t next);
extern void ry_task_switch(void);

extern ry_task_t *ry_get_new_task(void);

extern ry_task_t *ryCurrentTask;


/**
 * �����������������
 *
 **/
void ry_scheduler_start(void)
{
	ry_task_switch_first((ry_cpu_t)ry_get_new_task());
}

/**
 * �������������
 *
 **/
void ry_scheduler(void)
{
	ryNewTask = ry_get_new_task();
	if(ryCurrentTask == ryNewTask)
		return;
	ry_task_switch();
}





