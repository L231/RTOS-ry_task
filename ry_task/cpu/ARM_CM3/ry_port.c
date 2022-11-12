
#include "ry_timer.h"
#include "ry_core.h"
#include "ry_port.h"

#include "stm32f10x_rcc.h"





typedef struct
{
	/* �쳣ʱ�ֶ�����ļĴ��� */
	ry_cpu_t     r4;
	ry_cpu_t     r5;
	ry_cpu_t     r6;
	ry_cpu_t     r7;
	ry_cpu_t     r8;
	ry_cpu_t     r9;
	ry_cpu_t     r10;
	ry_cpu_t     r11;
	/* �쳣ʱ�Զ�����ļĴ��� */
	ry_cpu_t     r0;
	ry_cpu_t     r1;
	ry_cpu_t     r2;
	ry_cpu_t     r3;
	ry_cpu_t     r12;
	ry_cpu_t     lr;
	ry_cpu_t     pc;
	ry_cpu_t     psr;
}ry_stack_t;




/**
 * ����������ջ��ʼ��
 *
 **/
ry_u8_t *ry_stack_init(void *entry, void *param, ry_u8_t *stack_addr)
{
	ry_cpu_t    *Pos;
	ry_cpu_t     Addr;
	ry_stack_t  *Stack;
	
	/* ���¶��룬ֻ��Addr < stack_addr */
	Addr  = RY_ALIGN_DOWN((stack_addr + sizeof(ry_cpu_t)), 8);
	Stack = (ry_stack_t *)(Addr - sizeof(ry_stack_t));
	/* ���Ĵ����趨��ʼֵ */
	for(Pos = (ry_cpu_t *)Stack; (ry_cpu_t)Pos < Addr; Pos++)
	{
		*Pos = 0x12252512;
	}
	Stack->r0  = (ry_cpu_t)param;
	Stack->r1  = 0;
	Stack->r2  = 0;
	Stack->r3  = 0;
	Stack->r12 = 0;
	Stack->lr  = 0;
	Stack->pc  = (ry_cpu_t)entry;
	Stack->psr = 0x01000000L;
	return (ry_u8_t *)Stack;
}



/**
 * ������rtos����ǰ��ӡ��Ϣ
 *
 **/
void ry_start_hook(void)
{
	ry_printf("\r\n\r\n");
	ry_printf("        - ry_task -\r\n");
	ry_printf("%s", RY_AUTHOR);
	ry_printf("    @version  %s\r\n", RY_STRING(RY_VERSION));
	ry_printf("    @date     %s,%s\r\n", __DATE__, __TIME__);
	ry_printf("\r\n =>ry_task run...\r\n\r\n");
}




/**
 * �������������д���㣬��ʼ��ʱ
 *
 **/
void ry_time_point_start(void)
{
	DEM_CR     |= (uint32_t)DEM_CR_TRCENA;
	DWT_CYCCNT  = 0;
	DWT_CR     |= (uint32_t)DWT_CR_CYCCNTENA;
}
/**
 * ������������ʱ����ӡ����ε�����ʱ��
 *
 **/
void ry_time_point_end(void)
{
	ry_u32_t dwt = DWT_CYCCNT;
	ry_u32_t time;
	
	time = dwt/72;
	if(time > 999999)
		ry_printf("@%d.%ds\r\n", time/1000000, time%1000000);
	else if(time > 999)
		ry_printf("@%d.%dms\r\n", time/1000, time%1000);
	else
		ry_printf("@%d.%dus\r\n", dwt/72, dwt%72);
}



ry_weak void ry_systick_cfg(void)
{
	/* ����ϵͳʱ�� */
}


/**
 * ������ϵͳʱ�����жϴ�������ά��rtos������
 *
 **/
void SysTick_Handler(void)
{
	ry_tick_handler();
	ry_scheduler();
}


ry_weak void ry_printf(const char * fmt, ...)
{
	
}

