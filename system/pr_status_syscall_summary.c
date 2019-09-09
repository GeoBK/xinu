/* pr_status_summary.c - pr_status_summary*/

#include <xinu.h>
#include <stdio.h>

/*-------------------------------------------------------
 * summary of system calls since start of execution
 *------------------------------------------------------
 */

void pr_status_syscall_summary(){
    int32 i,j,k;
    //TODO: make this wait on a semaphore for console out temporary hack 
    for(k=0;k<1000000;k++);

    printf("%3s %7s %5s %14s\n",
		   "pid", "syscall", "count", "average cycles");

	printf("%3s %7s %5s %14s\n",
		   "---", "-------", "-----", "--------------");
    
    int process_id_stats[5];
    
    for(i=0;i<2;i++){
        
        for(j=0; j<2;j++){
            
            printf("%3s %7s %5s %-14s\n","1","create","122","34344");
            printf("%32s\n","--------------------------------");
        }
    }
}