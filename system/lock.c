#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

void sync_printf(char *fmt, ...)

{
        intmask mask = disable();

        void *arg = __builtin_apply_args();

        __builtin_apply((void*)kprintf, arg, 100);

        restore(mask);

}
void enq(queue* q,pid32 pid)
{
    node *new_node=(node*)getmem(sizeof(node));
    new_node->pid=pid;
    new_node->next=NULL;
    q->tail=new_node;
    if(q->head==NULL)
    {
        q->head=new_node;
    }
}

pid32 dq(queue* q)
{
    if(q->head != NULL)
    {
        node* node_to_delete = q->head;
        pid32 pid = q->head->pid;
        q->head=q->head->next;
        if(q->tail==node_to_delete){
            q->tail=NULL;
        }
        freemem((char*)node_to_delete,sizeof(node));
        return pid;
    }
    return SYSERR;    
}

void printq(queue q)
{
    node* it = q.head;
    if(it==NULL)sync_printf("Q empty!!! \n");
    while(it != NULL)
    {
        sync_printf("%d->",it->pid);
        it=it->next;
    }
    sync_printf("\n");
}

process park(lock_t *l)
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

void setpark(lock_t *l,pid32 pid)
{
    intmask	mask;			/* Saved interrupt mask		*/
    mask = disable();

    //---------------------------critical section--------------------
    l->set_park_called=pid;
    //---------------------------------------------------------------

    restore(mask);    
}

void unpark(lock_t *l,pid32 pid)
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

syscall initlock(lock_t *l)
{
    l->flag=0;
    l->guard=0;
    l->q.head=NULL;
    l->q.tail=NULL;
    return OK;
}
syscall lock(lock_t *l)
{
    sync_printf("Inside LOCK for PID -> %d \n",currpid);
    while(test_and_set(&l->guard,1)==1){kprintf("spinning on lock guard \n");}

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
        enq(&(l->q),currpid);
        printq(l->q);
        setpark(l,currpid);
        l->guard=0;
        park(l);        
    }
    return OK;    
    
}
syscall unlock(lock_t *l)
{
    sync_printf("Inside UNLOCK for PID -> %d \n",currpid);
    while(test_and_set(&l->guard,1)==1){sync_printf("spinning on unlock guard (currpid= %d)\n",currpid);}

    if(currpid!=l->owner){
        sync_printf("Returning SYSERR currpid-> %d lockowner -> %d", currpid, l->owner);
        return SYSERR;
    }
    if(l->q.head==NULL)
    {
        sync_printf("Inside unlock when q empty\n");
        l->flag=0;
        l->guard=0;
        l->owner=0;
    }
    else
    {
        sync_printf("Inside unlock when q has elements\n");
        printq(l->q);
        pid32 pid = dq(&(l->q));  
        unpark(l,pid);  
        l->owner=pid;
        l->guard=0;
    }
    return OK;    
}