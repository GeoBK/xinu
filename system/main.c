/*  main.c  - main */

#include <xinu.h>

// process	main(void)
// {

// 	/* Run the Xinu shell */

// 	recvclr();
// 	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));	

// 	/* Wait for shell to exit and recreate it */

// 	while (TRUE) {
// 		receive();
// 		sleepms(200);
// 		kprintf("\n\nMain process recreating shell\n\n");
// 		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
// 	}
// 	return OK;
    
// }


process test(void){

        kprintf("%s: process #%d\n",proctab[currpid].prname, currpid);

}


process main(void)

{


        pid32 pid1;

        pid32 pid2;

        pid32 pid3;


        kprintf("main: process #%d\n",currpid);


        pr_status_syscall_summary();



        pid1 = create((void *)test, 8192, 50, "test1", 1);

        resume(pid1);


        pid2 = create((void *)test, 8192, 50, "test2", 1);

        resume(pid2);


        pid3 = create((void *)test, 8192, 50, "test3", 1);

        resume(pid3);


        sleep(2);


        pr_status_syscall_summary();


}



// #include <stdarg.h>

// #define TESTCASE1
// #define TESTCASE2
// #define TESTCASE3

// uint32 sum(uint32 a, uint32 b){
// 	return (a+b);
// }

// void sync_printf(char *fmt, ...)
// {
//     	intmask mask = disable();
// 	void *arg = __builtin_apply_args();
// 	__builtin_apply((void*)kprintf, arg, 100);
// 	restore(mask);
// }

// process test1(){
	
// 	sync_printf("HELLO! I am process %d\n", currpid);
	
// 	pid32 pid = fork();
// 	if (pid == SYSERR)	
// 		sync_printf("process %d:: fork failed\n",currpid);
// 	else if (pid != NPROC){
// 		sync_printf("process %d:: forked child %d\n", currpid, pid);
// 		receive();
// 	}

// 	sync_printf("process %d:: pid=%d\n",currpid, pid);

// 	sync_printf("GOODBYE! I am process %d\n", currpid);

// 	return OK;
// }

// process test2(){
// 	uint32 i;
// 	pid32 pid;
	
// 	sync_printf("HELLO! I am process %d\n", currpid);

// 	for (i=0;i<3;i++){
// 		pid = fork();
// 		if (pid == SYSERR)	
// 			sync_printf("process %d:: fork failed\n",currpid);
// 		else if (pid != NPROC){
// 			sync_printf("process %d:: forked child %d\n", currpid, pid);
// 			receive();
// 		}
// 	}

// 	sync_printf("GOODBYE! I am process %d\n", currpid);
	
// 	return OK;
// }

// process test3(int a, int b, int *c, int *d){
// 	uint32 i;
// 	pid32 pid;
// 	int *v;

// 	*c = a+b+currpid;	

// 	for (i=0;i<3;i++){
// 		pid = fork();
// 		if (pid == SYSERR)	
// 			sync_printf("process %d:: fork failed\n",currpid);
// 		else if (pid != NPROC){
// 			sync_printf("process %d:: forked child %d\n", currpid, pid);
// 			receive();
// 		}
// 	}

// 	v = (int *)getmem(4);

// 	*d = *d + currpid;

// 	*v = pid;

// 	sync_printf("process %d:: c=0x%x, *c=%d, d=0x%x, *d=%d, v=0x%x, *v=%d\n", currpid, 
// 				c, *c, d, *d, v, *v);
	
// 	return OK;
// }

// process	main(void)
// {

// pid32 pid1;

// #ifdef TESTCASE1
//         sync_printf("\n[TESTCASE-1]\n");
//         pid1 = create((void *)test1, 8192, 50, "test1", 1);
//         resume(pid1);
//         receive();
//         sync_printf("[END-TESTCASE-1]\n\n");
// #endif


// #ifdef TESTCASE2
// 	sync_printf("\n[TESTCASE-2]\n");
//         pid1 = create((void *)test2, 8192, 50, "test2", 1);
//         resume(pid1);
// 	receive();
// 	sync_printf("[END-TESTCASE-2]\n\n");
// #endif
	
// #ifdef TESTCASE3

// 	int c=0;
// 	int d=0;

// 	sync_printf("\n[TESTCASE-3]\n");
//         pid1 = create((void *)test3, 8192, 50, "test3", 4, 1, 10, &c, &d);
//         resume(pid1);
// 	receive();
// 	sync_printf("[END-TESTCASE-3]\n\n");
// #endif
	
// 	return OK;
    
// }
