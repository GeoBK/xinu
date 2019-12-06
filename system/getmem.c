/* getmem.c - getmem */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  getmem  -  Allocate heap storage, returning lowest word address
 *------------------------------------------------------------------------
 */
char  	*getmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*prev, *curr, *leftover;

	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/

	prev = &memlist;
	curr = memlist.mnext;
	while (curr != NULL) {			/* Search free list	*/

		if (curr->mlength == nbytes) {	/* Block is exact match	*/
			prev->mnext = curr->mnext;
			memlist.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { /* Split big block	*/
			leftover = (struct memblk *)((uint32) curr +
					nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			memlist.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);
		} else {			/* Move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}
	restore(mask);
	return (char *)SYSERR;
}
int32 find_contiguous_vheap(uint32 frames)
{
	kprintf("inside find_contiguous_vheap!\n");
	uint32 free_frames=0;
    if(frames==0)return SYSERR;
	uint32 beg_frame,frame_count=XINU_PAGES;
	bool8 break_occured=1;
	
    pd_t *pd=(pd_t*)proctab[currpid].pdbr;
    int i,j;
    for(i=XINU_PAGES/(PAGE_SIZE/4);i<PAGE_SIZE/4;i++)
    {
		kprintf("i: %d\n",i);
		if(break_occured==1)
		{
			beg_frame=frame_count;
			free_frames=0;
			break_occured=0;
		}
		if(free_frames>=frames)
		{
			kprintf("i: %d\n",i);
			kprintf("beg_frame: %x\n",beg_frame);
			kprintf("free_frames: %u\n",free_frames);
			kprintf("frames: %u\n",frames);
			return beg_frame;
		}
		
        if(pd[i].pd_allocated==0)kprintf("Error!No allocation but directory is being accessed!!!\n");
        if(pd[i].pd_pres==1)
        {
            pt_t *pt= (pt_t*)(pd[i].pd_base<<12);
            for(j=0;j<PAGE_SIZE/4;j++)
            {
				kprintf("i: %d, j: %d\n",i,j);	
				if(break_occured==1)
				{
					beg_frame=frame_count;
					free_frames=0;
					break_occured=0;
				}
				if(free_frames>=frames)
				{
					kprintf("i: %d, j: %d\n",i,j);	
					return beg_frame;
				}				
                if(pt[j].pt_valid==0)
                {
                    ++free_frames;
                }
				else
				{					
					break_occured=1;
				}		
				++frame_count;		
            }
			
        }
		else
		{		
			kprintf("inside the part where pd.pres=0 - i: %d\n",i);
			free_frames=free_frames+(PAGE_SIZE/4);
			frame_count=frame_count+(PAGE_SIZE/4);
		}
		
    }
	return SYSERR;
}

char	*vmalloc(uint32 size)
{
	kprintf("Inside vmalloc\n");
	intmask	mask;			/* Saved interrupt mask		*/	
	mask = disable();
	uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);
	
	uint32 req_frames = size/PAGE_SIZE;
	if(size%PAGE_SIZE!=0)
	{
		req_frames++;
	}
	uint32 free_frame = find_contiguous_vheap(req_frames);
	if(free_frame==SYSERR)
	{
		write_cr3(old_pdbr);
		restore(mask);
		return SYSERR;
	}
	uint32 pd_index = free_frame>>10;
	uint32 pt_index = free_frame & 0x003FF;
	pd_t *pd = (pd_t*)proctab[currpid].pdbr;
	kprintf("req_frames: %u\n",req_frames);
	while(req_frames>0)
	{
		kprintf("pd_index: %u, pt_index: %u\n",pd_index, pt_index);
		
		if(pd[pd_index].pd_pres==1)
		{
			pt_t *pt=(pt_t*)(pd[pd_index].pd_base<<12);
			kprintf("pt base: %u\n",(uint32)pt);
			if(pt[pt_index].pt_pres==1)kprintf("Possible mistake in allocation pd_index: %x, pt_index: %x\n",pd_index,pt_index);
			pt[pt_index].pt_valid=1;
			pt[pt_index].pt_pres=0;
			pt[pt_index].pt_write=1;
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
			pd[pd_index].pd_base = allocate_next_table()>>12;
			kprintf("New page table at : %u\n",(uint32)pd[pd_index].pd_base);
			pd[pd_index].pd_valid= 1;
			pd[pd_index].pd_pres = 1;
		}
		
	}
	write_cr3(old_pdbr);
	restore(mask);
	return (char*)(free_frame<<12);
}

char  	*generic_getmem(
	  struct memblk* 	generic_freelist,
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*prev, *curr, *leftover;

	mask = disable();
	uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);
	if (nbytes == 0) {
		write_cr3(old_pdbr);
		restore(mask);
		return (char *)SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/

	prev = generic_freelist;
	curr = generic_freelist->mnext;
	while (curr != NULL) {			/* Search free list	*/

		if (curr->mlength == nbytes) {	/* Block is exact match	*/
			prev->mnext = curr->mnext;
			generic_freelist->mlength -= nbytes;
			write_cr3(old_pdbr);
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { /* Split big block	*/
			leftover = (struct memblk *)((uint32) curr +
					nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			generic_freelist->mlength -= nbytes;
			write_cr3(old_pdbr);
			restore(mask);
			return (char *)(curr);
		} else {			/* Move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}
	write_cr3(old_pdbr);
	restore(mask);
	return (char *)SYSERR;
}
