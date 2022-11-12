
#include "ry_type.h"



/* 新任务 */
ry_task_t *ryNewTask;

extern void ry_task_switch_first(ry_cpu_t next);
extern void ry_task_switch(void);

extern ry_task_t *ry_get_new_task(void);

extern ry_task_t *ryCurrentTask;


/**
 * 描述：启动任务调度
 *
 **/
void ry_scheduler_start(void)
{
	ry_task_switch_first((ry_cpu_t)ry_get_new_task());
}

/**
 * 描述：任务调度
 *
 **/
void ry_scheduler(void)
{
	ryNewTask = ry_get_new_task();
	if(ryCurrentTask == ryNewTask)
		return;
	ry_task_switch();
}





