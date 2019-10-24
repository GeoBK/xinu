#include <xinu.h>

syscall initlock(lock_t *l)
{
    l->flag=0;
    l->guard=0;
    l->q.head=NULL;
    l->q.tail=NULL;
}
syscall lock(lock_t *l)
{
    while(test_and_set(l->guard,1)==1);
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
    while(test_and_set(l->guard,1)==1);
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