# file:	 crt02.s
# name:	 cole maxwell
# description:	 user startup module for uprog2.c

.globl ustart2, main2, exit
.text
			
ustart2:	movl $0x3ffff0, %esp         # reinit the stack
		movl $0, %ebp                # and frame pointer
		call main2                   # call main2 in the uprog2.c
                pushl %eax                   # push the retval=exit_code on stack
                call exit                    # call sysycall exit
               

