# ry_task  
一款轻量级的RTOS，目前已实现：  
* **优先级抢占式调度**  
* **软件定时器，阻塞延时**  
* **同步/通信机制**  
* **内存管理（静态、动态）**  

设计开发参考了RT-Thread.  
目前新增对微芯的**dsPIC33E**系列的支持。  

*网盘下载地址：https://pan.baidu.com/s/1uoS-Xpm8JSPdkq5KYSMFFA?pwd=g1di*
---

# 系统结构  
## ry_task框图  
![系统框图](ry_task框图.png)  
## 文件结构  
```
ry_task  
   │  ry_core.c       //核心  
   │  ry_core.h  
   │  ry_idle.c       //空闲任务  
   │  ry_ipc.c        //同步/通信机制  
   │  ry_ipc.h  
   │  ry_mem.c        //内存管理  
   │  ry_mem.h  
   │  ry_scheduler.c  
   │  ry_timer.c      //软件定时器  
   │  ry_timer.h  
   │  ry_list.h       //双向链表  
   │  ry_type.h       //内核的数据类型定义  
   │  ry_conf.h       //内核的配置文件  
   │  ry_lib.h        //内核的API接口  
   │  
   └─cpu  
      | ARM_CM3
      |    |  ry_port.c     //任务栈等初始化  
      |    |  ry_port.h
      |    └─ ry_switch.s   //任务切换，适配了Cortex-M3  
      └─PIC24_dsPIC
           |  ry_port.c     //任务栈等初始化  
           |  ry_port.h
           └─ ry_switch.s   //任务切换，适配了dsPIC33E系列  
```
---		 

# 如何使用  
## 这里以STM32F103举例  

### 配置系统时基  
```
//ry_task内部采用嘀嗒定时器，弱定义了系统时基的配置函数
__weak void ry_systick_cfg(void)
{
    RCC_ClocksTypeDef  mcu_clk;
    RCC_GetClocksFreq(&mcu_clk);
    SysTick_Config(mcu_clk.SYSCLK_Frequency / 1000000 * RY_TICK_PERIOD);
}
```
### 配置串口，实现调试打印  
```
/* 初始化一个USART外设，如USART1 */

/* 重定向 printf 函数 */
int fputc(int ch, FILE *f)
{
    while((USARTx->SR & USART_FLAG_TXE) == RESET);
    USARTx->DR = ch;
    return ch;
}
```

### 定义任务，运行ry_task  
```
#include <stdint.h>
#include "ry_lib.h"

static ry_task_t *task_uart_comm;

static void task_uart_comm_func(void *p)
{
    uint32_t cnt = 0;
    while(1)
    {
        cnt++;
        TP_Printf("\r\n(%d)uart_send_data\r\n", cnt);
        ry_task_delay(2000);
    }
}

void main(void)
{
    ry_init();
	/* 创建一个任务 */
    task_uart_comm = ry_task_create("uart_comm",          /* 任务名字 */
                                     task_uart_comm_func, /* 任务主体 */
                                     0,                   /* 任务主体的形参 */
                                     0,                   /* 优先级 */
                                     10,                  /* 时间片 */
                                     256);                /* 栈大小 */
    ry_start();
}
```

		 
