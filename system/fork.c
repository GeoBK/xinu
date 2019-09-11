/* fork.c fork */
#include <xinu.h>
#include <string.h>

pid32 fork(){
    struct	procent	*prptr= &proctab[currpid];
    pid32 child_pid = create(0,prptr->prstklen,prptr->prprio,"forked child",0);
    uint32 ebp_fork=10;
    asm volatile("movl %%ebp,%0\n\t": "=r" (ebp_fork));
    kprintf("Value in ebp_fork: %d \n", ebp_fork);
    kprintf("Marker 5 \n");
    /* copying the parents stack here */
    int32 stack_length= prptr->prstkbase - ebp_fork + 1;
    int32 offset = proctab[child_pid].prstkbase - prptr->prstkbase;
    kprintf("Marker 6 \n");
    memcpy(proctab[child_pid].prstkbase-stack_length,ebp_fork,stack_length);
    kprintf("Marker 7 \n");
    int32 *ebp_recursive=ebp_fork;
    while (*ebp_recursive != STACKMAGIC)
    {
        kprintf("Marker 9 \n");
        ebp_recursive+=offset;
        kprintf("Marker 10 \n");
        kprintf("ebp_recursive %08X \n");
        ebp_recursive= *ebp_recursive;
    }
    kprintf("Marker 8 \n");
    kprintf("Marker 1 \n");
    uint32 *pushsp;
    uint32 *savsp;
    savsp = (uint32*) ebp_fork;		/* Start of frame for ctxsw	*/
    uint32 *saddr;
    saddr=(uint32*) ebp_fork;
	*--saddr = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/
    kprintf("Marker 2 \n");

	*--saddr = 0;			/* %eax */
	*--saddr = 0;			/* %ecx */
	*--saddr = 0;			/* %edx */
	*--saddr = 0;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = 0;			/* %esi */
	*--saddr = 0;			/* %edi */
	*pushsp = (unsigned long) (proctab[child_pid].prstkptr = (char *)saddr);
    kprintf("Marker 3 \n");
    resume(child_pid);
    kprintf("Marker 4 \n");
    return child_pid;
}

