/* pr_status_summary.c - pr_status_summary*/

#include <xinu.h>
#include <stdio.h>

/*-------------------------------------------------------
 * summary of system calls since start of execution
 *------------------------------------------------------
 */
static char sys_call_description[number_of_system_calls][7] = 
{
    "create",
    "kill",
    "ready",
    "sleep",
    "suspend",
    "wait",
    "wakeup",
    "yield"
};
void pr_status_syscall_summary(){
    int32 i,j,k;
    //TODO: make this wait on a semaphore for console out temporary hack 
    for(k=0;k<1000000;k++);

    printf("%3s %7s %5s %14s\n",
		   "pid", "syscall", "count", "average cycles");

	printf("%3s %7s %5s %14s\n",
		   "---", "-------", "-----", "--------------");    
    
    for(i=0;i<=max_pid;i++){        
        for(j=create_enum; j< number_of_system_calls; j++){
            if(procsumm_table[i].rec_count[j]!=0){
                int average_cycles= procsumm_table[i].total_cycles[j]/procsumm_table[i].rec_count[j];
                printf("%3d %7s %5d %-14d\n",
                        i, sys_call_description[j],procsumm_table[i].rec_count[j],average_cycles);
                
            }                   
        }
        printf("%32s\n","--------------------------------");  
    }
}