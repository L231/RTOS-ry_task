#ifndef __RY_MEM_H__
    #define    __RY_MEM_H__

#include "ry_type.h"




#if RY_MEM == 1






#define  RY_MEM_USED_FLAG       0x80000000

/* �û����ڴ������ͷ */
typedef struct
{
    ry_cpu_t              size;    /* ���λ��ʾ�ڴ��Ƿ�ʹ�� */
    ry_cpu_t              prev;
}ry_mem_head_t;

/* ��̬�ڴ�(����ʽ)����ṹ�� */
typedef struct
{
    ry_cpu_t              heap_begin;
    ry_cpu_t              heap_size;
    ry_cpu_t              heap_end;
    ry_cpu_t              free;
    ry_u32_t              heap[RY_HEAP_SIZE];
    
    ry_semaphore_t        semaphore;
}ry_mem_t;





extern void *ry_malloc(ry_u32_t size);
extern void  ry_free(void *mem);





#endif







#endif
