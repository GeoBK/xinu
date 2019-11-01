/*  main.c  - main */

#include <xinu.h>
lock_t l;
void lock_test()
{
	kprintf("------------------Inside child process -------------\n");
	lock(&l);
	kprintf("Inside lock of child process !!!\n");
	unlock(&l);
}

process	main(void)
{

	// /* Run the Xinu shell */

	// recvclr();
	// resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	// /* Wait for shell to exit and recreate it */

	// while (TRUE) {
	// 	receive();
	// 	sleepms(200);
	// 	kprintf("\n\nMain process recreating shell\n\n");
	// 	resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	// }

	
	initlock(&l);
	// lock(&l);
	// unlock(&l);

	pid32 pid = create(lock_test,8192,1,"lock_tester_child",0);
	lock(&l);
	kprintf("Parent has lock now \n");
	resume(pid);
	sleep(10);
	printf("Inside parent process after sleep(If the lock had worked this line should be printed first)\n");
	unlock(&l);
	sleep(10);
	
	kprintf("Runtime of child process pid(%d) : %d",pid, proctab[pid].runtime);	

	return OK;
    
}
