#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

uint32 num_activelocks=0;
process al_park(al_lock_t *l)
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

void al_setpark(al_lock_t *l,pid32 pid)
{
    intmask	mask;			/* Saved interrupt mask		*/
    mask = disable();

    //---------------------------critical section--------------------
    l->set_park_called=pid;
    //---------------------------------------------------------------

    restore(mask);    
}

void al_unpark(al_lock_t *l,pid32 pid)
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

bool8 checkinq(queue *q,pid32 pid)
{
    node* it = q->head;
    while(it!=NULL)
    {
        if(pid==it->pid)
        {
            return 1;
        }
        it=it->next;
    }
    return 0;
}

syscall al_initlock(al_lock_t *l)
{
    if(num_locks<NALOCKS)
    {
        al_lock_list[num_activelocks]=l;
        l->flag=0;
        l->guard=0;
        l->q.head=NULL;
        l->q.tail=NULL;
        l->index=num_activelocks;
        num_activelocks++;
        return OK;
    }
    else
    {
        return SYSERR;
    }
}
syscall al_lock(al_lock_t *l)
{
    sync_printf("Inside LOCK for PID -> %d \n",currpid);
    
    
    while(test_and_set(&l->guard,1)==1){sync_printf("spinning on lock guard (currpid = %d)\n",currpid);}    
    if(l->flag==0)
    {
        sync_printf("inside when flag =0 lock code part \n");
        l->owner=currpid;
        l->flag=1;
        l->guard=0;        
    }
    else
    {
        sync_printf("inside when flag =1 lock code part \n");
        struct	procent	*prptr;		/* Pointer to proc. table entry */
        queue   cycleq;
        enq(&cycleq,currpid);
        sync_printf("CYCleQ  -> ");
        printq(cycleq);
        pid32   cyclepid=l->owner;
        sync_printf("Current lock index - %d, current lock owner pid - %d",l->index,l->owner);
        prptr=&proctab[cyclepid];
        while(prptr->prlockindex!=-1)
        {
            if(checkinq(&cycleq,cyclepid)==1)
            {
                //printf that a cycle has been formed
                kprintf("lock_detected=");
                printq(cycleq);                
                break;
            }
            else
            {
                enq(&cycleq,cyclepid);
                sync_printf("Inside deadlock detection loop(else part) \n");
                cyclepid=al_lock_list[prptr->prlockindex]->owner;
                sync_printf("Next lock index - %d, next lock owner pid - %d",prptr->prlockindex,l->owner);
                prptr= &proctab[cyclepid];
            }
        }

        proctab[currpid].prlockindex=l->index;
        sync_printf("Did i shit my pants yet??? \n");
        enq(&(l->q),currpid);
        printq(l->q);
        al_setpark(l,currpid);
        l->guard=0;
        al_park(l);        
    }
    return OK;    
    
}
syscall al_unlock(al_lock_t *l)
{
    sync_printf("Inside UNLOCK for PID -> %d \n",currpid);
    while(test_and_set(&l->guard,1)==1){sync_printf("spinning on unlock guard (currpid= %d)\n",currpid);}

    if(currpid!=l->owner){
        sync_printf("Returning SYSERR currpid-> %d lockowner -> %d", currpid, l->owner);
        l->guard=0;
        return SYSERR;
    }
    if(l->q.head==NULL)
    {
        sync_printf("Inside unlock when q empty\n");
        l->flag=0;
        proctab[l->owner].prlockindex=-1;
        l->owner=0;     
        l->guard=0;
           
    }
    else
    {
        sync_printf("Inside unlock when q has elements\n");
        printq(l->q);
        pid32 pid = dq(&(l->q));  
        al_unpark(l,pid);  
        proctab[l->owner].prlockindex=-1;
        l->owner=pid;
        l->guard=0;
    }
    return OK;    
}