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
    int32 i,j;
    //TODO: make this wait on a semaphore for console out temporary hack 
    

    printf("%3s %7s %5s %14s\n",
		   "pid", "syscall", "count", "average cycles");

	printf("%3s %7s %5s %14s\n",
		   "---", "-------", "-----", "--------------");    
    double a=5;
    kprintf("double value: %f\n",a);

    unsigned long long b=6;
    kprintf("unsigned long long value: %u\n", b);
    for(i=1;i<=max_pid;i++){        
        for(j=create_enum; j< number_of_system_calls; j++){
            if(procsumm_table[i].rec_count[j]!=0){
                double average_cycles= procsumm_table[i].total_cycles[j]/(double)procsumm_table[i].rec_count[j];
                kprintf("total cycles %u \n",procsumm_table[i].total_cycles[j]);
                printf("%3d %7s %5d %f\n",
                        i, sys_call_description[j],procsumm_table[i].rec_count[j],average_cycles);
                
            }                   
        }
        printf("%32s\n","--------------------------------");  
    }
}