# file:	 crt01.s
# name:	 cole maxwell
# description:	 user startup module for uprog1.c

.globl ustart1, main1, exit
.text
			
ustart1:	movl $0x3ffff0, %esp         # reinit the stack
		movl $0, %ebp                # and frame pointer
		call main1                   # call main1 in the uprog1.c
                pushl %eax                   # push the retval=exit_code on stack
                call exit                    # call sysycall exit
               

