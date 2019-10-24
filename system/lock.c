#include <xinu.h>
#include <stdlib.h>
void enq(queue q,pid32 pid)
{
    node *new_node=malloc(sizeof(node));
    new_node->pid=pid;
    new_node->next=NULL;
    q.tail=new_node;
    if(q.head==NULL)
    {
        q.head=new_node;
    }
}

pid32 dq(queue q)
{
    if(q.head != NULL)
    {
        node* node_to_delete = q.head;
        pid32 pid = q.head->pid;
        q.head=q.head->next;
        if(q.tail==node_to_delete){
            q.tail==NULL;
        }
        free(node_to_delete);
        return pid;
    }
    return SYSERR;    
}

void park(lock_t *l)
{
    intmask	mask;			/* Saved interrupt mask		*/
    mask = disable();

    //---------------------------critical section--------------------
    if(l->set_park_called == l->unpark_called)
    {
        l->unpark_called=0;
        l->set_park_called=0;
    }
    else
    {
        struct	procent *prptr;		/* Ptr to process's table entry	*/
        umsg32	msg;			/* Message to return		*/        
        prptr = &proctab[currpid];
        if (prptr->prhasmsg == FALSE) {
            prptr->prstate = PR_RECV;
            resched();		/* Block until message arrives	*/
        }
        msg = prptr->prmsg;		/* Retrieve message		*/
        prptr->prhasmsg = FALSE;	/* Reset message flag		*/
        l->unpark_called=0;
        l->set_park_called=0;
    }    
    //-----------------------------------------------------------------

    restore(mask);    
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
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prhasmsg) {
		restore(mask);
		return SYSERR;
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
}
syscall lock(lock_t *l)
{
    while(test_and_set(&l->guard,1)==1);
    if(l->flag==0)
    {
        l->flag=1;        
        l->guard=0;        
    }
    else
    {
        enq(l->q,currpid);
        setpark(l,currpid);
        l->guard=0;
        park(l);
    }
    
    
}
syscall unlock(lock_t *l)
{
    while(test_and_set(&l->guard,1)==1);

    if(l->q.head==NULL)
    {
        l->flag=0;
        l->guard=0;
    }
    else
    {
        pid32 pid = dq(l->q);  
        unpark(l,pid);  
        l->guard=0;
    }
    
}