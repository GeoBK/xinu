/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	i,j;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		kprintf("returning syserr!");
		restore(mask);		
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	kprintf("Just before freeing up page directories for the process!!\n");
	pd_t *pd=(pd_t*)proctab[currpid].pdbr;
	if((uint32)pd!=SYS_PD)
	{
		uint32 old_pdbr=read_cr3();
		write_cr3(XINU_PAGES*PAGE_SIZE);
		kprintf("pd: %x\n",pd);
		for(i=0;i<PAGE_SIZE/4;i++)
		{        
			if(pd[i].pd_pres==1 && i>=(XINU_PAGES/(PAGE_SIZE/4)))
			{
				pt_t *pt= (pt_t*)(pd[i].pd_base<<12);
				kprintf("pt: %x\n",pt);
				for(j=0;j<PAGE_SIZE/4;j++)
				{
					//kprintf("kill - i: %d, j: %d\n",i,j);
					if(pt[j].pt_pres==1)
					{
						kprintf("In kill calling freemem\n");
						generic_freemem(&ffsmemlist,(char*)(pt[j].pt_base<<12),PAGE_SIZE);
					}
					pt[j].pt_pres=0;
					pt[j].pt_valid=0;
					pt[j].pt_swap=0;
					pt[j].pt_allocated=0;
				}
			}
			else if(i>=(XINU_PAGES/(PAGE_SIZE/4)))
			{
				//kprintf("kill - i: %d\n",i);
				pd[i].pd_pres=0;
				pd[i].pd_valid=0;
				pd[i].pd_swap=0;
				pd[i].pd_allocated=0;	
			}		
		}
		write_cr3(old_pdbr);
	}
    

	freestk(prptr->prstkbase, prptr->prstklen);

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
