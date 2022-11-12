
#include "ry_lib.h"



#if RY_MEM != 1
static ry_task_t taskIdle;
static ry_u8_t   stackIdle[RY_IDLE_STACK_SIZE];
#endif


static ry_u32_t   ryIdleRunCnt;

extern ry_u8_t   ryCPU;



/**
 * 描述：空闲任务
 *
 **/
ry_weak void ry_idle_task(void *p)
{
	RY_NEW_IT_VARI;
	ry_u32_t temp = 0, TickOld = 0x12251225;
	while(1)
	{
		/* 时基变化了才进行操作 */
		if(temp != ry_get_tick())
		{
			temp = ry_get_tick();
			/* 记录空闲时长 */
			ryIdleRunCnt++;
			/* 运行时长达1000个时基 */
			if(ry_get_tick() - TickOld >= 1000)
			{
				ry_u32_t sum;
				RY_INTERRUPT_OFF;
				/* CPU使用率 = （总时长 - 空闲时长）/ 总时长 */
				sum = ry_get_tick() - TickOld;
				ryCPU = (sum - ryIdleRunCnt) * 100 / sum;
				RY_DBG_PRINTF(ry_printf, ("[cpu:%d%%]\r\n", ryCPU));
				RY_INTERRUPT_ON;
				TickOld      = ry_get_tick();
				ryIdleRunCnt = 0;
			}
		}
	}
}

void ry_idle_init(void)
{
	ryIdleRunCnt = 0;
#if RY_MEM == 1
	ry_task_create("task_idle",
									ry_idle_task,
									0,
									RY_PRIORITY_NUM-1,
									0,
									RY_IDLE_STACK_SIZE);
#else
	ry_task_reg(&taskIdle,
							"task_idle",
							ry_idle_task,
							0,
							RY_PRIORITY_NUM-1,
							0,
							stackIdle,
							RY_IDLE_STACK_SIZE);
#endif
}


ry_u32_t ry_get_idle_cnt(void)
{
	return ryIdleRunCnt;
}


