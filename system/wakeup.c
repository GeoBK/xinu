/* wakeup.c - wakeup */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  wakeup  -  Called by clock interrupt handler to awaken processes
 *------------------------------------------------------------------------
 */
void	wakeup(void)
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


	/* Awaken all processes that have no more time to sleep */

	resched_cntl(DEFER_START);
	while (nonempty(sleepq) && (firstkey(sleepq) <= 0)) {
		ready(dequeue(sleepq));
	}

	resched_cntl(DEFER_STOP);
	//Timing trace for system call summary-------------------------------------------------------------------
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
	procsumm_table[getpid()].rec_count[wakeup_enum]++;
	procsumm_table[getpid()].total_cycles[wakeup_enum]+=(int)num_cycles;
	//---------------------------------------------------------------------------------------------------------------
	return;
}
