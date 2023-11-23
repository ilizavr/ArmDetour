#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define _SYS_PAGE_SIZE_ (sysconf(_SC_PAGE_SIZE))
#define _PAGE_START_OF_(x)    ((uintptr_t)x & ~(uintptr_t)(_SYS_PAGE_SIZE_ - 1))
#define _PAGE_END_OF_(x, len) (_PAGE_START_OF_((uintptr_t)x + len - 1))
#define _PAGE_LEN_OF_(x, len) (_PAGE_END_OF_(x, len) - _PAGE_START_OF_(x) + _SYS_PAGE_SIZE_)
#define _PAGE_OFFSET_OF_(x)   ((uintptr_t)x - _PAGE_START_OF_(x))



uintptr_t ArmDetour(uintptr_t addresssrc,uintptr_t function){
	//simple arm hook function by ilizavr. t.me/pinkdevil222
	
	//mapping region for oldfunc
	uintptr_t oldvalue=(uintptr_t)mmap(NULL,1024,(PROT_READ|PROT_WRITE|PROT_EXEC),(MAP_PRIVATE|MAP_ANONYMOUS),-1,NULL);
	//copy 8 bytes from orig func to old func
	*(long long *)(oldvalue) = *(long long *)(addresssrc);
	//trampoline to orig func
	*(unsigned char *)(oldvalue+8)=4;
	*(unsigned char *)(oldvalue+9)=0xF0;
	*(unsigned char *)(oldvalue+10)=0x1F;
	*(unsigned char *)(oldvalue+11)=0xE5;
	*(uintptr_t *)(oldvalue+0xC)=addresssrc+8;
	
	//change protect of orig func
	mprotect((void *)_PAGE_START_OF_(addresssrc), _PAGE_LEN_OF_(addresssrc, 8), (PROT_READ|PROT_WRITE|PROT_EXEC));
	//trampoline to hooked func
	*(unsigned char *)(addresssrc+0)=4;
	*(unsigned char *)(addresssrc+1)=0xF0;
	*(unsigned char *)(addresssrc+2)=0x1F;
	*(unsigned char *)(addresssrc+3)=0xE5;
	*(uintptr_t *)(addresssrc+4)=function;
	
	//return old func
	return oldvalue;
}
