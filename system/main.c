/*  main.c  - main */

#include <xinu.h>

process	main(void)
{

	/* Run the Xinu shell */

	recvclr();
	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));	

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}
	return OK;
    
}


// process test(void){

//         kprintf("%s: process #%d\n",proctab[currpid].prname, currpid);

// }


// process main(void)

// {


//         pid32 pid1;

//         pid32 pid2;

//         pid32 pid3;


//         kprintf("main: process #%d\n",currpid);


//         pr_status_syscall_summary();



//         pid1 = create((void *)test, 8192, 50, "test1", 1);

//         resume(pid1);


//         pid2 = create((void *)test, 8192, 50, "test2", 1);

//         resume(pid2);


//         pid3 = create((void *)test, 8192, 50, "test3", 1);

//         resume(pid3);


//         sleep(2);


//         pr_status_syscall_summary();


// }