#ifndef __RY_CONF_H__
	#define	__RY_CONF_H__


/* ��ǰ�汾�� */
#define  RY_VERSION                              V0.15

/* ���� */
//#define  RY_AUTHOR                               1225
#define  RY_AUTHOR   ".               _\r\n            \
_ _|_|\r\n          _|_|_|_\r\n         |_|   |_|\r\n"


/* ���ȼ����� */
#define  RY_PRIORITY_NUM                        (32)

/* Tick���ڣ�x(us)������ */
#define  RY_TICK_PERIOD                         (1000)

#define  RY_DEBUG                               (1)


/* ���������ջ��С */
#define  RY_IDLE_STACK_SIZE                     (128)



/**
 * �����ͨ��
 **/
/* ���� */
#define  RY_MSG                                 (1)
/* �ź����������� */
#define  RY_SEM                                 (1)
/* �¼� */
#define  RY_EVENT                               (1)



//#define  RY_CPU_8BIT                             0
#define  RY_CPU_16BIT                            1
#define  RY_CPU_32BIT                            2
//#define  RY_CPU_64BIT                            3
/* CPUλ�� */
#define  RY_CPU_BIT_WIDE                        (RY_CPU_32BIT)

/* ջ�������� */
#define  RY_STACK_GROW_UP                       (0)


/* �ڴ������ֽ��� */
#define  RY_ALIGN_SIZE                          (4)

/* �ڴ����ʹ���˻������������ж� */
#if RY_SEM == 1
/* ��̬�ڴ����ʹ�� */
#define  RY_MEM                                 (1)
#if RY_MEM == 1
/* ��̬�ڴ�������Ķѿռ� N ���ֽ� */
#define  RY_HEAP_SIZE                           (1024 * 8)
/* ��̬�ڴ�����ڴ����ʽʹ�� */
#define  RY_MEMPOOL                             (1)
#endif
#endif




#define  ry_weak                                 __weak
#define  ry_inline                               static __inline

#define  ALIGN(n)                                __attribute__((aligned(n)))



#if RY_DEBUG == 1
#define  RY_DBG_PRINTF(p, msg)                   p msg
#else
#define  RY_DBG_PRINTF(p, msg)
#endif





#endif
