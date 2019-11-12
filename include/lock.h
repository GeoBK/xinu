#define NSPINLOCKS 20
extern uint32 num_spinlocks;
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
extern uint32 num_locks;
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



#define NALOCKS 20
extern int32 num_activelocks;
typedef struct al_lock_t 
{
    uint32  flag;          /* lock flag */
    uint32  guard;
    queue   q;
    uint32  set_park_called;
    uint32  unpark_called;
    pid32   owner;
    int32   index;
} al_lock_t;
al_lock_t* al_lock_list[NALOCKS];
syscall al_initlock(al_lock_t *l); 
syscall al_lock(al_lock_t *l); 
syscall al_unlock(al_lock_t *l); 
bool8   al_trylock(al_lock_t *l); 

#define NPILOCKS  20
extern uint32 num_pilocks;
typedef struct pi_lock_t 
{
    uint32  flag;          /* lock flag */
    uint32  guard;
    queue   q;
    uint32  set_park_called;
    uint32  unpark_called;
    pid32   owner;    
    pri16	lockpriority;		/* lock priority			*/
        
} pi_lock_t;
syscall pi_initlock(pi_lock_t *l); 
syscall pi_lock(pi_lock_t *l);
syscall pi_unlock(pi_lock_t *l);