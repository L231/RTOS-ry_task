

.extern _ryCurrentTask
.extern _ryNewTask
    
.global _ry_task_switch_first
.global _ry_task_switch
    

_ry_task_switch_first:
    MOV    W0, _ryCurrentTask
    MOV    [W0], W15
    
    POP    DSWPAG
    POP    DSRPAG
    POP    CORCON
    POP    DOENDH
    POP    DOENDL
    POP    DOSTARTH
    POP    DOSTARTL
    POP    DCOUNT
    POP    ACCBU
    POP    ACCBH
    POP    ACCBL
    POP    ACCAU
    POP    ACCAH
    POP    ACCAL
    POP    TBLPAG
    POP    RCOUNT
    POP    W14
    POP.D  W12
    POP.D  W10
    POP.D  W8
    POP.D  W6
    POP.D  W4
    POP.D  W2
    POP.D  W0
    POP    SR
    return


    
_ry_task_switch:
    PUSH	SR						/* Save the SR used by the task.... */
    PUSH	W0						/* ....then disable interrupts. */
    MOV #0, W0
    MOV W0, INTCON2
    PUSH	W1						/* Save registers to the stack. */
    PUSH.D	W2
    PUSH.D	W4
    PUSH.D	W6
    PUSH.D 	W8
    PUSH.D 	W10
    PUSH.D	W12
    PUSH	W14
    PUSH	RCOUNT
    PUSH	TBLPAG
    PUSH	ACCAL
    PUSH	ACCAH
    PUSH	ACCAU
    PUSH	ACCBL
    PUSH	ACCBH
    PUSH	ACCBU
    PUSH	DCOUNT
    PUSH	DOSTARTL
    PUSH	DOSTARTH
    PUSH	DOENDL
    PUSH	DOENDH


    PUSH	CORCON
    PUSH    DSRPAG
    PUSH    DSWPAG
    MOV		_ryCurrentTask, W0			/* Save the new top of stack into the TCB. */
    MOV		W15, [W0]
    
    MOV		_ryNewTask, W0
    MOV		W0, _ryCurrentTask

    MOV		_ryCurrentTask, W0			/* Restore the stack pointer for the task. */
    MOV		[W0], W15
    POP    DSWPAG
    POP    DSRPAG
    POP		CORCON
    POP		DOENDH
    POP		DOENDL
    POP		DOSTARTH
    POP		DOSTARTL
    POP		DCOUNT
    POP		ACCBU
    POP		ACCBH
    POP		ACCBL
    POP		ACCAU
    POP		ACCAH
    POP		ACCAL
    POP		TBLPAG
    POP		RCOUNT						/* Restore the registers from the stack. */
    POP		W14
    POP.D	W12
    POP.D	W10
    POP.D	W8
    POP.D	W6
    POP.D	W4
    POP.D	W2
    POP.D	W0
    POP		SR
    MOV		#32768, W0
    MOV		W0, INTCON2
    
    return
.end


