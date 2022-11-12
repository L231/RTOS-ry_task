
#include <p33EP256MC506.h>
#include "ry_core.h"
#include "ry_timer.h"
#include "ry_port.h"



typedef struct
{
	ry_cpu_t     pcl;
	ry_cpu_t     pch;
	ry_cpu_t     sr;
	ry_cpu_t     w0;
    
	ry_cpu_t     w1;
	ry_cpu_t     w2;
	ry_cpu_t     w3;
	ry_cpu_t     w4;
	ry_cpu_t     w5;
	ry_cpu_t     w6;
	ry_cpu_t     w7;
	ry_cpu_t     w8;
	ry_cpu_t     w9;
	ry_cpu_t     w10;
	ry_cpu_t     w11;
	ry_cpu_t     w12;
	ry_cpu_t     w13;
	ry_cpu_t     w14;
	ry_cpu_t     rcount;
	ry_cpu_t     tblpag;
    
	ry_cpu_t     accal;
	ry_cpu_t     accah;
	ry_cpu_t     accau;
	ry_cpu_t     accbl;
	ry_cpu_t     accbh;
	ry_cpu_t     accbu;
	ry_cpu_t     dcount;
	ry_cpu_t     dostartl;
	ry_cpu_t     dostarth;
	ry_cpu_t     doendl;
	ry_cpu_t     doendh;
    
	ry_cpu_t     corcon;
	ry_cpu_t     dsrpag;
	ry_cpu_t     dswpag;
}ry_stack_t;





ry_u8_t *ry_stack_init( void *entry, void *param, ry_u8_t *stack_addr )
{
	ry_cpu_t    Max;
    ry_cpu_t   *Pos;
    ry_stack_t *Stack;
    
	Stack = (ry_stack_t *)RY_ALIGN((ry_cpu_t)(stack_addr), RY_ALIGN_SIZE);
	Max   = (ry_cpu_t)Stack + sizeof(ry_stack_t);
	/* 给寄存器设定初始值 */
	for(Pos = (ry_cpu_t *)Stack; Pos < (ry_cpu_t *)Max; Pos++)
	{
		*Pos = 0x1225;
	}
    Stack->pcl      = (ry_cpu_t)entry;
    Stack->pch      = 0;
    Stack->sr       = SR;
    Stack->w0       = (ry_cpu_t)param;
    Stack->corcon   = CORCON;
    Stack->dsrpag   = DSRPAG;
    Stack->dswpag   = DSWPAG;
	return (ry_u8_t *)Max;
}

ry_cpu_t ry_interrupt_off( void )
{
    ry_cpu_t status = INTCON2bits.GIE;
	INTCON2bits.GIE = 0;
    return status;
}
/*-----------------------------------------------------------*/

void ry_interrupt_on( ry_cpu_t cmd )
{
    INTCON2bits.GIE = cmd;
}
/*-----------------------------------------------------------*/

void __attribute__((interrupt, auto_psv)) _T1Interrupt()
{
    ry_tick_handler();
	/* Clear the timer interrupt. */
	IFS0bits.T1IF = 0;
    ry_scheduler();
}




ry_weak void ry_systick_cfg(void)
{
	T1CONbits.TON = 1;
}

/**
 * 描述：rtos启动前打印信息
 *
 **/
void ry_start_hook(void)
{
	RY_PRINTF("\r\n\r\n");
	RY_PRINTF("        - ry_task -\r\n");
	RY_PRINTF("%s", RY_AUTHOR);
	RY_PRINTF("    @version  %s\r\n", RY_STRING(RY_VERSION));
	RY_PRINTF("    @date     %s,%s\r\n", __DATE__, __TIME__);
	RY_PRINTF("\r\n =>ry_task run...\r\n\r\n");
}

