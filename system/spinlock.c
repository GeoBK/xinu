#include <xinu.h>

syscall sl_initlock(sl_lock_t *l)
{
    if(num_spinlocks<NSPINLOCKS)
    {
        l->flag=0;
        num_spinlocks++;
        return OK;
    }
    else
    {
        return SYSERR;
    }
    
    
}
syscall sl_lock(sl_lock_t *l)
{
    while(test_and_set((void*)&l->flag,1)==1);
    return OK;
}
syscall sl_unlock(sl_lock_t *l)
{
    l->flag=0;
    return OK;
}