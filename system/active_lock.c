#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

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
    
    
    while(test_and_set(&l->guard,1)==1){sync_printf("spinning on lock guard \n");}
    
    if(l->flag==0)
    {
        sync_printf("inside when flag =0 lock code part \n");
        l->owner=currpid;
        l->flag=1;        
        l->guard=0;        
    }
    else
    {
        struct	procent	*prptr;		/* Pointer to proc. table entry */
        queue   cycleq;
        enq(&cycleq,currpid);
        pid32   cyclepid=l->owner;
        prptr=proctab[cyclepid];
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
                cyclepid=al_lock_list[prptr->prlockindex]->owner;
                prptr= proctab[al_lock_list[prptr->prlockindex]->owner];
                enq(&cycleq,cyclepid);                
            }
        }
        proctab[currpid].prlockindex=l->index;
        sync_printf("inside when flag =1 lock code part \n");
        enq(&(l->q),currpid);
        printq(l->q);
        setpark(l,currpid);
        l->guard=0;
        park(l);        
    }
    return OK;    
    
}
syscall al_unlock(al_lock_t *l)
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
        proctab[l->owner].prlockindex=-1;
        l->owner=0;        
    }
    else
    {
        sync_printf("Inside unlock when q has elements\n");
        printq(l->q);
        pid32 pid = dq(&(l->q));  
        unpark(l,pid);  
        proctab[l->owner].prlockindex=-1;
        l->owner=pid;
        l->guard=0;
    }
    return OK;    
}