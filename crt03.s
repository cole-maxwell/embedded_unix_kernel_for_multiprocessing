# file:	 crt03.s
# name:	 cole maxwell
# description:	 user startup module for uprog3.c

.globl ustart3, main3, exit
.text
			
ustart3:	movl $0x3ffff0, %esp         # reinit the stack
		movl $0, %ebp                # and frame pointer
		call main3                   # call main3 in the uprog3.c
                pushl %eax                   # push the retval=exit_code on stack
                call exit                    # call sysycall exit
               

