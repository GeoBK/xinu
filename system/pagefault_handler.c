#include <xinu.h>
void pagefault_handler()
{
    kprintf("Page fault handler called\n");
}