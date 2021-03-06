# file :	ulib.s
# name :	joseph tam paul hui
#date  :	3-2-97
#description:	assembler routine for read,write, ioctl1,exit
				

.globl write,read,ioctl,exit	
.text
	
write:	pushl %ebx                    # save the value of ebx
	movl 8(%esp),%ebx              
	movl 12(%esp),%ecx
	movl 16(%esp),%edx
	movl $3,%eax
        int $0x80                     # call trap handler
	popl  %ebx                    # restore the value of ebx
	ret

read:	pushl %ebx                       # save the value of ebx
	movl 8(%esp),%ebx
	movl 12(%esp),%ecx
	movl 16(%esp),%edx
	movl $2,%eax
        int $0x80                     # call trap handler      
	popl  %ebx                    # restore the value of ebx
	ret
	
ioctl:	pushl %ebx                     # save the value of ebx
	movl 8(%esp),%ebx
	movl 12(%esp),%ecx
	movl 16(%esp),%edx
	movl $5,%eax
        int $0x80                     # call trap handler
	popl  %ebx                    # restore the value of ebx
	ret
			
exit:	pushl %ebx                      # save the value of ebx
	movl 8(%esp),%ebx
	movl $1,%eax          
        int $0x80                      # call trap handler
	popl  %ebx                     # restore the value of ebx
	ret	


