
;/**
 ;* 描述：Cortex-M3，任务切换核心
 ;*
 ;**/


SCB_VTOR           EQU         0xE000ED08
NVIC_INT_CTRL      EQU         0xE000ED04
NVIC_SYSPRI2       EQU         0xE000ED20
NVIC_PENDSV_PRI    EQU         0x00FF0000
NVIC_PENDSVSET     EQU         0x10000000
	
	;汇编一个代码段，只读，2^2字节对齐，THUMB指令代码，栈8字节对齐
	AREA |.1225|, CODE, READONLY, ALIGN = 2
	THUMB
	REQUIRE8
	PRESERVE8
	
	;导入外部全局变量
	IMPORT ryCurrentTask
	IMPORT ryNewTask



;/**
; * void ry_interrupt_on(ry_u32_t cmd);
; *
; * 开启中断
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
; * 关闭所有中断
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
; * 第一次任务切换
; *
; * 形参缓存在‘r0’
; **/
ry_task_switch_first    PROC
	;声明它具有全局属性，可被外部文件调用
	EXPORT ry_task_switch_first
		MOV      r3, #0               ;配置r3=0
    MSR      psp, r3              ;配置psp=0

    LDR      r2, = ryNewTask      ;加载当前任务
		STR      r0, [r2]             ;r0缓存了下一个任务，更新为当前任务
		
    ;设置 PendSV 异常的优先级
    LDR      r1, = NVIC_SYSPRI2
    LDR      r2, = NVIC_PENDSV_PRI
    LDR.W    r3, [r1, #0x00]      ; 读
    ORR      r2, r2, r3           ; 改
    STR      r2, [r1]             ; 写

    ;触发 PendSV，产生任务切换
    LDR      r1, = NVIC_INT_CTRL
    LDR      r2, = NVIC_PENDSVSET
    STR      r2, [r1]
		
		;开中断
		CPSIE    F
		CPSIE    I
	ENDP

;/**
; * void ry_task_switch(ry_u32_t next);
; *
; * 任务切换
; *
; * next缓存在‘r0’
; **/
ry_task_switch    PROC
	;声明它具有全局属性，可被外部文件调用
	EXPORT ry_task_switch
    LDR      r2, = ryNewTask          ;加载'NewTask'指针的地址
		STR      r0, [r2]                 ;r0缓存了最新任务的地址，放置于'NewTask'
		LDR      r1, = NVIC_INT_CTRL
		LDR      r2, = NVIC_PENDSVSET
		STR      r2, [r1]                 ;触发 PendSV，产生任务切换
		BX       lr                       ;子函数返回
	ENDP

;/**
; * void PendSV_Handler(void);
; *
; **/
PendSV_Handler    PROC
	;声明它具有全局属性，可被外部文件调用
	EXPORT PendSV_Handler
		MRS      r3, PRIMASK
		CPSID    I                        ;失能中断，避免该中断处理被抢占
		
    LDR      r2, = ryCurrentTask      ;加载当前任务指针的地址
		;保存上一个任务
		MRS      r1, psp                  ;获取psp
    CBZ      r1, _switch_task         ;psp==0，跳转直接启动下一任务
    STMFD    r1!, {r4 - r11}          ;缓存r4--r11，r1地址自减
    LDR      r0, [r2]                 ;r2指向的地址存储当前任务的'task->sp'，加载到r0
    STR      r1, [r0]                 ;更新当前'task->sp'
		
		;切换下一个任务
_switch_task
    ;LDR      r2, = ryCurrentTask      ;加载当前任务
    LDR      r0, = ryNewTask          ;加载下一个任务指针的地址
    LDR      r0, [r0]                 ;r0指向的地址存储'task->sp'，加载到r0
		STR      r0, [r2]                 ;r0缓存了下一个任务，更新为当前任务
		
    LDR      r0, [r0]                 ;'task->sp'是任务栈指针，把它指向的内容加载到r0
    LDMFD    r0!, {r4 - r11}          ;加载r4--r11，r0地址自增
    MSR      psp, r0                  ;最新任务的栈顶加载到PSP
		
    ; 恢复中断
    MSR     PRIMASK, r3
    ORR     lr, lr, #0x04             ;确保返回时使用的堆栈指针是PSP，即LR的bit2必须为1
    BX      lr
	ENDP






  ALIGN   4    ;当前文件的代码要求4字节对齐
	END          ;结束

