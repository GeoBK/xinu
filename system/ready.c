/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	unsigned long num_cycles;
	unsigned long start, end;
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	unsigned long flags;	
	asm volatile ("CPUID\n\t"
					 "RDTSC\n\t"
					 "mov %%edx, %0\n\t"
					 "mov %%eax, %1\n\t": "=r" (cycles_high), "=r"
					(cycles_low):: "%rax", "%rbx", "%rcx", "%rdx");
	register struct procent *prptr;

	if (isbadpid(pid)) {
		asm volatile("RDTSCP\n\t"
				 "mov %%edx, %0\n\t"
				 "mov %%eax, %1\n\t"
				 "CPUID\n\t": "=r" (cycles_high1), "=r"
				(cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");	
		start = ( ((long)cycles_high << 32) | cycles_low );
		end = ( ((long)cycles_high1 << 32) | cycles_low1 );	
		int len= sizeof(long);
		if ( (end - start) < 0) {
 			printf("\n\n>>>>>>>>>>>>>> CRITICAL ERROR IN TAKING TIME!!!!!!\n start = %llu, end = %llu, \n",  start, end);
 			num_cycles = 0;
 		}
 		else
 		{
 			num_cycles = end - start;
 		}
		procsumm_table[pid].rec_count[ready_enum]++;
		procsumm_table[pid].total_cycles[ready_enum]+=(int)num_cycles;
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	insert(pid, readylist, prptr->prprio);
	resched();

	asm volatile("RDTSCP\n\t"
				 "mov %%edx, %0\n\t"
				 "mov %%eax, %1\n\t"
				 "CPUID\n\t": "=r" (cycles_high1), "=r"
				(cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");	
	start = ( ((long)cycles_high << 32) | cycles_low );
	end = ( ((long)cycles_high1 << 32) | cycles_low1 );	
	int len= sizeof(long);
	if ( (end - start) < 0) {
 		printf("\n\n>>>>>>>>>>>>>> CRITICAL ERROR IN TAKING TIME!!!!!!\n start = %llu, end = %llu, \n",  start, end);
 		num_cycles = 0;
 	}
 	else
 	{
 		num_cycles = end - start;
 	}
	procsumm_table[pid].rec_count[ready_enum]++;
	procsumm_table[pid].total_cycles[ready_enum]+=(int)num_cycles;

	return OK;
}
