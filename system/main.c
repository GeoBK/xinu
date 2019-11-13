// // /*  main.c  - main */

// // #include <xinu.h>



// // process	main(void)
// // {

// // 	// /* Run the Xinu shell */

// // 	// recvclr();
// // 	// resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

// // 	// /* Wait for shell to exit and recreate it */

// // 	// while (TRUE) {
// // 	// 	receive();
// // 	// 	sleepms(200);
// // 	// 	kprintf("\n\nMain process recreating shell\n\n");
// // 	// 	resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
// // 	// }

	
// // 	initlock(&l);
// // 	// lock(&l);
// // 	// unlock(&l);

	

// // 	return OK;
    
// // }


// // /*  main.c  - main */

// /*  main.c  - main */






// #include <xinu.h>

// void sync_printf(char *fmt, ...)

// {    
// 	intmask mask = disable();
// 	void *arg = __builtin_apply_args();
// 	__builtin_apply((void*)kprintf, arg, 100);
// 	restore(mask);
// }

// process multlocks(pi_lock_t *l1, pi_lock_t* l2)
// {
	
// 	pi_lock(l1);	
// 	pi_lock(l2);
// 	int i,j,k;
// 	for(i=0;i<2000;i++)
// 	{
// 		for(j=0;j<2000;j++)
// 		{
// 			for(k=0;k<2000;k++);
// 		}
// 		//sync_debug_out("%d\n",i);
// 	}
// 	pi_unlock(l2);
// 	pi_unlock(l1);
// 	return OK;
// }

// process pilocks(pi_lock_t *l1)
// {	
// 	pi_lock(l1);
// 	int i,j,k;
// 	for(i=0;i<2000;i++)
// 	{
// 		for(j=0;j<2000;j++)
// 		{
// 			for(k=0;k<2000;k++);
// 		}
// 		//sync_debug_out("%d\n",i);
// 	}
// 	sync_printf("PID: %d with priority %d completed.\n",currpid,proctab[currpid].prprio);
// 	pi_unlock(l1);    	
// 	return OK;
// }

// process lockswithdiffpri(lock_t *l1)
// {
	
// 	lock(l1);
// 	int i,j,k;
// 	for(i=0;i<2000;i++)
// 	{
// 		for(j=0;j<2000;j++)
// 		{
// 			for(k=0;k<2000;k++);
// 		}
// 		//sync_debug_out("%d\n",i);
// 	}
// 	sync_printf("PID: %d with priority %d completed.\n",currpid,proctab[currpid].prprio);	
// 	unlock(l1);    
// 	return OK;
// }

// process singlelock(pi_lock_t *l1)
// {
	
// 	pi_lock(l1);	
// 	pi_unlock(l1);    
// 	return OK;
// }

// process longrunningprocess()
// {
// 	sync_printf("Inside long running process\n");
// 	int i,j,k;
//     for(i=0;i<200;i++)
//     {
//         for(j=0;j<2000;j++)
// 		{
// 			for(k=0;k<2000;k++);
// 		}        
//     }
//     sync_printf("PID: %d with priority %d completed.\n",currpid,proctab[currpid].prprio);	
// 	return OK;
// }

// process main()
// {
    
//     pi_lock_t l5;
// 	pi_initlock(&l5);
//     pid32 pid1 = create((void *)pilocks, INITSTK, 1,"trylock", 2, &l5);
// 	pid32 pid2 = create((void *)pilocks, INITSTK, 3,"trylock", 2, &l5);
//     pid32 pid3 = create((void *)longrunningprocess, INITSTK, 2,"trylock", 0);
//     resume(pid1);
// 	sleepms(6);
// 	resume(pid3);
// 	resume(pid2);	
// 	receive();
// 	receive();
// 	receive();
//     kprintf("Testcase complete\n");   

//     //Expected output
//     // PID: 6 with priority 1 completed.
//     // PID: 7 with priority 3 completed.
//     // PID: 8 with priority 2 completed.


// 	//Behaviour without priority inversion
//     lock_t l6;
// 	initlock(&l6);	
//     pid32 pid4 = create((void *)lockswithdiffpri, INITSTK, 1,"trylock", 2, &l6);
// 	pid32 pid5 = create((void *)lockswithdiffpri, INITSTK, 3,"trylock", 2, &l6);
//     pid32 pid6 = create((void *)longrunningprocess, INITSTK, 2,"trylock", 0);
//     resume(pid4);
// 	sleepms(6);
// 	resume(pid5);
// 	resume(pid6);	
// 	receive();
// 	receive();
// 	receive();
//     kprintf("Testcase complete\n");  
// 	//Expected output
// 	// PID: 11 with priority 2 completed. 
//     // PID: 9 with priority 1 completed.
//     // PID: 10 with priority 3 completed.
    


// 	//Testing transitivity of priority inheritance
// 	pi_lock_t l7,l8;
// 	pi_initlock(&l7);	
// 	pi_initlock(&l8);
// 	pid32 pid7 = create((void *)multlocks, INITSTK, 2,"ml", 2, &l7, &l8);
// 	pid32 pid8 = create((void *)singlelock, INITSTK, 3,"ml", 1, &l7);
// 	pid32 pid9 = create((void *)singlelock, INITSTK, 4,"ml", 1, &l8);
// 	resume(pid7);
// 	sleepms(10);
// 	resume(pid8);	
// 	resume(pid9);	
// 	receive();
// 	receive();
// 	receive();
// 	kprintf("Transitivity testcase complete!\n");
// 	return OK;
// }

/*  main.c  - main */

#include <xinu.h>

inline uint32 get_timestamp(){
	return ctr1000;
}

void run_for_ms(uint32 time){
	uint32 start = proctab[currpid].runtime;
	while (proctab[currpid].runtime-start < time);
}

process p_spinlock(sl_lock_t *l){
	uint32 i;
	for (i=0; i<5; i++){
		sl_lock(l);
		run_for_ms(1000);
		sl_unlock(l);		
	}
	return OK;
}
	
process p_lock(lock_t *l){
	uint32 i;
	for (i=0; i<5; i++){
		lock(l);
		run_for_ms(1000);
		unlock(l);		
	}
	return OK;
}
	
process	main(void)
{

	sl_lock_t	mutex_sl;  		
	lock_t 		mutex;  		
	pid32		pid1, pid2;
	uint32 		timestamp;

	kprintf("\n\n=========== TEST 1: spinlock & 2 threads  ===================\n\n");
 	sl_initlock(&mutex_sl); 
	
	pid1 = create((void *)p_spinlock, INITSTK, 1,"nthreads", 1, &mutex_sl);
	pid2 = create((void *)p_spinlock, INITSTK, 1,"nthreads", 1, &mutex_sl);

	timestamp = get_timestamp();
	
	resume(pid1);
	sleepms(500);
	resume(pid2);		

	receive();
	receive();

    kprintf("Time = %d ms\n", get_timestamp()-timestamp);
        
	kprintf("\n\n=========== TEST 2: lock w/sleep & 2 threads  ===============\n\n");
	initlock(&mutex);

    pid1 = create((void *)p_lock, INITSTK, 1,"nthreads", 1, &mutex);
    pid2 = create((void *)p_lock, INITSTK, 1,"nthreads", 1, &mutex);

    timestamp = get_timestamp();

    resume(pid1);
    sleepms(500);
    resume(pid2);

    receive();
    receive();

    kprintf("Time = %d ms\n", get_timestamp()-timestamp);

	return OK;
}
