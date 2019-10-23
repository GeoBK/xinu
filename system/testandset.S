/* testandset.S - test_and_set (for x86) */

		.text
		.globl	test_and_set

/*------------------------------------------------------------------------
 * test_and_set -  X86 atomic assignment; the call is test_and_set(&value, new_value)
 *
 *    uint32 test_and_set(uint32 *ptr, uint32 new_value) {
 *        uint32 old_value = *ptr;
 *        *ptr = new_value;
 *        return old_value;
 *    }
 *------------------------------------------------------------------------
 */
test_and_set:
        
		pushl	%ebp		/* Push ebp onto stack		*/
		movl	%esp, %ebp	/* Record current SP in ebp	*/

        movl    8(%ebp), %ecx   /* move the pointer into ecx */
        movl    12(%ebp), %eax  /* move the new value into ebx */

        xchgl   (%ecx), %eax    /* atomic operation to exchange value in memory with new_value */
        
        movl    %ebp, %esp  /* restore value of sp from ebp */
        popl    %ebp        /* pop the ebp value of previous frame into ebp */
        ret

