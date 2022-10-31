
#include "ry_lib.h"
#include "ry_core.h"
#include "ry_hw.h"





static ry_u32_t   ryIdleRunCnt;

extern ry_u8_t   ryCPU;



/**
 * ��������������
 *
 **/
static void ry_idle_task(void *p)
{
	RY_NEW_IT_VARI;
	ry_u32_t temp = 0, TickOld = 0xFFFFFF00;
	while(1)
	{
		/* ʱ���仯�˲Ž��в��� */
		if(temp != ry_get_tick())
		{
			temp = ry_get_tick();
			/* ��¼����ʱ�� */
			ryIdleRunCnt++;
			/* ����ʱ����1000��ʱ�� */
			if(ry_get_tick() - TickOld >= 1000)
			{
				ry_u32_t sum;
				RY_INTERRUPT_OFF;
				/* CPUʹ���� = ����ʱ�� - ����ʱ����/ ��ʱ�� */
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


