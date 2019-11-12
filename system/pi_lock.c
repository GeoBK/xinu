#include <xinu.h>
uint32 num_pilocks=0;
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

#define MAX_PRIORITY 30000
process pi_park(pi_lock_t *l)
{
    intmask	mask;			/* Saved interrupt mask		*/
    mask = disable();
    kprintf("Inside park\n");
    //---------------------------critical section--------------------
    if(l->set_park_called == l->unpark_called && l->set_park_called !=0)
    {
        kprintf("Inside that instance when park is called after unpark is called!!!\n");
        l->unpark_called=0;
        l->set_park_called=0;
    }
    else
    {
        kprintf("pid : %d will now be put to sleep\n",currpid);
        struct	procent *prptr;		/* Ptr to process's table entry	*/        
        prptr = &proctab[currpid];
        if (prptr->prhasmsg == FALSE) {
            kprintf("pid : %d will now be put to sleep 2\n",currpid);
            prptr->prstate = PR_RECV;
            resched();		/* Block until message arrives	*/
        }        
        prptr->prhasmsg = FALSE;	/* Reset message flag		*/
        l->unpark_called=0;
        l->set_park_called=0;
    }    
    //-----------------------------------------------------------------	
	
	restore(mask);		/* Restore interrupts */
	return OK;
      
}

void pi_setpark(pi_lock_t *l,pid32 pid)
{
    intmask	mask;			/* Saved interrupt mask		*/
    mask = disable();

    //---------------------------critical section--------------------
    l->set_park_called=pid;
    //---------------------------------------------------------------

    restore(mask);    
}

void pi_unpark(pi_lock_t *l,pid32 pid)
{    
    intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();

	if (isbadpid(pid)) {
		restore(mask);
		return;
	}

	prptr = &proctab[pid];
	while (prptr->prhasmsg) {
		ready(currpid); // modified here - if phasmsg was set another process might have sent a message. Wait for receiver to receive and clean the flag
		//restore(mask); OLD CODE
		//return SYSERR; OLD CODE
	}
	prptr->prmsg = currpid;		/* Deliver message		*/
	prptr->prhasmsg = TRUE;		/* Indicate message is waiting	*/

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
    l->unpark_called=pid;
	restore(mask);		/* Restore interrupts */    
}

syscall pi_initlock(pi_lock_t *l)
{
    if(num_locks<NPILOCKS)
    {
        l->flag=0;
        l->guard=0;
        l->q.head=NULL;
        l->q.tail=NULL;
        num_locks++;
        return OK;
    }
    else
    {
        return SYSERR;
    }    
}
syscall pi_lock(pi_lock_t *l)
{
    sync_debug_out("Inside LOCK for PID -> %d \n",currpid);
    while(test_and_set(&l->guard,1)==1){sleepms(QUANTUM);sync_debug_out("spinning on lock guard \n");}    
    if(l->flag==0)
    {
        sync_debug_out("inside when flag =0 lock code part \n");
        l->owner=currpid;
        l->flag=1;   
        l->initialpriority=proctab[currpid].prprio; 
        l->lockpriority=proctab[currpid].prprio;     
        l->guard=0;    
            
    }
    else
    {
        sync_debug_out("inside when flag =1 lock code part \n");
        enq(&(l->q),currpid);
        printq(l->q);
        if(proctab[currpid].prprio>l->lockpriority)
        {
            sync_printf("priority_change=P%d::%d-%d",l->owner,l->lockpriority,proctab[currpid].prprio);
            l->lockpriority=proctab[currpid].prprio;  
            proctab[l->owner].prprio=l->lockpriority;
        }
        pi_setpark(l,currpid);
        l->guard=0;
        pi_park(l);        
    }
    return OK;    
    
}
syscall pi_unlock(pi_lock_t *l)
{
    sync_debug_out("Inside UNLOCK for PID -> %d \n",currpid);
    while(test_and_set(&l->guard,1)==1){sleepms(QUANTUM);sync_debug_out("spinning on unlock guard (currpid= %d)\n",currpid);}

    if(currpid!=l->owner){
        sync_debug_out("Returning SYSERR currpid-> %d lockowner -> %d", currpid, l->owner);
        return SYSERR;
    }
    if(l->q.head==NULL)
    {
        sync_debug_out("Inside unlock when q empty\n");
        l->flag=0;
        if(proctab[l->owner].prprio!=l->initialpriority)
        {
            sync_printf("priority_change=P%d::%d-%d",l->owner,proctab[l->owner].prprio,l->initialpriority);
        }
        proctab[l->owner].prprio=l->initialpriority;        
        l->owner=0;
        l->lockpriority=0;
        l->initialpriority=0;
        l->guard=0;
        
    }
    else
    {
        sync_debug_out("Inside unlock when q has elements\n");
        pid32 oldowner=l->owner;
        pri16 oldpriority=l->initialpriority;
        
        printq(l->q);
        pid32 pid = dq(&(l->q)); 
        l->owner=pid; 
        l->initialpriority=proctab[l->owner].prprio;
        l->lockpriority=l->initialpriority;
        pri16 maxpri=maxpriority(l->q);
        if(maxpri>l->lockpriority)
        {
            sync_printf("priority_change=P%d::%d-%d",l->owner,l->lockpriority,maxpri);
            l->lockpriority=maxpri;
            proctab[l->owner].prprio=maxpri;
        }
        pi_unpark(l,pid);        
        l->guard=0;
        proctab[oldowner].prprio=oldpriority;
    }
    return OK;    
}