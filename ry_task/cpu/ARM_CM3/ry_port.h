
#ifndef __RY_PORT_H__
	#define	__RY_PORT_H__



#include <stdio.h>




#define  DWT_CR               *(uint32_t *)0xE0001000
#define  DWT_CYCCNT           *(uint32_t *)0xE0001004
#define  DEM_CR               *(uint32_t *)0xE000EDFC

#define  DEM_CR_TRCENA        (1 << 24)
#define  DWT_CR_CYCCNTENA     (1 << 0)







extern void ry_time_point_start(void);
extern void ry_time_point_end(void);







#endif
