
#include "ry_mem.h"
#include "ry_core.h"
#include "ry_timer.h"
#include "ry_list.h"
#include "ry_lib.h"



#define  MEM_MIN              (8)
#define  MEM_MIN_ALIGN        RY_ALIGN(MEM_MIN, RY_ALIGN_SIZE)
#define  MEM_STRUCT_ALIGN     RY_ALIGN(sizeof(ry_mem_head_t), RY_ALIGN_SIZE)

#define  pMemHead_t(mem)      ((ry_mem_head_t *)(mem))


#define  MEMPOOL_HEAD_SIZE    (sizeof(ry_u8_t *))



#if RY_MEM == 1


static ry_mem_t ryMem;




/**
 * 描述：系统堆空间的初始化
 *
 **/
void ry_sysheap_init(void)
{
	ry_mem_head_t *Mem;
	ryMem.heap_begin = RY_ALIGN(ryMem.heap, RY_ALIGN_SIZE);
	ryMem.heap_end   = RY_ALIGN_DOWN(ryMem.heap + RY_HEAP_SIZE, RY_ALIGN_SIZE);
	ryMem.heap_size  = ryMem.heap_end - ryMem.heap_begin - MEM_STRUCT_ALIGN;
	ryMem.free       = ryMem.heap_begin;
	Mem              = pMemHead_t(ryMem.heap_begin);
	Mem->prev        = ryMem.heap_begin;
	Mem->size        = ryMem.heap_size;
	/* 注册一个二值信号量 */
	ry_semaphore_reg(&ryMem.semaphore, RY_SEM_FIFO_NORMAL, 1);
}

/**
 * 描述：系统堆内存申请
 *
 **/
void *ry_malloc(ry_cpu_t size)
{
	ry_cpu_t Free;
	ry_mem_head_t *Mem;
	do
	{
		if(size == 0 || size & RY_MEM_USED_FLAG)
			break;
		
		size = RY_ALIGN(size, RY_ALIGN_SIZE);
		if(size > ryMem.heap_size)
			break;
		
		if(size < MEM_MIN_ALIGN)
			size = MEM_MIN_ALIGN;
		
		/* 使用二值信号量，确保独占 */
		ry_sem_rec(&ryMem.semaphore, -1);
		/* 查找可用的内存空间 */
		for(Free = ryMem.free; Free != ryMem.heap_end; Free += (Mem->size & ~RY_MEM_USED_FLAG) + MEM_STRUCT_ALIGN)
		{
			Mem = pMemHead_t(Free);
			/* 该内存已使用 */
			if(Mem->size & RY_MEM_USED_FLAG)
				continue;
			if(Mem->size >= size)
			{
				if(Mem->size >= (size + MEM_STRUCT_ALIGN + MEM_MIN_ALIGN))
				{
					Free += size + MEM_STRUCT_ALIGN;
					/* 新的free内存数据头的配置 */
					pMemHead_t(Free)->prev = (ry_cpu_t)Mem;
					pMemHead_t(Free)->size = (Mem->size - size - MEM_STRUCT_ALIGN) & ~RY_MEM_USED_FLAG;
					/* 下一块内存的prev指针指向新的free内存 */
					if(((ry_cpu_t)Mem + Mem->size + MEM_STRUCT_ALIGN) < ryMem.heap_end)
						pMemHead_t((ry_cpu_t)Mem + (ry_cpu_t)Mem->size + MEM_STRUCT_ALIGN)->prev = Free;
					/* 当前分割的内存配置大小、使用标志 */
					Mem->size = size | RY_MEM_USED_FLAG;
				}
				/* 内存空间刚好满足 */
				else
				{
					Mem->size |= RY_MEM_USED_FLAG;
				}
				/* 更新free内存 */
				while(pMemHead_t(ryMem.free)->size & RY_MEM_USED_FLAG && ryMem.free != ryMem.heap_end)
				{
					ryMem.free += (pMemHead_t(ryMem.free)->size & ~RY_MEM_USED_FLAG) + MEM_STRUCT_ALIGN;
				}
				ry_sem_release(&ryMem.semaphore);
				return (void *)((ry_cpu_t)Mem + MEM_STRUCT_ALIGN);
			}
		}
		ry_sem_release(&ryMem.semaphore);
	}while(0);
	RY_DBG_PRINTF(ry_printf, ("ry_malloc error!\r\n"));
	return RY_NULL;
}


/**
 * 描述：系统堆内存释放
 *
 **/
void ry_free(void *mem)
{
	ry_cpu_t Free, pFree, nFree;
	if(mem == RY_NULL)
		return;
	if((ry_cpu_t)mem > (ryMem.heap_end - MEM_MIN_ALIGN) || (ry_cpu_t)mem - MEM_STRUCT_ALIGN < ryMem.heap_begin)
		return;
	
	Free = ((ry_cpu_t)mem - MEM_STRUCT_ALIGN);
	/* 使用二值信号量，确保独占 */
	ry_sem_rec(&ryMem.semaphore, -1);
	pMemHead_t(Free)->size &= ~RY_MEM_USED_FLAG;
	if(ryMem.free > Free)
		ryMem.free = Free;
	
	/* 提取下一块内存 */
	nFree = Free + pMemHead_t(Free)->size + MEM_STRUCT_ALIGN;
	/* 确保当前内存不是最后一块，同时下一块内存未使用 */
	if(nFree != ryMem.heap_end && !(pMemHead_t(nFree)->size & RY_MEM_USED_FLAG))
	{
		pMemHead_t(Free)->size += pMemHead_t(nFree)->size + MEM_STRUCT_ALIGN;
		pMemHead_t(nFree + (ry_cpu_t)pMemHead_t(nFree)->size + MEM_STRUCT_ALIGN)->prev = Free;
		pMemHead_t(nFree)->size = 0;
	}
	
	/* 提取上一块内存 */
	pFree = pMemHead_t(Free)->prev;
	/* 确保当前内存不是第一块，同时上一块内存未使用 */
	if(Free != ryMem.heap_begin && !(pMemHead_t(pFree)->size & RY_MEM_USED_FLAG))
	{
		pMemHead_t(pFree)->size += pMemHead_t(Free)->size + MEM_STRUCT_ALIGN;
		/* 确保当前内存不是最后一块 */
		if(nFree != ryMem.heap_end)
			pMemHead_t(Free + (ry_cpu_t)pMemHead_t(Free)->size + MEM_STRUCT_ALIGN)->prev = pFree;
		pMemHead_t(Free)->size = 0;
		if(ryMem.free == Free)
			ryMem.free = pFree;
	}
	
	ry_sem_release(&ryMem.semaphore);
}

#endif



#if RY_MEMPOOL == 1

/**
 * 描述：内存池的注册
 *
 **/
void ry_mempool_reg(ry_mempool_t *mem, void *start, ry_cpu_t size, ry_cpu_t block_size)
{
	ry_u8_t *ptr, *ptrEnd;
	mem->begin          = (ry_u8_t *)RY_ALIGN(start, RY_ALIGN_SIZE);
	mem->size           = RY_ALIGN_DOWN(size, RY_ALIGN_SIZE);
	block_size          = RY_ALIGN(block_size, RY_ALIGN_SIZE);
	mem->block_size     = block_size;
	mem->block_sum      = mem->size / (MEMPOOL_HEAD_SIZE + block_size);
	mem->block_free_cnt = mem->block_sum;
	ptrEnd              = mem->begin + (MEMPOOL_HEAD_SIZE + block_size) * mem->block_sum;
	for(ptr = mem->begin; ptr < ptrEnd; ptr += (block_size + MEMPOOL_HEAD_SIZE))
	{
		/* 指针p指向一个变量地址，p表示那个变量的地址，*p可以访问该地址存储的值 */
		*(ry_u8_t **)ptr  = ptr + block_size + MEMPOOL_HEAD_SIZE;
		/* 现在有个二级指针指向这个地址，前面加*取二级指针指向该地址的值，此时表示该地址定义了一个指针 */
	}
	mem->free_list      = mem->begin;
	
	mem->suspend_cnt    = 0;
	ry_list_init(&mem->suspend_list);
}

/**
 * 描述：创建内存池
 *
 **/
ry_mempool_t *ry_mempool_create(ry_cpu_t block_num, ry_cpu_t block_size)
{
	ry_cpu_t Size;
	ry_mempool_t *Mem;
	
	Mem        = ry_malloc(sizeof(ry_mempool_t));
	if(Mem == RY_NULL)
		return RY_NULL;
	block_size = RY_ALIGN(block_size, RY_ALIGN_SIZE);
	Size       = (block_size + MEMPOOL_HEAD_SIZE) * block_num;
	Mem->begin = ry_malloc(Size);
	ry_mempool_reg(Mem, Mem->begin, Size, block_size);
	return Mem;
}

/**
 * 描述：删除内存池
 *
 **/
void ry_mempool_delete(ry_mempool_t *mem)
{
	ry_task_t *Task;
	ry_list_t *NodePos = mem->suspend_list.next;
	RY_NEW_IT_VARI;
	
	for(; NodePos != &mem->suspend_list; NodePos = NodePos->next)
	{
		RY_INTERRUPT_OFF;
		Task      = TASK_CONTAINER_OF(NodePos, ry_task_t, list);
		ry_task_recover(&Task->list);
		Task->err = RY_ERR_MEM;
		RY_INTERRUPT_ON;
	}
	
	ry_free(&mem->free_list);
	ry_free(mem->begin);
}

/**
 * 描述：内存池申请内存
 *
 **/
void *ry_mempool_malloc(ry_mempool_t *mem, ry_int16_t time)
{
	ry_u8_t *ptr;
	RY_NEW_IT_VARI;
	
	RY_INTERRUPT_OFF;
	if(mem->block_free_cnt == 0)
	{
		ry_task_t *Task = ry_get_task();
		
		/* 任务不等待 */
		if(time == 0)
		{
			RY_INTERRUPT_ON;
			return RY_NULL;
		}
		Task->err   = RY_OK;
		time        = (time > 0) ? time : 0;
		Task->delay = time + ry_get_tick();
		
		/* 挂起任务 */
		ry_task_discard();
		ry_list_insert_before(&mem->suspend_list, &Task->list);
		mem->suspend_cnt++;
		
		/* 延时处理 */
		if(time > 0)
		{
			ry_list_t  *NodePos;
			/* 找到要插入挂起链表的节点位置 */
			NodePos      = ry_get_timer_pos(&Task->delay_list);
			ry_list_remove(&Task->delay_list);
			ry_list_insert_before(NodePos, &Task->delay_list);
			Task->status = RY_SUSPEND;
		}
		RY_INTERRUPT_ON;
		ry_scheduler();
		
		/* 等待超时、任务恢复后资源仍为空 */
		if(Task->err != RY_OK || mem->block_free_cnt == 0)
			return RY_NULL;
	}
	mem->block_free_cnt--;
	ptr              = mem->free_list;
	mem->free_list   = *(ry_u8_t **)ptr;
	/* 被申请内存的指针指向内存控制块 */
	*(ry_u8_t **)ptr = (ry_u8_t *)mem;
	RY_INTERRUPT_ON;
	return (void *)(ptr + MEMPOOL_HEAD_SIZE);
}

/**
 * 描述：释放内存至内存池
 *
 **/
void ry_mempool_free(void *block)
{
	ry_u8_t **ptr;
	ry_mempool_t *Mem;
	RY_NEW_IT_VARI;
	
	if(block == RY_NULL)
		return;
	/* 二级指针指向该地址 */
	ptr = (ry_u8_t **)((ry_u8_t *)block - MEMPOOL_HEAD_SIZE);
	/* 取该地址的值，强制转换为"ry_mempool_t *" */
	Mem = (ry_mempool_t *)*ptr;
	/* 该地址不合法，并未指向内存池控制块 */
	if(Mem->size != (Mem->block_sum * (Mem->block_size + MEMPOOL_HEAD_SIZE)))
		return;
	RY_INTERRUPT_OFF;
	
	Mem->block_free_cnt++;
	/* 取该地址的值，此时在该地址上建立了一个指针，让它指向内存池的空闲链表 */
	*ptr = Mem->free_list;
	/* 该地址放入空闲链表 */
	Mem->free_list = (ry_u8_t *)ptr;
	if(Mem->suspend_cnt > 0)
	{
		/* 挂起链表的第一个节点，就是要恢复的任务的节点 */
		ry_task_t *Task = TASK_CONTAINER_OF(Mem->suspend_list.next, ry_task_t, list);
		Mem->suspend_cnt--;
		/* 恢复任务时，任务节点会自动从挂起链表上脱离 */
		ry_task_recover(&Task->list);
		Task->err = RY_OK;
		RY_INTERRUPT_ON;
		ry_scheduler();
		return;
	}
	RY_INTERRUPT_ON;
}

#endif
