#define NSPINLOCKS 20
typedef struct sl_lock_t {
    uint32   flag;          /* lock flag */
} sl_lock_t;

syscall sl_initlock(sl_lock_t *l);
syscall sl_lock(sl_lock_t *l);
syscall sl_unlock(sl_lock_t *l);


