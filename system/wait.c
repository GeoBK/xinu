/* wait.c - wait */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  wait  -  Cause current process to wait on a semaphore
 *------------------------------------------------------------------------
 */
syscall	wait(
	  sid32		sem		/* Semaphore on which to wait  */
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


	intmask mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	struct	sentry *semptr;		/* Ptr to sempahore table entry	*/

	mask = disable();
	if (isbadsem(sem)) {
		restore(mask);
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
		procsumm_table[getpid()].rec_count[wait_enum]++;
		procsumm_table[getpid()].total_cycles[wait_enum]+=num_cycles;
		//---------------------------------------------------------------------------------------------------------------
		return SYSERR;
	}

	semptr = &semtab[sem];
	if (semptr->sstate == S_FREE) {
		restore(mask);
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
		procsumm_table[getpid()].rec_count[wait_enum]++;
		procsumm_table[getpid()].total_cycles[wait_enum]+=num_cycles;
		//---------------------------------------------------------------------------------------------------------------
		return SYSERR;
	}

	if (--(semptr->scount) < 0) {		/* If caller must block	*/
		prptr = &proctab[currpid];
		prptr->prstate = PR_WAIT;	/* Set state to waiting	*/
		prptr->prsem = sem;		/* Record semaphore ID	*/
		enqueue(currpid,semptr->squeue);/* Enqueue on semaphore	*/
		resched();			/*   and reschedule	*/
	}

	restore(mask);
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
	procsumm_table[getpid()].rec_count[wait_enum]++;
	procsumm_table[getpid()].total_cycles[wait_enum]+=num_cycles;
	//---------------------------------------------------------------------------------------------------------------
	return OK;
}
