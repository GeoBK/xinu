/* sleep.c - sleep sleepms */

#include <xinu.h>

#define	MAXSECONDS	2147483		/* Max seconds per 32-bit msec	*/

/*------------------------------------------------------------------------
 *  sleep  -  Delay the calling process n seconds
 *------------------------------------------------------------------------
 */
syscall	sleep(
	  int32	delay		/* Time to delay in seconds	*/
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

	if ( (delay < 0) || (delay > MAXSECONDS) ) {
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
		procsumm_table[getpid()].rec_count[sleep_enum]++;
		procsumm_table[getpid()].total_cycles[sleep_enum]+=(int)num_cycles;
		//---------------------------------------------------------------------------------------------------------------
		return SYSERR;
	}
	sleepms(1000*delay);

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
	procsumm_table[pid].rec_count[sleep_enum]++;
	procsumm_table[pid].total_cycles[sleep_enum]+=(int)num_cycles;
	//---------------------------------------------------------------------------------------------------------------
	return OK;
}

/*------------------------------------------------------------------------
 *  sleepms  -  Delay the calling process n milliseconds
 *------------------------------------------------------------------------
 */
syscall	sleepms(
	  int32	delay			/* Time to delay in msec.	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/

	if (delay < 0) {
		return SYSERR;
	}

	if (delay == 0) {
		yield();
		return OK;
	}

	/* Delay calling process */

	mask = disable();
	if (insertd(currpid, sleepq, delay) == SYSERR) {
		restore(mask);
		return SYSERR;
	}

	proctab[currpid].prstate = PR_SLEEP;
	resched();
	restore(mask);
	return OK;
}
