#define NSPINLOCKS 20
typedef struct sl_lock_t {
    uint32   flag;          /* lock flag */
} sl_lock_t;

syscall sl_initlock(sl_lock_t *l);
syscall sl_lock(sl_lock_t *l);
syscall sl_unlock(sl_lock_t *l);

typedef struct node_struct
{
    pid32 pid;
    struct node_struct* next;
}node;

typedef struct queue
{
    node* head;
    node* tail;
}queue;



void enq(queue* q,pid32 pid);

pid32 dq(queue* q);


#define NLOCKS 20
typedef struct lock_t 
{
    uint32  flag;          /* lock flag */
    uint32  guard;
    queue   q;
    uint32  set_park_called;
    uint32  unpark_called;
    pid32   owner;
} lock_t;

process park(lock_t *l);

void setpark(lock_t *l,pid32 pid);

void unpark(lock_t *l,pid32 pid);


syscall initlock(lock_t *l); 
syscall lock(lock_t *l); 
syscall unlock(lock_t *l); 