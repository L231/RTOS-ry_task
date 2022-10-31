
#include "ry_type.h"
#include "ry_hw.h"
#include "stm32f10x_rcc.h"



__weak void ry_systick_cfg(void)
{
	RCC_ClocksTypeDef  mcu_clk;
	RCC_GetClocksFreq(&mcu_clk);
	SysTick_Config(mcu_clk.SYSCLK_Frequency / 1000000 * RY_TICK_PERIOD);
}

void ry_start_hook(void)
{
	RY_PRINTF("\r\n\r\n");
	RY_PRINTF("        - ry_task -\r\n");
	RY_PRINTF("%s", RY_AUTHOR);
	RY_PRINTF("    @version  %s\r\n", RY_STRING(RY_VERSION));
	RY_PRINTF("    @date     %s,%s\r\n", __DATE__, __TIME__);
	RY_PRINTF("\r\n =>ry_task run...\r\n\r\n");
}






#define  DWT_CR               *(uint32_t *)0xE0001000
#define  DWT_CYCCNT           *(uint32_t *)0xE0001004
#define  DEM_CR               *(uint32_t *)0xE000EDFC

#define  DEM_CR_TRCENA        (1 << 24)
#define  DWT_CR_CYCCNTENA     (1 << 0)

void ry_time_point_start(void)
{
	DEM_CR     |= (uint32_t)DEM_CR_TRCENA;
	DWT_CYCCNT  = 0;
	DWT_CR     |= (uint32_t)DWT_CR_CYCCNTENA;
}
void ry_time_point_end(void)
{
	ry_u32_t dwt = DWT_CYCCNT;
	ry_u32_t time;
	
	time = dwt/72;
	if(time > 999999)
		RY_PRINTF("@%d.%ds\r\n", time/1000000, time%1000000);
	else if(time > 999)
		RY_PRINTF("@%d.%dms\r\n", time/1000, time%1000);
	else
		RY_PRINTF("@%d.%dus\r\n", dwt/72, dwt%72);
}

