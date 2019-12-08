/* pagefault_handler_disp.s - pagefault_handler_disp (x86) */

/*------------------------------------------------------------------------
 * pagefault_handler_disp  -  Interrupt dispatcher for page faults
 *------------------------------------------------------------------------
 */
		.text
		.globl	pagefault_handler_disp		# Page fault interrupt dispatcher 
pagefault_handler_disp:
        push %eax
        movl 4(%esp), %eax
		pushal			# Save registers
		cli			# Disable further interrupts
		push %eax
		call	pagefault_handler	# Call high level handler
        pop %eax
		sti			# Restore interrupt status
		popal	
        pop %eax		# Restore registers
		add $4, %esp		# Skip error message	
		iret			# Return from interrupt
