#ifndef __RY_CONF_H__
    #define    __RY_CONF_H__


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

#define  RY_UART_BAUDRATE                       (115200)





/**
 * �����ͨ��
 **/
/* ���� */
#define  RY_MSG                                 (1)
/* �ź����������� */
#define  RY_SEM                                 (1)
/* �¼� */
#define  RY_EVENT                               (1)

/* IPC�������� */
#define  RY_IPC_MSG_NUM                         (5)




/* �ڴ������ֽ��� */
#define  RY_ALIGN_SIZE                          (4)

/* �ڴ����ʹ���˻������������ж� */
#if RY_SEM == 1
/* ��̬�ڴ����ʹ�� */
#define  RY_MEM                                 (1)
#if RY_MEM == 1
/* ��̬�ڴ�������Ķѿռ� 4 * N ���ֽ� */
#define  RY_HEAP_SIZE                           (1024 * 2)
/* ��̬�ڴ�����ڴ����ʽʹ�� */
#define  RY_MEMPOOL                             (1)
#endif
#endif





#if RY_DEBUG == 1
#define  RY_DBG_PRINTF(p, msg)         p msg
#else
#define  RY_DBG_PRINTF(p, msg)
#endif







#endif
