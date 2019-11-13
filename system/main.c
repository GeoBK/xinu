// /*  main.c  - main */

// #include <xinu.h>



// process	main(void)
// {

// 	// /* Run the Xinu shell */

// 	// recvclr();
// 	// resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

// 	// /* Wait for shell to exit and recreate it */

// 	// while (TRUE) {
// 	// 	receive();
// 	// 	sleepms(200);
// 	// 	kprintf("\n\nMain process recreating shell\n\n");
// 	// 	resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
// 	// }

	
// 	initlock(&l);
// 	// lock(&l);
// 	// unlock(&l);

	

// 	return OK;
    
// }


// /*  main.c  - main */

/*  main.c  - main */






#include <xinu.h>

void sync_printf(char *fmt, ...)

{    
	intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
}

process multlocks(al_lock_t *l1, al_lock_t* l2)
{
	
	al_lock(l1);	
	al_lock(l2);
	int i,j,k;
	for(i=0;i<2000;i++)
	{
		for(j=0;j<2000;j++)
		{
			for(k=0;k<2000;k++);
		}
		//sync_debug_out("%d\n",i);
	}
	al_unlock(l2);
	al_unlock(l1);
	return OK;
}

process pilocks(pi_lock_t *l1)
{	
	pi_lock(l1);
	int i,j,k;
	for(i=0;i<2000;i++)
	{
		for(j=0;j<2000;j++)
		{
			for(k=0;k<2000;k++);
		}
		//sync_debug_out("%d\n",i);
	}
	sync_printf("PID: %d with priority %d completed.\n",currpid,proctab[currpid].prprio);
	pi_unlock(l1);    	
	return OK;
}

process lockswithdiffpri(lock_t *l1)
{
	
	lock(l1);
	int i,j,k;
	for(i=0;i<2000;i++)
	{
		for(j=0;j<2000;j++)
		{
			for(k=0;k<2000;k++);
		}
		//sync_debug_out("%d\n",i);
	}
	sync_printf("PID: %d with priority %d completed.\n",currpid,proctab[currpid].prprio);	
	unlock(l1);    
	return OK;
}

process singlelock(lock_t *l1)
{
	
	lock(l1);	
	unlock(l1);    
	return OK;
}

process longrunningprocess()
{
	sync_printf("Inside long running process\n");
	int i,j,k;
    for(i=0;i<200;i++)
    {
        for(j=0;j<2000;j++)
		{
			for(k=0;k<2000;k++);
		}        
    }
    sync_printf("PID: %d with priority %d completed.\n",currpid,proctab[currpid].prprio);	
	return OK;
}

process main()
{
    
    pi_lock_t l5;
	pi_initlock(&l5);
    pid32 pid1 = create((void *)pilocks, INITSTK, 1,"trylock", 2, &l5);
	pid32 pid2 = create((void *)pilocks, INITSTK, 3,"trylock", 2, &l5);
    pid32 pid3 = create((void *)longrunningprocess, INITSTK, 2,"trylock", 0);
    resume(pid1);
	sleepms(6);
	resume(pid3);
	resume(pid2);	
	receive();
	receive();
	receive();
    kprintf("Testcase complete\n");   

    //Expected output
    // PID: 6 with priority 1 completed.
    // PID: 7 with priority 3 completed.
    // PID: 8 with priority 2 completed.


	//Behaviour without priority inversion
    lock_t l6;
	initlock(&l6);	
    pid32 pid4 = create((void *)lockswithdiffpri, INITSTK, 1,"trylock", 2, &l6);
	pid32 pid5 = create((void *)lockswithdiffpri, INITSTK, 3,"trylock", 2, &l6);
    pid32 pid6 = create((void *)longrunningprocess, INITSTK, 2,"trylock", 0);
    resume(pid4);
	sleepms(6);
	resume(pid5);
	resume(pid6);	
	receive();
	receive();
	receive();
    kprintf("Testcase complete\n");  
	//Expected output
	// PID: 11 with priority 2 completed. 
    // PID: 9 with priority 1 completed.
    // PID: 10 with priority 3 completed.
    


	//Testing transitivity of priority inheritance
	pi_lock_t l7,l8;
	initlock(&l7);	
	initlock(&l8);
	pid32 pid7 = create((void *)multlocks, INITSTK, 1,"ml", 2, &l7, &l8);
	pid32 pid8 = create((void *)singlelock, INITSTK, 1,"ml", 3, &l7);
	pid32 pid9 = create((void *)singlelock, INITSTK, 1,"ml", 4, &l8);
	resume(pid7);
	sleepms(10);
	resume(pid8);	
	resume(pid9);	
	receive();
	receive();
	receive();
	return OK;
}