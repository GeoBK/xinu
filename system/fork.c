/* fork.c fork */
#include <xinu.h>
#include <string.h>

pid32 fork(){
    struct	procent	*prptr= &proctab[currpid];
    pid32 child_pid = create(0,prptr->prstklen,prptr->prprio,"forked child",0);
    uint32 ebp_fork=10;
    asm("movl %ebp,ebp_fork");
    kprintf("Value in ebp_fork: %d", ebp_fork);
    /* copying the parents stack here */
    int32 stack_length= prptr->prstkbase - ebp_fork + 1;
    int32 offset = proctab[child_pid].prstkbase - prptr->prstkbase;
    memcpy(proctab[child_pid].prstkbase-stack_length,ebp_fork,stack_length);
    
    int32 *ebp_recursive=ebp_fork;
    while (*ebp_recursive != STACKMAGIC)
    {
        ebp_recursive+=offset;
        ebp_recursive= *ebp_recursive;
    }

    uint32 *pushsp;
    uint32 savsp = 
    savsp = (uint32) ebp_fork;		/* Start of frame for ctxsw	*/
    uint32 *saddr;
    saddr=ebp_fork;
	*--saddr = 0x00000200;		/* New process runs with	*/
					/*   interrupts enabled		*/

	/* Basically, the following emulates an x86 "pushal" instruction*/

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
    resume(child_pid);
    return child_pid;
}

