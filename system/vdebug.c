#include "xinu.h"
uint32 free_ffs_pages()
{
    return ffsmemlist.mlength/PAGE_SIZE;
}

uint32 free_swap_pages()
{
    return swapmemlist.mlength/PAGE_SIZE;
}

uint32 allocated_virtual_pages(pid32 pid)
{
    uint32 allocated_frames=0;
    uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);
    pd_t *pd=(pd_t*)proctab[pid].pdbr;
    int i,j;
    for(i=0;i<PAGE_SIZE/4;i++)
    {
        if(pd[i].pd_allocated==0)kprintf("Error!No allocation but directory is being accessed!!!\n");
        if(pd[i].pd_pres==1)
        {
            pt_t *pt= (pt_t*)(pd[i].pd_base<<12);
            for(j=0;j<PAGE_SIZE/4;j++)
            {
                if(pt[j].pt_valid==1)
                {
                    ++allocated_frames;
                }
            }
        }
    }
    write_cr3(old_pdbr);
    return allocated_frames;
}

uint32 used_ffs_frames(pid32 pid)
{
    uint32 used_frames=0;
    uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);
    pd_t *pd=(pd_t*)proctab[pid].pdbr;
    int i,j;
    for(i=XINU_PAGES/(PAGE_SIZE/4);i<PAGE_SIZE/4;i++)
    {
        if(pd[i].pd_allocated==0)kprintf("Error!No allocation but directory is being accessed!!!\n");
        if(pd[i].pd_pres==1)
        {
            pt_t *pt= (pt_t*)(pd[i].pd_base<<12);
            for(j=0;j<PAGE_SIZE/4;j++)
            {
                if(pt[j].pt_pres==1)
                {
                    ++used_frames;
                }
            }
        }
    }
    write_cr3(old_pdbr);
    return used_frames;
}

uint32 used_swap_frames(pid32 pid)
{
    uint32 old_pdbr=read_cr3();
	write_cr3(XINU_PAGES*PAGE_SIZE);
    pd_t *pd=(pd_t*)proctab[pid].pdbr;
    uint32 used_frames;
    int i,j;
    for(i=0;i<PAGE_SIZE/4;i++)
    {
        if(pd[i].pd_allocated==0)kprintf("Error!No allocation but directory is being accessed!!!\n");
        if(pd[i].pd_pres==1)
        {
            pt_t *pt= (pt_t*)(pd[i].pd_base<<12);
            for(j=0;j<PAGE_SIZE/4;j++)
            {
                if(pt[j].pt_pres==0 && pt[j].pt_swap==1)
                {
                    ++used_frames;
                }
            }
        }
    }
    write_cr3(old_pdbr);
    return used_frames;
}