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
	
	if ( (delay < 0) || (delay > MAXSECONDS) ) {		
		return SYSERR;
	}
	sleepms(1000*delay);
	
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
	double long num_cycles;
	double start, end;
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	unsigned long flags;	
	asm volatile ("CPUID\n\t"
					 "RDTSC\n\t"
					 "mov %%edx, %0\n\t"
					 "mov %%eax, %1\n\t": "=r" (cycles_high), "=r"
					(cycles_low):: "%rax", "%rbx", "%rcx", "%rdx");
	kprintf("Cycles high : %d , Cycles Low : %d", cycles_high, cycles_low);
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
	if (delay < 0) {
		//Timing trace for system call summary-------------------------------------------------------------------
		asm volatile("RDTSCP\n\t"
					 "mov %%edx, %0\n\t"
					 "mov %%eax, %1\n\t"
					 "CPUID\n\t": "=r" (cycles_high1), "=r"
					(cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");	
		start = (double)(cycles_high)*4294967296 + (double)(cycles_low);
		end = ( ((double)cycles_high1*4294967296) + (double)cycles_low1 );	
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
		procsumm_table[getpid()].total_cycles[sleep_enum]+=num_cycles;
		//---------------------------------------------------------------------------------------------------------------
		restore(mask);
		return SYSERR;
	}

	if (delay == 0) {
		yield();
		return OK;
	}

	/* Delay calling process */

	
	if (insertd(currpid, sleepq, delay) == SYSERR) {
		
		//Timing trace for system call summary-------------------------------------------------------------------
		asm volatile("RDTSCP\n\t"
					 "mov %%edx, %0\n\t"
					 "mov %%eax, %1\n\t"
					 "CPUID\n\t": "=r" (cycles_high1), "=r"
					(cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");	
		start = (double)(cycles_high)*4294967296 + (double)(cycles_low);
		end = ( ((double)cycles_high1*4294967296) + (double)cycles_low1 );	
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
		procsumm_table[getpid()].total_cycles[sleep_enum]+=num_cycles;
		//---------------------------------------------------------------------------------------------------------------
		restore(mask);
		return SYSERR;
	}

	proctab[currpid].prstate = PR_SLEEP;
	resched();
	
	//Timing trace for system call summary-------------------------------------------------------------------
	asm volatile("RDTSCP\n\t"
				 "mov %%edx, %0\n\t"
				 "mov %%eax, %1\n\t"
				 "CPUID\n\t": "=r" (cycles_high1), "=r"
				(cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");	
	start = (double)(cycles_high)*4294967296 + (double)(cycles_low);
	end = ( ((double)cycles_high1*4294967296) + (double)cycles_low1 );	
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
	procsumm_table[getpid()].total_cycles[sleep_enum]+=num_cycles;
	//---------------------------------------------------------------------------------------------------------------
	kprintf("Cycles high1 : %d , Cycles Low 1: %d \n", cycles_high1, cycles_low1);
	restore(mask);
	return OK;
}
