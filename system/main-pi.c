#include <xinu.h>

process deadlockfunc(pi_lock_t *l1)
{
	
	pi_lock(l1);
	for(int i=0;i<200000;i++);
	pi_lock(l2);
    sync_printf("PID: %d with priority %d completed.\n",currpid,proctab[currpid].prprio);	
	return OK;
}

process longrunningprocess()
{
    for(int i=0;i<200000;i++)
    {
        for(int i=0;i<200000;i++)
        {
            for(int i=0;i<200000;i++);
        }
    }
    sync_printf("PID: %d with priority %d completed.\n",currpid,proctab[currpid].prprio);	
}

process main()
{
    //Testcase2 using the trylock function
    pi_lock_t l5,l6;
	pi_initlock(&l5);
	pi_initlock(&l6);
    pid32 pid1 = create((void *)cycliclockswithtrylock, INITSTK, 1,"trylock", 2, &l5);
	pid32 pid3 = create((void *)cycliclockswithtrylock, INITSTK, 3,"trylock", 2, &l5);
    pid32 pid2 = create((void *)longrunningprocess, INITSTK, 2,"trylock", 0);
    resume(pid1);
	resume(pid3);
	resume(pid2);	
	receive();
	receive();
	receive();
    kprintf("Testcase complete\n");   

    //Expected output
    // PID: 7 with priority 3 completed.
    // PID: 8 with priority 2 completed.
    // PID: 6 with priority 1 completed.


	return OK;
}