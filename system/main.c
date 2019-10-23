/*  main.c  - main */

#include <xinu.h>

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

	sl_lock_t l;
	sl_initlock(&l);
	sl_lock(&l);
	sl_unlock(&l);
	uint32 x=5;
	printf("old value: %d\n",test_and_set((void*)&x,1));
	printf("new value: %d\n",x);

	return OK;
    
}
