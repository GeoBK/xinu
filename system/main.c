/*  main.c  - main */

#include <xinu.h>
sl_lock_t l;
void lock_test()
{
	kprintf("------------------Inside child process -------------\n");
	sl_lock(&l);
	kprintf("Inside lock of child process !!!\n");
	sl_unlock(&l);
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

	
	sl_initlock(&l);
	sl_lock(&l);
	sl_unlock(&l);

	pid32 pid = create(lock_test,8192,1,"lock_tester_child",0);
	sl_lock(&l);	
	resume(pid);
	sleep(10);
	printf("Inside parent process after sleep(If the lock had worked this line should be printed first)\n");
	sl_unlock(&l);
	sleep(10);
	uint32 x=5;
	printf("old value: %d\n",test_and_set((void*)&x,1));
	printf("new value: %d\n",x);

	return OK;
    
}
