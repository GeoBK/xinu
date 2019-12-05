#include <xinu.h>

/* Memory bounds */

void	*minheap;		/* Start of heap			*/
void	*maxheap;		/* Highest valid heap address		*/

/*------------------------------------------------------------------------
 * meminit - initialize memory bounds and the free memory list
 *------------------------------------------------------------------------
 */
void	meminit(void) {

       struct	memblk	*memptr;	/* Ptr to memory block		*/

       /* Initialize the free memory list */

       /* Note: we pre-allocate  the "hole" between 640K and 1024K */
	//maxheap already initialized in i386.c
//       maxheap = (void *)0x600000;	/* Assume 64 Mbytes for now */
       minheap = &end;

       memptr = memlist.mnext = (struct memblk *)roundmb(minheap);
       if ((char *)(maxheap+1) > HOLESTART) {
       	/* create two blocks that straddle the hole */
       	memptr->mnext = (struct memblk *)HOLEEND;
       	memptr->mlength = (int) truncmb((unsigned) HOLESTART -
            		 (unsigned)&end - 4);
       	memptr = (struct memblk *) HOLEEND;
       	memptr->mnext = (struct memblk *) NULL;
       	memptr->mlength = (int) truncmb( (uint32)maxheap - 
       			(uint32)HOLEEND - NULLSTK);
       } else {
       	/* initialize free memory list to one block */
       	memlist.mnext = memptr = (struct memblk *) roundmb(&end);
       	memptr->mnext = (struct memblk *) NULL;
       	memptr->mlength = (uint32) truncmb((uint32)maxheap -
       			(uint32)&end - NULLSTK);
       }
	   initialize_page_table();
	   kprintf("Switching on paging\n");
	   enable_paging();
	   kprintf("paging switched on\n");
	//    fflush(NULL);

       return;
}
void print_page_table()
{
	int i;
	pd_t* pdbr=(pd_t*)read_cr3();
	kprintf("Address\t : Content\n");
	for(i=0;i<PAGE_SIZE;i++)
	{
		kprintf("%x\t : %x | %u | %u | %u\n",(uint32)pdbr+i*4,pdbr[i].pd_base<<12,pdbr[i].pd_pres,pdbr[i].pd_mbz, pdbr[i].pd_fmb);
	}
}
void initialize_page_table()
{
	//Initialize the bits available for programmer to 0 to mark an invalid entry
	int i,j;
	pd_t *pd = (pd_t*)(XINU_PAGES*PAGE_SIZE);
	//first 1024 entries are for the system page directory
	for(i=0;i<PAGE_SIZE;i++)
	{
		pd[i].pd_pres 	= 0;	
		pd[i].pd_write 	= 0;
		pd[i].pd_user	= 0;
		pd[i].pd_pwt	= 0;
		pd[i].pd_pcd	= 0;
		pd[i].pd_acc	= 0;
		pd[i].pd_mbz	= 0;
		pd[i].pd_fmb	= 0;
		pd[i].pd_global = 0;
		pd[i].pd_valid 	= 1;		
		pd[i].pd_avail 	= 0;
		pd[i].pd_base	= 0;
		

	}
	for(i=1;i<MAX_PT_SIZE*PAGE_SIZE;i++)
	{

		pd[i].pd_pres 	= 0;	
		pd[i].pd_write 	= 0;
		pd[i].pd_user	= 0;
		pd[i].pd_pwt	= 0;
		pd[i].pd_pcd	= 0;
		pd[i].pd_acc	= 0;
		pd[i].pd_mbz	= 0;
		pd[i].pd_fmb	= 0;
		pd[i].pd_global = 0;
		pd[i].pd_valid 	= 0;		
		pd[i].pd_avail 	= 0;
		pd[i].pd_base	= 0;
	}
	int k=0;
	for(i=0;i<(XINU_PAGES+MAX_PT_SIZE+MAX_FFS_SIZE+MAX_SWAP_SIZE);)
	{
		if(pd[k].pd_base==0)
		{			
			uint32 new_pd=allocate_next_table();
			if(new_pd!=SYSERR)
			{
				pd[k].pd_base=new_pd>>12;
				pd[k].pd_pres=1;
			}				
		}
		for(j=0;j<PAGE_SIZE/4;j++)
		{
			pt_t *curr_ptb = (pt_t*)(pd[k].pd_base<<12);
			curr_ptb[j].pt_base=i;
			if(i<XINU_PAGES)
			{
				curr_ptb[j].pt_pres=1;
			}			
			//kprintf("%u\t : %u\n",(uint32)pdbr+i*4,pdbr[i].pd_base);
			i++;
		}
		k++;
		kprintf("i: %d, j:%d, k: %d\n",i,j,k);		
	}
	
	uint32 pdbr=((uint32)pd)&0x11111000;
	kprintf("pdbr: %x\n",pdbr);
	write_cr3(pdbr);
	//print_page_table();
}

uint32 allocate_next_table()
{
	uint32 i;
	pd_t *j;
	pd_t* pt_begin = (pd_t*)(XINU_PAGES*PAGE_SIZE);	
	
	for(i=0;i<MAX_PT_SIZE;i++)
	{
		if(pt_begin[i*(PAGE_SIZE/4)].pd_valid==0)
		{
			kprintf("New page table address : %x, i:%u, j: %x",&(pt_begin[i*(PAGE_SIZE/4)]),i,&(pt_begin[i*(PAGE_SIZE/4)]));
			for(j=&(pt_begin[i*(PAGE_SIZE/4)]);j<(pd_t*)((XINU_PAGES+i+1)*PAGE_SIZE);j++)
			{
				j->pd_valid=1;
				//kprintf("\tj: %x\n",j);
			}
			return (uint32)&(pt_begin[i*(PAGE_SIZE/4)]);
		}
	}
	return SYSERR;
}


