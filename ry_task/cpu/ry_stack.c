

#include "ry_type.h"


typedef struct
{
	/* �쳣ʱ�ֶ�����ļĴ��� */
	ry_u32_t     r4;
	ry_u32_t     r5;
	ry_u32_t     r6;
	ry_u32_t     r7;
	ry_u32_t     r8;
	ry_u32_t     r9;
	ry_u32_t     r10;
	ry_u32_t     r11;
	/* �쳣ʱ�Զ�����ļĴ��� */
	ry_u32_t     r0;
	ry_u32_t     r1;
	ry_u32_t     r2;
	ry_u32_t     r3;
	ry_u32_t     r12;
	ry_u32_t     lr;
	ry_u32_t     pc;
	ry_u32_t     psr;
}ry_stack_t;



/**
 * ����������ջ��ʼ��
 *
 **/
ry_u8_t *ry_stack_init(void *entry, void *param, ry_u8_t *stack_addr)
{
	ry_u32_t    *Pos;
	ry_u32_t     Addr;
	ry_stack_t  *Stack;
	
	/* ���¶��룬ֻ��Addr < stack_addr */
	Addr  = RY_ALIGN_DOWN((ry_u32_t)(stack_addr + sizeof(ry_u32_t)), 8);
	Stack = (ry_stack_t *)(Addr - sizeof(ry_stack_t));
	/* ���Ĵ����趨��ʼֵ */
	for(Pos = (ry_u32_t *)Stack; (ry_u32_t)Pos < Addr; Pos++)
	{
		*Pos = 0x12252512;
	}
	Stack->r0  = (ry_u32_t)param;
	Stack->r1  = 0;
	Stack->r2  = 0;
	Stack->r3  = 0;
	Stack->r12 = 0;
	Stack->lr  = 0;
	Stack->pc  = (ry_u32_t)entry;
	Stack->psr = 0x01000000L;
	return (ry_u8_t *)Stack;
}





