#include <xinu.h>
void pagefault_handler(uint32 error)
{
    intmask	mask;			/* Saved interrupt mask		*/	
	mask = disable();
	uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);
    //0 is for page fault caused by page not present
    //1 is for page fault caused by page-level protection violation
    uint32 violation = error&1;

    uint32 addr = read_cr2();
    uint32 pd_index = addr>>22;
    uint32 pt_index = (addr>>12)&0x003FF;
    pd_t *pd = (pd_t*)(porctab[currpid].pdbr);
    if(pd[pd_index].pd_pres==1)
    {
        pt_t *pt = (pt_t*)(pd[pd_index].pd_base<<12);
        if(pt[pt_index].pt_valid==1)
        {
            //Access type = 1 is for writes
            if(violation==1)
            {
                kprintf("P%d:: PROTECTION_FAULT\n",currpid);
            }
            else
            {
                // Since we are in the page fault handler it is implied that the present bit is 0
                if(pt[pt_index].pt_pres==1)kprintf("Present bit is 1 inside the page fault handler!!! \n");
                //Find physical memory location mapping 
                uint32 test = -1;
                kprintf("%%d: %d, %%u: %u",test, test);
                uint32 phys_addr = (uint32)generic_getmem(ffsmemlist,PAGE_SIZE);
                if(phys_addr==-1)
                {
                    //Do stuff to move the LRU to swap space
                    uint32 victim_pdbr, victim_pdi, victim_pti;
                    uint32 swap_addr = (uint32)generic_getmem(swapmemlist,PAGE_SIZE);
                    if(swap_addr==-1)
                    {
                        kprintf("Swap space full- ABORT!!!\n");
                    }
                    

                    //Change the pt_swap of the pt entry to 1 and the pt_base to the location of the swap space
                    pd_t* vpd=(pd_t*)victim_pdbr;
                    pt_t* vpt=(pt_t*)(vpd[victim_pdi].pd_base<<12);
                    memcpy(vpt[victim_pti].pt_base<<12,swap_addr,PAGE_SIZE);

                    vpt[victim_pti].pt_base=swap_addr>>12;
                    vpt[victim_pti].pt_swap=1;
                }
                if(pt[pt_index].pt_swap==1)
                {
                    memcpy(pt[pt_index].pt_base<<12,phys_addr,PAGE_SIZE);
                    generic_freemem(swapmemlist,pt[pt_index].pt_base<<12,PAGE_SIZE);
                }                
                pt[pt_index].pt_base=phys_addr>>12;
                pt[pt_index].pt_pres = 1;
                pt[pt_index].pt_swap=0;
            }
        }
        else
        {
            kprintf("P%d:: SEGMENTATION_FAULT\n",currpid);
        }
    }
    else
    {
        kprintf("P%d:: SEGMENTATION_FAULT\n",currpid);
        kill(currpid);
    }
    
    write_cr3(old_pdbr);
	restore(mask);
    
}