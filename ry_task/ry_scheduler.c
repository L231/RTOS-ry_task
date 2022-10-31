
#include "ry_type.h"



/* 新任务 */
ry_task_t *ryNewTask;

extern void ry_task_switch_first(ry_u32_t next);
extern void ry_task_switch(ry_u32_t next);

extern ry_task_t *ry_get_new_task(void);

extern ry_task_t *ryCurrentTask;


/**
 * 描述：启动任务调度
 *
 **/
void ry_scheduler_start(void)
{
	ry_task_switch_first((ry_u32_t)ry_get_new_task());
}

/**
 * 描述：任务调度
 *
 **/
void ry_scheduler(void)
{
	ry_task_t  *Task = ry_get_new_task();
	if(ryCurrentTask == Task)
		return;
	ry_task_switch((ry_u32_t)Task);
}





