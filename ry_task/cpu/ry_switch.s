
;/**
 ;* ������Cortex-M3�������л�����
 ;*
 ;**/


SCB_VTOR           EQU         0xE000ED08
NVIC_INT_CTRL      EQU         0xE000ED04
NVIC_SYSPRI2       EQU         0xE000ED20
NVIC_PENDSV_PRI    EQU         0x00FF0000
NVIC_PENDSVSET     EQU         0x10000000
	
	;���һ������Σ�ֻ����2^2�ֽڶ��룬THUMBָ����룬ջ8�ֽڶ���
	AREA |.1225|, CODE, READONLY, ALIGN = 2
	THUMB
	REQUIRE8
	PRESERVE8
	
	;�����ⲿȫ�ֱ���
	IMPORT ryCurrentTask
	IMPORT ryNewTask



;/**
; * void ry_interrupt_on(ry_u32_t cmd);
; *
; * �����ж�
; *
; **/
ry_interrupt_on    PROC
	EXPORT ry_interrupt_on
		MSR      PRIMASK, r0
		BX       lr
		ENDP


;/**
; * ry_u32_t ry_interrupt_off(void);
; *
; * �ر������ж�
; *
; **/
ry_interrupt_off   PROC
	EXPORT ry_interrupt_off
		MRS      r0, PRIMASK
		CPSID    I
		BX       lr
		ENDP



;/**
; * void ry_task_switch_first(ry_u32_t next);
; *
; * ��һ�������л�
; *
; * �βλ����ڡ�r0��
; **/
ry_task_switch_first    PROC
	;����������ȫ�����ԣ��ɱ��ⲿ�ļ�����
	EXPORT ry_task_switch_first
		MOV      r3, #0               ;����r3=0
    MSR      psp, r3              ;����psp=0

    LDR      r2, = ryNewTask      ;���ص�ǰ����
		STR      r0, [r2]             ;r0��������һ�����񣬸���Ϊ��ǰ����
		
    ;���� PendSV �쳣�����ȼ�
    LDR      r1, = NVIC_SYSPRI2
    LDR      r2, = NVIC_PENDSV_PRI
    LDR.W    r3, [r1, #0x00]      ; ��
    ORR      r2, r2, r3           ; ��
    STR      r2, [r1]             ; д

    ;���� PendSV�����������л�
    LDR      r1, = NVIC_INT_CTRL
    LDR      r2, = NVIC_PENDSVSET
    STR      r2, [r1]
		
		;���ж�
		CPSIE    F
		CPSIE    I
	ENDP

;/**
; * void ry_task_switch(ry_u32_t next);
; *
; * �����л�
; *
; * next�����ڡ�r0��
; **/
ry_task_switch    PROC
	;����������ȫ�����ԣ��ɱ��ⲿ�ļ�����
	EXPORT ry_task_switch
    LDR      r2, = ryNewTask          ;����'NewTask'ָ��ĵ�ַ
		STR      r0, [r2]                 ;r0��������������ĵ�ַ��������'NewTask'
		LDR      r1, = NVIC_INT_CTRL
		LDR      r2, = NVIC_PENDSVSET
		STR      r2, [r1]                 ;���� PendSV�����������л�
		BX       lr                       ;�Ӻ�������
	ENDP

;/**
; * void PendSV_Handler(void);
; *
; **/
PendSV_Handler    PROC
	;����������ȫ�����ԣ��ɱ��ⲿ�ļ�����
	EXPORT PendSV_Handler
		MRS      r3, PRIMASK
		CPSID    I                        ;ʧ���жϣ�������жϴ�����ռ
		
    LDR      r2, = ryCurrentTask      ;���ص�ǰ����ָ��ĵ�ַ
		;������һ������
		MRS      r1, psp                  ;��ȡpsp
    CBZ      r1, _switch_task         ;psp==0����תֱ��������һ����
    STMFD    r1!, {r4 - r11}          ;����r4--r11��r1��ַ�Լ�
    LDR      r0, [r2]                 ;r2ָ��ĵ�ַ�洢��ǰ�����'task->sp'�����ص�r0
    STR      r1, [r0]                 ;���µ�ǰ'task->sp'
		
		;�л���һ������
_switch_task
    ;LDR      r2, = ryCurrentTask      ;���ص�ǰ����
    LDR      r0, = ryNewTask          ;������һ������ָ��ĵ�ַ
    LDR      r0, [r0]                 ;r0ָ��ĵ�ַ�洢'task->sp'�����ص�r0
		STR      r0, [r2]                 ;r0��������һ�����񣬸���Ϊ��ǰ����
		
    LDR      r0, [r0]                 ;'task->sp'������ջָ�룬����ָ������ݼ��ص�r0
    LDMFD    r0!, {r4 - r11}          ;����r4--r11��r0��ַ����
    MSR      psp, r0                  ;���������ջ�����ص�PSP
		
    ; �ָ��ж�
    MSR     PRIMASK, r3
    ORR     lr, lr, #0x04             ;ȷ������ʱʹ�õĶ�ջָ����PSP����LR��bit2����Ϊ1
    BX      lr
	ENDP






  ALIGN   4    ;��ǰ�ļ��Ĵ���Ҫ��4�ֽڶ���
	END          ;����

