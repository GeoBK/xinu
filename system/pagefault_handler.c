#include <xinu.h>

process find_victim_frame(uint32* victim_pdbr, uint32* victim_pdi, uint32* victim_pti)
{
    static uint32 pr_ptr, pti_ptr=0, pdi_ptr=XINU_PAGES/(PAGE_SIZE/4);
    kprintf("Trying to find victim frame!!\n");
    while(1)
    {  
		pr_ptr %= NPROC;	/* Wrap around to beginning */
        //kprintf("process id: %d, pr_state: %u, system_process: %u \n",pr_ptr,proctab[pr_ptr].prstate,proctab[pr_ptr].sys_proc);
		if (proctab[pr_ptr].prstate != PR_FREE && proctab[pr_ptr].sys_proc == 0) 
        {
            pd_t* pd = (pd_t*)proctab[pr_ptr].initial_pdbr;
            if(pdi_ptr==(PAGE_SIZE/4))pdi_ptr=0;
            for(;pdi_ptr<(PAGE_SIZE/4);pdi_ptr++)
            {
                if(pd[pdi_ptr].pd_pres==1)
                {
                    pt_t* pt=(pt_t*)(pd[pdi_ptr].pd_base<<12);                    
                    for(;pti_ptr<(PAGE_SIZE/4);pti_ptr++)
                    {
                        //kprintf("pd: %x, pd_index: %d, pt: %x, pt_index: %d\n",pd,pdi_ptr,pt, pti_ptr);
                        if(pt[pti_ptr].pt_pres==1 && pt[pti_ptr].pt_valid==1)
                        {
                            if(pt[pti_ptr].pt_acc==0)
                            {
                                *victim_pdbr=(uint32)proctab[pr_ptr].initial_pdbr;
                                if((uint32)victim_pdbr==SYS_PD)kprintf("pdbr cannot be the same as the system pdbr... this probably means that this happened between a context switch!!!\n");
                                *victim_pdi=pdi_ptr;
                                *victim_pti=pti_ptr++;
                                if(pti_ptr==(PAGE_SIZE/4))
                                {
                                    pti_ptr=0;
                                    pdi_ptr++;
                                    if(pdi_ptr==(PAGE_SIZE/4))
                                    {
                                        pdi_ptr=0;
                                        pr_ptr++;
                                    }
                                }
                                //kprintf("victim details - pd: %x, pd_index: %d, pt_index: %d\n",*victim_pdbr,*victim_pdi,*victim_pti);
                                return OK;
                            }
                            else
                            {
                                pt[pti_ptr].pt_acc=0;
                            }
                        }
                        //kprintf("Finding infinite loop 1\n");
                    }
                    pti_ptr=0;
                }                
                //kprintf("Finding infinite loop 2, pdi_ptr: %d\n", pdi_ptr);                
            }
            pdi_ptr=XINU_PAGES/(PAGE_SIZE/4);
		} 
        pr_ptr++;       
        //kprintf("Finding infinite loop 3, pr_ptr: %d\n",pr_ptr);
	}
    return SYSERR;
}
void pagefault_handler(uint32 error)
{
    //kprintf("initial error: %x\n",error);
    
    intmask	mask;			/* Saved interrupt mask		*/	
	mask = disable();
	uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);
    //0 is for page fault caused by page not present
    //1 is for page fault caused by page-level protection violation
    // kprintf("error: %x\n",error);
    uint32 violation = error&1;

    uint32 addr = read_cr2();
    // kprintf("addr: %x\n",addr);
    uint32 pd_index = addr>>22;
    uint32 pt_index = (addr>>12)&0x003FF;
    pd_t *pd = (pd_t*)(proctab[currpid].pdbr);
    if(violation==1)
    {
        kprintf("P%d:: PROTECTION_FAULT\n",currpid);
        kill(currpid);
    }
    else
    {
        if(pd[pd_index].pd_pres==1)
        {
            // kprintf("deso!!\n");
            pt_t *pt = (pt_t*)(pd[pd_index].pd_base<<12);
            if(pt[pt_index].pt_valid==1)
            {
                // kprintf("deso2!!\n");
                // Since we are in the page fault handler it is implied that the present bit is 0
                if(pt[pt_index].pt_pres==1)kprintf("Present bit is 1 inside the page fault handler!!! \n");
                //Find physical memory location mapping                               
                uint32 phys_addr = (uint32)generic_getmem(&ffsmemlist,PAGE_SIZE);
                if(phys_addr==-1)
                { 
                    //kprintf("addr: %x\n",addr);
                    //kprintf("Entered SWAP operations!!\n");
                    //Do stuff to move the LRU to swap space
                    uint32 victim_pdbr, victim_pdi, victim_pti;
                    find_victim_frame(&victim_pdbr, &victim_pdi, &victim_pti);
                    //kprintf("victim frame found!!\n");
                    uint32 swap_addr = (uint32)generic_getmem(&swapmemlist,PAGE_SIZE);
                    if(swap_addr==-1)
                    {
                        kprintf("Swap space full- ABORT!!!\n");
                    }
                    

                    //Change the pt_swap of the pt entry to 1 and the pt_base to the location of the swap space
                    pd_t* vpd=(pd_t*)victim_pdbr;
                    pt_t* vpt=(pt_t*)(vpd[victim_pdi].pd_base<<12);
                    phys_addr=(pt_t*)(vpd[victim_pdi].pd_base<<12);
                    memcpy((void*)swap_addr,(void*)(vpt[victim_pti].pt_base<<12),PAGE_SIZE);

                    vpt[victim_pti].pt_base=swap_addr>>12;
                    vpt[victim_pti].pt_swap=1;
                    
                    //kprintf("Exiting SWAP operations!!\n");
                }
                if(pt[pt_index].pt_swap==1)
                {
                    kprintf("addr: %x\n",addr);
                    memcpy((void*)phys_addr,(void*)(pt[pt_index].pt_base<<12),PAGE_SIZE);
                    generic_freemem(&swapmemlist,(char*)(pt[pt_index].pt_base<<12),PAGE_SIZE);
                }                
                pt[pt_index].pt_base=phys_addr>>12;
                pt[pt_index].pt_pres = 1;
                pt[pt_index].pt_swap=0;
                
            }
            else
            {
                kprintf("P%d:: SEGMENTATION_FAULT\n",currpid);
                kill(currpid);
            }
        }
        else
        {
            kprintf("pd: %x, pd_index: %u, pt_base(?): %x\n",pd,pd_index,pd[pd_index].pd_base);
            kprintf("P%d:: SEGMENTATION_FAULT because of page directory entry being absent\n",currpid);
            kill(currpid);
        }
    }
    
    write_cr3(old_pdbr);
	restore(mask);
    
}