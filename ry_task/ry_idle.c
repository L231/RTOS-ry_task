
#include "ry_lib.h"
#include "ry_core.h"
#include "ry_hw.h"





static ry_u32_t   ryIdleRunCnt;

extern ry_u8_t   ryCPU;



/**
 * 描述：空闲任务
 *
 **/
static void ry_idle_task(void *p)
{
	RY_NEW_IT_VARI;
	ry_u32_t temp = 0, TickOld = 0xFFFFFF00;
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
				RY_DBG_PRINTF(RY_PRINTF, ("[cpu:%d%%]\r\n", ryCPU));
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
	ry_task_create("task_idle",
									ry_idle_task,
									0,
									RY_PRIORITY_NUM-1,
									0,
									128);
}


ry_u32_t ry_get_idle_cnt(void)
{
	return ryIdleRunCnt;
}


