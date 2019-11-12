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

process deadlockfunc(al_lock_t *l1, al_lock_t* l2)
{
	sync_debug_out("In deadlock function \n");
	al_lock(l1);
	yield();
	al_lock(l2);
	al_unlock(l2);
	al_unlock(l1);
}

process	main(void)
{
	uint32 x;			// shared variable
	unsigned nt;			// number of threads cooperating
	unsigned value = 100; 		// target value of variable
	al_lock_t mutex;  		// mutex	

	kprintf("\n\n=====     Testing the SPINLOCK's implementation     =====\n");

	// 10 threads
	kprintf("\n\n================= TEST 1 = 10 threads ===================\n");
	x = 0;	nt = 10;
 	al_initlock(&mutex); 
	resume(create((void *)nthreads, INITSTK, 1,"nthreads", 4, nt, &x, value/nt, &mutex));
	receive(); 
	sync_printf("%d threads, n=%d, target value=%d\n", nt, value, x);
	if (x==value) kprintf("TEST PASSED.\n"); else kprintf("TEST FAILED.\n");

	al_lock_t l1,l2;
	al_initlock(&l1);
	al_initlock(&l2);
	resume(create((void *)deadlockfunc, INITSTK, 1,"deadlock1", 2, &l1, &l2));
	resume(create((void *)deadlockfunc, INITSTK, 1,"deadlock2", 2, &l2, &l1));
	return OK;
}



