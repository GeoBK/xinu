#include <xinu.h>

syscall sl_initlock(sl_lock_t *l)
{
    l->flag=0;
}
syscall sl_lock(sl_lock_t *l)
{
    while(test_and_set((void*)&l->flag,1)==1);
}
syscall sl_unlock(sl_lock_t *l)
{
    l->flag=0;
}