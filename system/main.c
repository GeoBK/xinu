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

/*  main.c  - main */

#include <xinu.h>

void sync_printf(char *fmt, ...)

{    
	intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
}


process cycliclockswithtrylock(al_lock_t *l1, al_lock_t* l2)
{
	while(1)
	{
		while(!al_trylock(l1));	
		sync_printf("Acquired first lock! \n");
		sleep(1);
		if(al_trylock(l2))
		{
			//Do activity here
			al_unlock(l2);
			al_unlock(l1);
			break;
		}
		else{
			al_unlock(l1);
		}		
	}
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




