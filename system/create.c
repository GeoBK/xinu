/* create.c - create, newpid */

#include <xinu.h>

local	int newpid();

/*------------------------------------------------------------------------
 *  create  -  Create a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32	create(
	  void		*funcaddr,	/* Address of the function	*/
	  uint32	ssize,		/* Stack size in bytes		*/
	  pri16		priority,	/* Process priority > 0		*/
	  char		*name,		/* Name (for debugging)		*/
	  uint32	nargs,		/* Number of args that follow	*/
	  ...
	)
{
	int			num_cycles;
	int32 start, end;
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	unsigned long flags;
	// asm volatile ("CPUID\n\t"
	// 				 "RDTSC\n\t"
	// 				 "mov %%edx, %0\n\t"
	// 				 "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
	// 				"%rax", "%rbx", "%rcx", "%rdx");
	// asm volatile("RDTSCP\n\t"
	// 				 "mov %%edx, %0\n\t"
	// 				 "mov %%eax, %1\n\t"
	// 				"CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: "%rax",
	// 				"%rbx", "%rcx", "%rdx");
	// asm volatile ("CPUID\n\t"
	// 				 "RDTSC\n\t"
	// 				 "mov %%edx, %0\n\t"
	// 				 "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
	// 				"%rax", "%rbx", "%rcx", "%rdx");
	// asm volatile("RDTSCP\n\t"
	// 				 "mov %%edx, %0\n\t"
	// 				 "mov %%eax, %1\n\t"
	// 				"CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: "%rax",
	// 				"%rbx", "%rcx", "%rdx");
	// preempt_disable();
	// raw_local_irq_save(flags);
	asm volatile ("CPUID\n\t"
					 "RDTSC\n\t"
					 "mov %%edx, %0\n\t"
					 "mov %%eax, %1\n\t": "=r" (cycles_high), "=r"
					(cycles_low):: "%rax", "%rbx", "%rcx", "%rdx");

	printf("cycle high: %d, cycle low: %d",cycles_high,cycles_low);

	asm volatile("RDTSCP\n\t"
				 "mov %%edx, %0\n\t"
				 "mov %%eax, %1\n\t"
				 "CPUID\n\t": "=r" (cycles_high1), "=r"
				(cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");
				// raw_local_irq_restore(flags);
				// preempt_enable();


	uint32		savsp, *pushsp;
	intmask 	mask;    	/* Interrupt mask		*/
	pid32		pid;		/* Stores new process id	*/
	struct	procent	*prptr;		/* Pointer to proc. table entry */
	int32		i;
	uint32		*a;		/* Points to list of args	*/
	uint32		*saddr;		/* Stack address		*/

	

	mask = disable();
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( (priority < 1) || ((pid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) {
		restore(mask);
		return SYSERR;
	}

	prcount++;
	max_pid++;
	prptr = &proctab[pid];

	/* Initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
	prptr->prprio = priority;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
		;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;

	prptr->prstime = ctr1000;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

	/* Initialize stack as if the process was called		*/

	*saddr = STACKMAGIC;
	savsp = (uint32)saddr;

	/* Push arguments */
	a = (uint32 *)(&nargs + 1);	/* Start of args		*/
	a += nargs -1;			/* Last argument		*/
	for ( ; nargs > 0 ; nargs--)	/* Machine dependent; copy args	*/
		*--saddr = *a--;	/* onto created process's stack	*/
	*--saddr = (long)INITRET;	/* Push on return address	*/

	/* The following entries on the stack must match what ctxsw	*/
	/*   expects a saved process state to contain: ret address,	*/
	/*   ebp, interrupt mask, flags, registers, and an old SP	*/

	*--saddr = (long)funcaddr;	/* Make the stack look like it's*/
					/*   half-way through a call to	*/
					/*   ctxsw that "returns" to the*/
					/*   new process		*/
	*--saddr = savsp;		/* This will be register ebp	*/
					/*   for process exit		*/
	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
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
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
	restore(mask);
	kprintf("Initial rec_count: %d, Initial total_cycles: %d, Local Var: %d \n",
				procsumm_table[pid].rec_count[create_enum],procsumm_table[pid].total_cycles[create_enum],num_cycles);
	procsumm_table[pid].rec_count[create_enum]++;
	procsumm_table[pid].total_cycles[create_enum]+=num_cycles;
	kprintf("PID: %d , Process Name: %s , Parent Process: %d \n",pid, name, prptr->prparent);
	return pid;
}

/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------
 */
local	pid32	newpid(void)
{
	uint32	i;			/* Iterate through all processes*/
	static	pid32 nextpid = 1;	/* Position in table to try or	*/
					/*   one beyond end of table	*/

	/* Check all NPROC slots */

	for (i = 0; i < NPROC; i++) {
		nextpid %= NPROC;	/* Wrap around to beginning */
		if (proctab[nextpid].prstate == PR_FREE) {
			return nextpid++;
		} else {
			nextpid++;
		}
	}
	return (pid32) SYSERR;
}
