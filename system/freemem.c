/* freemem.c - freemem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  freemem  -  Free a memory block, returning the block to the free list
 *------------------------------------------------------------------------
 */
syscall	freemem(
	  char		*blkaddr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*next, *prev, *block;
	uint32	top;

	mask = disable();
	if ((nbytes == 0) || ((uint32) blkaddr < (uint32) minheap)
			  || ((uint32) blkaddr > (uint32) maxheap)) {
		restore(mask);
		return SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	block = (struct memblk *)blkaddr;

	prev = &memlist;			/* Walk along free list	*/
	next = memlist.mnext;
	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->mnext;
	}

	if (prev == &memlist) {		/* Compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->mlength;
	}

	/* Ensure new block does not overlap previous or next blocks	*/

	if (((prev != &memlist) && (uint32) block < top)
	    || ((next != NULL)	&& (uint32) block+nbytes>(uint32)next)) {
		restore(mask);
		return SYSERR;
	}

	memlist.mlength += nbytes;

	/* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* Coalesce with previous block	*/
		prev->mlength += nbytes;
		block = prev;
	} else {			/* Link into list as new node	*/
		block->mnext = next;
		block->mlength = nbytes;
		prev->mnext = block;
	}

	/* Coalesce with next block if adjacent */

	if (((uint32) block + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
	}
	restore(mask);
	return OK;
}




syscall	generic_freemem(
	  struct memblk* 	generic_freelist,
	  char		*blkaddr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*next, *prev, *block;
	uint32	top;
	

	mask = disable();
	uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);
	if ((nbytes == 0) || ((uint32) blkaddr < (uint32) minheap)
			  || ((uint32) blkaddr > (uint32) maxheap)) {
		write_cr3(old_pdbr);
		restore(mask);
		return SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	block = (struct memblk *)blkaddr;

	prev = generic_freelist;			/* Walk along free list	*/
	next = generic_freelist->mnext;
	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->mnext;
	}

	if (prev == generic_freelist) {		/* Compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->mlength;
	}

	/* Ensure new block does not overlap previous or next blocks	*/

	if (((prev != generic_freelist) && (uint32) block < top)
	    || ((next != NULL)	&& (uint32) block+nbytes>(uint32)next)) {
		write_cr3(old_pdbr);
		restore(mask);
		return SYSERR;
	}

	generic_freelist->mlength += nbytes;

	/* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* Coalesce with previous block	*/
		prev->mlength += nbytes;
		block = prev;
	} else {			/* Link into list as new node	*/
		block->mnext = next;
		block->mlength = nbytes;
		prev->mnext = block;
	}

	/* Coalesce with next block if adjacent */

	if (((uint32) block + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
	}
	write_cr3(old_pdbr);
	restore(mask);
	return OK;
}


syscall	vfree(char * addr, uint32 size)
{
	//Find out when vfree would fail (Spec talks about failure but doesnt mention why it might fail)
	//Im freeing space even when there is no mapping in the page directory.
	intmask	mask;			/* Saved interrupt mask		*/
	mask = disable();
	uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);

	uint32 pd_index=size>>22;
	uint32 pt_index= (size>>12)&0x003FF;

	uint32 req_frames = size/PAGE_SIZE;
	if(size%PAGE_SIZE!=0)
	{
		req_frames++;
	}
	pd_t *pd=(pd_t*)proctab[currpid].pdbr;

	while(req_frames>0)
	{
		if(pd[pd_index].pd_pres==1)
		{
			pt_t *pt=(pt_t*)(pd[pd_index].pd_base<<12);
			kprintf("Freeing pd_index: %d, pt_index: %d\n",pd_index,pt_index);
			pt[pt_index].pt_valid=0;
			pt[pt_index].pt_pres=0;
			pt[pt_index].pt_write=0;
			--req_frames;
			++pt_index;
			if(pt_index==(PAGE_SIZE/4))
			{
				pd_index++;
				pt_index=0;
			}
		}
		else
		{
			--req_frames;
			pd_index++;
			if(pt_index==(PAGE_SIZE/4))
			{
				pd_index++;
				pt_index=0;
			}			
		}
	}  

	write_cr3(old_pdbr);
	restore(mask);
	return OK;
}
