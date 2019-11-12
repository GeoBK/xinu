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


/*  main.c  - main */

/*  main.c  - main */

#include <xinu.h>

void sync_printf(char *fmt, ...)

{    
	intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
}

process increment(uint32 *x, uint32 n, al_lock_t *mutex){
	uint32 i, j;	
	for (i=0; i<n; i++){
		al_lock(mutex);
		(*x)+=1;
		for (j=0; j<1000; j++);
		yield();
		al_unlock(mutex);
	}
	return OK;
}

process nthreads(uint32 nt, uint32 *x, uint32 n, al_lock_t *mutex){
	pid32 pids[nt];
	int i;	
	for (i=0; i < nt; i++){
		pids[i] = create((void *)increment, INITSTK, 1,"p", 3, x, n, mutex);
		if (pids[i]==SYSERR){
			kprintf("nthreads():: ERROR - could not create process");
			return SYSERR;
		}
	}
	for (i=0; i < nt; i++){
		if (resume(pids[i]) == SYSERR){
			kprintf("nthreads():: ERROR - could not resume process");
			return SYSERR;
		}
	}
	for (i=0; i < nt; i++) receive();
	return OK;
}

process cycliclockswithtrylock(al_lock_t *l1, al_lock_t* l2)
{
	while(!al_trylock(l1));	
	sleep(1);
    while(!al_trylock(l2));	
	al_unlock(l2);
	al_unlock(l1);
	return OK;
}

process deadlockfunc(al_lock_t *l1, al_lock_t* l2)
{
	
	al_lock(l1);

	sleep(1);
	al_lock(l2);
	al_unlock(l2);
	al_unlock(l1);
	return OK;
}

process	main(void)
{
	// uint32 x;			// shared variable
	// unsigned nt;			// number of threads cooperating
	// unsigned value = 100; 		// target value of variable
	// al_lock_t mutex;  		// mutex	

	// kprintf("\n\n=====     Testing the SPINLOCK's implementation     =====\n");

	// // 10 threads
	// kprintf("\n\n================= TEST 1 = 10 threads ===================\n");
	// x = 0;	nt = 10;
 	// al_initlock(&mutex); 
	// resume(create((void *)nthreads, INITSTK, 1,"nthreads", 4, nt, &x, value/nt, &mutex));
	// receive(); 
	// sync_printf("%d threads, n=%d, target value=%d\n", nt, value, x);
	// if (x==value) kprintf("TEST PASSED.\n"); else kprintf("TEST FAILED.\n");


	//Testcase2 using the trylock function
    al_lock_t l5,l6;
	al_initlock(&l5);
	al_initlock(&l6);
    pid32 pid5 = create((void *)cycliclockswithtrylock, INITSTK, 1,"trylock", 2, &l5, &l6);
	pid32 pid6 = create((void *)cycliclockswithtrylock, INITSTK, 1,"trylock", 2, &l6, &l5);
	resume(pid5);
	resume(pid6);
	receive();
	receive();
    kprintf("Deadlock not created!!!\n");

	al_lock_t l1,l2,l3,l4;
	al_initlock(&l1);
	al_initlock(&l2);
	al_initlock(&l3);
	al_initlock(&l4);
	
	
	kprintf("Creating deadlock creating child processes\n");
	pid32 pid1 = create((void *)deadlockfunc, INITSTK, 1,"deadlock1", 2, &l1, &l2);
	pid32 pid2 = create((void *)deadlockfunc, INITSTK, 1,"deadlock2", 2, &l2, &l1);
	pid32 pid3 = create((void *)deadlockfunc, INITSTK, 1,"deadlock3", 2, &l3, &l4);
	pid32 pid4 = create((void *)deadlockfunc, INITSTK, 1,"deadlock4", 2, &l4, &l3);
	kprintf("Created children\n");
	resume(pid1);
	resume(pid2);
	resume(pid3);
	resume(pid4);
	receive();
	receive();
	receive();
	receive();
	kprintf("Deadlock not created!!!\n");
	//resume(pid2);
	//Expected output   -   lock_detected=P1-P2	
	return OK;
}



