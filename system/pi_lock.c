#include <xinu.h>
uint32 num_pilocks=0;
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

#define MAX_PRIORITY 30000
sl_lock_t proc_tab_mutex;
sl_initlock(&proc_tab_mutex);
process pi_park(pi_lock_t *l)
{
    intmask	mask;			/* Saved interrupt mask		*/
    mask = disable();
    sync_debug_out("Inside park\n");
    //---------------------------critical section--------------------
    if(l->set_park_called == l->unpark_called && l->set_park_called !=0)
    {
        sync_debug_out("Inside that instance when park is called after unpark is called!!!\n");
        l->unpark_called=0;
        l->set_park_called=0;
    }
    else
    {
        sync_debug_out("pid : %d will now be put to sleep\n",currpid);
        struct	procent *prptr;		/* Ptr to process's table entry	*/        
        prptr = &proctab[currpid];
        if (prptr->prhasmsg == FALSE) {
            sync_debug_out("pid : %d will now be put to sleep 2\n",currpid);
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
        //sync_debug_out("inside when flag =0 lock code part \n");
        preempt = QUANTUM;
        l->owner=currpid;
        l->flag=1;   
        sl_lock(&proc_tab_mutex);
        proctab[l->owner].initialpriority=proctab[currpid].prprio;
        l->lockpriority=proctab[currpid].prprio;   
        sl_unlock(&proc_tab_mutex);           
        l->guard=0;    
            
    }
    else
    {
        //sync_debug_out("inside when flag =1 lock code part \n");
        preempt = QUANTUM;
        enq(&(l->q),currpid);
        //printq(l->q);
        sl_lock(&proc_tab_mutex);
        if(proctab[currpid].prprio>l->lockpriority)
        {
            sync_printf("priority_change=P%d::%d-%d\n",l->owner,l->lockpriority,proctab[currpid].prprio);
            preempt = QUANTUM;
            l->lockpriority=proctab[currpid].prprio;  
            proctab[l->owner].prprio=l->lockpriority;
            queuetab[queuetab[l->owner].qnext].qprev = queuetab[l->owner].qprev;
            queuetab[queuetab[l->owner].qprev].qnext = queuetab[l->owner].qnext;
            insert(l->owner, readylist, l->lockpriority);
        }
        sl_unlock(&proc_tab_mutex);
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
        //sync_debug_out("Inside unlock when q empty\n");
        preempt = QUANTUM;
        l->flag=0;
        sl_lock(&proc_tab_mutex);
        if(proctab[l->owner].prprio!=proctab[l->owner].initialpriority)
        {
            sync_printf("priority_change=P%d::%d-%d\n",l->owner,proctab[l->owner].prprio,proctab[l->owner].initialpriority);
            preempt = QUANTUM;
        }        
        proctab[l->owner].prprio=proctab[l->owner].initialpriority;   
        sl_unlock(&proc_tab_mutex);     
        l->owner=0;
        l->lockpriority=0;        
        l->guard=0;
        
    }
    else
    {
        //sync_debug_out("Inside unlock when q has elements\n");
        preempt = 2*QUANTUM;
        pid32 oldowner=l->owner;
        pri16 oldpriority=proctab[l->owner].initialpriority;
        
        // printq(l->q);
        pid32 pid = dq(&(l->q)); 
        l->owner=pid; 
        proctab[l->owner].initialpriority=proctab[l->owner].prprio;
        l->lockpriority=proctab[l->owner].initialpriority;
        pri16 maxpri=maxpriority(l->q);
        if(maxpri>l->lockpriority)
        {
            sync_printf("priority_change=P%d::%d-%d\n",l->owner,l->lockpriority,maxpri);
            preempt = QUANTUM;
            l->lockpriority=maxpri;
            proctab[l->owner].prprio=maxpri;
            queuetab[queuetab[l->owner].qnext].qprev = queuetab[l->owner].qprev;
            queuetab[queuetab[l->owner].qprev].qnext = queuetab[l->owner].qnext;	        
            insert(l->owner, readylist, l->lockpriority);
        }
        pi_unpark(l,pid);        
        l->guard=0;
        if(proctab[oldowner].prprio!=oldpriority)
        {
            sync_printf("priority_change=P%d::%d-%d\n",oldowner,proctab[oldowner].prprio,oldpriority);
        }
        proctab[oldowner].prprio=oldpriority;
    }
    return OK;    
}