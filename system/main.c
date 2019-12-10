// /*  main.c  - main */

// #include <xinu.h>

// process	main(void)
// {

// 	/* Run the Xinu shell */

// 	recvclr();
// 	//resume(create(shell, 8192, 50, "shell", 1, CONSOLE));
// 	resume(vcreate(shell, 8192, 50, "shell", 1, CONSOLE));

// 	/* Wait for shell to exit and recreate it */

// 	while (TRUE) {
// 		receive();
// 		sleepms(200);
// 		kprintf("\n\nMain process recreating shell\n\n");
// 		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
// 	}
// 	return OK;
    
// }


/*  main.c  - main */

#include <xinu.h>

/* tests using only FFS area - all students */
// #define TEST1
// #define TEST2
// #define TEST3
// #define TEST4

/* tests using SWAP area - only ECE592 students */
#define TEST5
// #define TEST6
// #define TEST7
// #define TEST8

uint32 error = 0; 
uint32 done = 0;

void sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
}


void test(uint32 numPages){
    char *ptr = NULL;
    ptr = vmalloc(numPages * PAGE_SIZE);

    if (ptr==(char *)SYSERR){
	sync_printf("[P%d] vmalloc failed\n", currpid);
	kill(currpid);
    }

    uint32 i=0;

    // write data
	kprintf("WTF!\n");
    for(i =0; i<numPages; i++){
		sync_printf("[P%d] writing i: %d\n", currpid,i);
        ptr[i*PAGE_SIZE] = 'A';
    }

    // read data
    char c = 0;
    i=0;
    for(i=0; i<numPages; i++){
		sync_printf("[P%d] reading i: %d\n", currpid,i);
        c =  ptr[i*PAGE_SIZE];
        if(c!='A'){
            error = 1;
			sync_printf("[P%d] error occured. c: %c\n", currpid,c);
            break;
        }
    }
    if (i!=numPages) 
	{
		sync_printf("[P%d] error occured. i: %d\n", currpid,i);
		error=1;
	}

    if (vfree(ptr, numPages*PAGE_SIZE)==SYSERR){
	sync_printf("[P%d] vfree failed\n", currpid);
	kill(currpid);
    }

    done = 1;
}

/*
 * Test1: An extreme case to exhaust FFS space
 */
void test1_run(void){

    error = 0; done = 0;
    pid32 p1 = vcreate(test, 2000, 50, "test", 1, MAX_FFS_SIZE);
    resume(p1);

    receive();
    if(error || !done){
        sync_printf("\nTest case 1 FAIL\n");
    }else{
        sync_printf("\nTest case 1 PASS\n");
    }
}


/*
 * Test5: An extreme case to exhaust SWAP space
 */
void test5_run(void){

    error = 0; done = 0;
	
    pid32 p1 = vcreate(test, 2000, 50, "test", 1, MAX_FFS_SIZE+MAX_SWAP_SIZE);
    resume(p1);

    receive();
    if(error || !done){
        sync_printf("\nTest case 5 FAIL\n");
    }else{
        sync_printf("\nTest case 5 PASS\n");
    }
}

/*
 * Test 2: Multiple processes exhaust FFS space 
 */
void test2_run(void){

    error = 0; done = 0;	

    pid32 p1 = vcreate(test, 2000, 10, "P1", 1, MAX_FFS_SIZE/4);
    pid32 p2 = vcreate(test, 2000, 10, "P2", 1, MAX_FFS_SIZE/4);
    pid32 p3 = vcreate(test, 2000, 10, "P3", 1, MAX_FFS_SIZE/4);
    pid32 p4 = vcreate(test, 2000, 10, "P4", 1, MAX_FFS_SIZE/4);
    resume(p1);
    resume(p2);
    resume(p3);
    resume(p4);

    receive();
    receive();
    receive();
    receive();

    if(error || !done){
        sync_printf("\nTest case 2 FAIL\n");
    }else{
        sync_printf("\nTest case 2 PASS\n");
    }
}

/*
 * Test 6: Multiple processes exhaust SWAP space 
 */ 
void test6_run(void){
    
    error = 0; done = 0; 

    pid32 p1 = vcreate(test, 2000, 10, "P1", 1, (MAX_FFS_SIZE+MAX_SWAP_SIZE)/4 );
    pid32 p2 = vcreate(test, 2000, 10, "P2", 1, (MAX_FFS_SIZE+MAX_SWAP_SIZE)/4 );
    pid32 p3 = vcreate(test, 2000, 10, "P3", 1, (MAX_FFS_SIZE+MAX_SWAP_SIZE)/4 );
    pid32 p4 = vcreate(test, 2000, 10, "P4", 1, (MAX_FFS_SIZE+MAX_SWAP_SIZE)/4 );
    resume(p1);
    resume(p2);
    resume(p3);
    resume(p4);

    receive();
    receive();
    receive();
    receive();

    if(error || !done){
        sync_printf("\nTest case 6 FAIL\n");
    }else{
        sync_printf("\nTest case 6 PASS\n");
    }
}

/*
 * Test 3: Tests if vfree() works when FFS space is exhausted
 */
void test3_run(void){
    error = 0; done = 0;
    pid32 p1 = vcreate(test, 2000, 10, "P1", 1,  MAX_FFS_SIZE);
    resume(p1);
    // wait for the first process to be finished
    receive();

    pid32 p2 = vcreate(test, 2000, 10, "P2", 1, MAX_FFS_SIZE);
    resume(p2);
    // wait for the second process to be finished
    receive();

    if(error || !done){
        sync_printf("\nTest case 3 FAIL\n");
    }else{
        sync_printf("\nTest case 3 PASS\n");
    }
}

/*
 * Test 7: Tests if vfree() works when SWAP space is exhausted
 */
void test7_run(void){

    error=0;   
 
    pid32 p1 = vcreate(test, 2000, 10, "P1", 1, MAX_FFS_SIZE+MAX_SWAP_SIZE);
    resume(p1);
    // wait for the first process to be finished
    receive();

    pid32 p2 = vcreate(test, 2000, 10, "P2", 1, MAX_FFS_SIZE+MAX_SWAP_SIZE);
    resume(p2);
    // wait for the second process to be finished
    receive();

    if(error || !done){
        sync_printf("\nTest case 7 FAIL\n");
    }else{
        sync_printf("\nTest case 7 PASS\n");
    }
}

/*
 * Test 4: Simple test case that uses only portion of FFS space
 */
void test4_run(void){

    error = 0; done = 0;

    pid32 p1 = vcreate(test, 2000, 50, "test4", 1, MAX_FFS_SIZE/2);
    resume(p1);

    receive();
    if(error || !done){
        sync_printf("\nTest case 4 FAIL\n");
    }else{
        sync_printf("\nTest case 4 PASS\n");
    }
}

/*
 * Test 8: Simple test case that uses all FFS space and part of the SWAP space
 */
void test8_run(void){

    error = 0; done = 0;

    pid32 p1 = vcreate(test, 2000, 50, "test8", 1, MAX_FFS_SIZE+MAX_SWAP_SIZE/2);
    resume(p1);

    receive();
    if(error || !done){
        sync_printf("\nTest case 8 FAIL\n");
    }else{
        sync_printf("\nTest case 8 PASS\n");
    }
}

process	main(void)
{
#ifdef TEST1
    sync_printf("\n.........run TEST1......\n");
    test1_run();
#endif
#ifdef TEST2
    sync_printf("\n.........run TEST2......\n");
    test2_run();
#endif
#ifdef TEST3
    sync_printf("\n.........run TEST3......\n");
    test3_run();
#endif
#ifdef TEST4
    sync_printf("\n.........run TEST4......\n");
    test4_run();
#endif
#ifdef TEST5
    sync_printf("\n.........run TEST5......\n");
    test5_run();
#endif
#ifdef TEST6
    sync_printf("\n.........run TEST6......\n");
    test6_run();
#endif
#ifdef TEST7
    sync_printf("\n.........run TEST7......\n");
    test7_run();
#endif
#ifdef TEST8
    sync_printf("\n.........run TEST8......\n");
    test8_run();
#endif
    sync_printf("\nAll tests are done!\n");
    return OK;
}


