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
    uint32 stack_length= prptr->prstkbase - ebp_fork + 1;
    uint32 offset =10;
    int32 offset_signed = -10;
    uint32 number=50;
    kprintf("offset: %d, number: %d, result: %d",offset, number, number-offset);
    kprintf("offset: %d, number: %d, result: %d",offset_signed, number, number+offset_signed);
    int offset_positive=0;
    if(proctab[child_pid].prstkbase>prptr->prstkbase){
        offset = proctab[child_pid].prstkbase - prptr->prstkbase;
        offset =1;
    }else{
        offset = prptr->prstkbase - proctab[child_pid].prstkbase;
    }

    kprintf("Offset: %0X \n",offset);
    kprintf("Marker 6 \n");
    kprintf("Child Stack Base: %0X \n",proctab[child_pid].prstkbase);
    kprintf("Parent Stack Base: %0X \n", prptr->prstkbase);
    kprintf("Child EBP: %0X \n",proctab[child_pid].prstkbase-stack_length+1);
    kprintf("Parent EBP: %0X \n", ebp_fork);
    memcpy(proctab[child_pid].prstkbase-stack_length+1,ebp_fork,stack_length);
    
    stacktrace(currpid);
    
    kprintf("Marker 7 \n");
    kprintf("Parent Base pointer: %0X \n", prptr->prstkbase);
    kprintf("Child base pointer: %0X \n",proctab[child_pid].prstkbase);
    char *ebp_recursive=proctab[child_pid].prstkbase-stack_length+1;
    // while (*(uint32*)ebp_recursive != STACKMAGIC)
    // {
    //     kprintf("Marker 9 \n");
    //     kprintf("ebp_recursive %0X \n",ebp_recursive);

    //     kprintf("offset: %0X \n",offset);
    //     kprintf("ebp_recursive actual value: %0X\n",*(uint32*)ebp_recursive);
    //     if(offset_positive==1){
    //         *(uint32*)ebp_recursive= *(uint32*)ebp_recursive + offset;
    //     }else{
    //         *(uint32*)ebp_recursive= *(uint32*)ebp_recursive - offset;
    //     }
    //     kprintf("ebp_recursive actual value after adding offset: %0X\n",*(uint32*)ebp_recursive);
    //     kprintf("Marker 10 \n");
    //     kprintf("ebp_recursive %0X \n",ebp_recursive);
    //     ebp_recursive= *(uint32*)ebp_recursive;
    // }
    kprintf("Marker 8 \n");
    kprintf("Marker 1 \n");
    uint32 *pushsp;
    uint32 *savsp;
    savsp = (uint32*) proctab[child_pid].prstkbase-stack_length+1;		/* Start of frame for ctxsw	*/
    uint32 *saddr;
    saddr=(uint32*) proctab[child_pid].prstkbase-stack_length+1;
    kprintf("prstkbase: %0X \n", proctab[child_pid].prstkbase);
    kprintf("proctab[child_pid].prstkbase-stack_length+1: %0X \n", proctab[child_pid].prstkbase-stack_length+1);
    kprintf("saddr: %0X \n", saddr);
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
    //resume(child_pid);
    kprintf("Marker 4 \n");
    stacktrace(child_pid);
    return child_pid;
}

