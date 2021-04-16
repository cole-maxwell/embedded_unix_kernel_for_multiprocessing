/* file: tunix.c core kernel code */

#include <stdio.h>
#include <cpu.h>
#include <gates.h>
#include "tty.h"
#include "tsyscall.h"
#include "tsystm.h"
#include "proc.h"
#include "sched.h"

extern IntHandler syscall; /* the assembler envelope routine    */
extern void _startupc(void);
extern void _start(void);
extern void ustart1(void);
extern void ustart2(void);
extern void ustart3(void);
extern int finale(void);

/* functions in this file */
int sysexit(int);
void k_init(void);
void shutdown(void);
void syscallc( int user_eax, int devcode, char *buff , int bufflen);

/* Record debug info in otherwise free memory between program and stack */
/* 0x300000 = 3M, the start of the last M of user memory on the SAPC */
#define DEBUG_AREA 0x300000
#define ESP0 0x3ffff0   /* process 0 stack starting address */
#define ESP1 0x280000	/* process 1 stack starting address */
#define ESP2 0x290000	/* process 2 stack starting address */
#define ESP3 0x2a0000	/* process 3 stack starting address */
char *debug_log_area = (char *)DEBUG_AREA;
char *debug_record;  /* current pointer into log area */ 
int saved_eflags;
int exitcodes[3];
int exitcodecount = 0;

#define MAX_CALL 6

/* optional part: syscall dispatch table */
static  struct sysent {
       short   sy_narg;        /* total number of arguments */
       int     (*sy_call)(int, ...);   /* handler */
} sysent[MAX_CALL];

  /* the process table with four entries loaded with dummy values */
PEntry proctab[] = {  
      {{0,0,0,0,0,0,0}, RUN, TTY1_OUTPUT, 0},  
      {{0,0,0,0,0,0,0}, RUN, TTY1_OUTPUT, 0},  
      {{0,0,0,0,0,0,0}, RUN, TTY1_OUTPUT, 0},  
      {{0,0,0,0,0,0,0}, RUN, TTY1_OUTPUT, 0},
};

PEntry *curproc;

/****************************************************************************/
/* k_init: this function for the initialize  of the kernel system*/
/****************************************************************************/

void k_init(){
  debug_record = debug_log_area; /* clear debug log */
  cli();
  ioinit();            /* initialize the deivce */ 
  set_trap_gate(0x80, &syscall);   /* SET THE TRAP GATE*/

  proctab[0].p_savedregs[SAVED_ESP] = ESP0;	
  proctab[0].p_savedregs[SAVED_PC] = (int) &_start;
  proctab[0].p_savedregs[SAVED_EFLAGS] = 0x1 << 9;  		
  proctab[0].p_savedregs[SAVED_EBP] = 0;  			
  proctab[0].p_status = RUN;

  proctab[1].p_savedregs[SAVED_ESP] = ESP1;  			
  proctab[1].p_savedregs[SAVED_PC] = (int) &ustart1;  		
  proctab[1].p_savedregs[SAVED_EFLAGS] = 0x1 << 9;  		
  proctab[1].p_savedregs[SAVED_EBP] = 0;  			
  proctab[1].p_status = RUN;

  proctab[2].p_savedregs[SAVED_ESP] = ESP2;  			
  proctab[2].p_savedregs[SAVED_PC] = (int) &ustart2;  		
  proctab[2].p_savedregs[SAVED_EFLAGS] = 0x1 << 9;  		
  proctab[2].p_savedregs[SAVED_EBP] = 0;  			
  proctab[2].p_status = RUN;

  proctab[3].p_savedregs[SAVED_ESP] = ESP3;  			
  proctab[3].p_savedregs[SAVED_PC] = (int) &ustart3;  		
  proctab[3].p_savedregs[SAVED_EFLAGS] = 0x1 << 9;  		
  proctab[3].p_savedregs[SAVED_EBP] = 0;  			
	proctab[3].p_status = RUN;

  curproc = &proctab[0];

  /* Note: Could set these with array initializers */
  /* Need to cast function pointer type to keep ANSI C happy */
  sysent[TREAD].sy_call = (int (*)(int, ...))sysread;
  sysent[TWRITE].sy_call = (int (*)(int, ...))syswrite;
  sysent[TEXIT].sy_call = (int (*)(int, ...))sysexit;
 
  sysent[TEXIT].sy_narg = 1;    /* set the arg number of function */
  sysent[TREAD].sy_narg = 3;
  sysent[TIOCTL].sy_narg = 3;
  sysent[TWRITE].sy_narg = 3;
  sti();    /* user runs with interrupts on */

  cli();    /* disable ints in CPU */
  schedule();   /* call the scheduler to decide which user program to run */
}

/* shut the system down */
void shutdown()
{
  kprintf("SHUTTING THE SYSTEM DOWN!\n");
  kprintf("Debug log from run:\n");
  kprintf("Marking kernel events as follows:\n");
  kprintf("  ~    COM2 output interrupt, ordinary char output\n");
  kprintf("  ~s   COM2 output interrupt, shutdown TX ints\n\n");
  kprintf("%s", debug_log_area);	/* the debug log from memory */
  kprintf("\nLEAVE KERNEL!\n\n");
  finale();		/* trap to Tutor */
}

/****************************************************************************/
/* syscallc: this function for the C part of the 0x80 trap handler          */
/* OK to just switch on the system call number here                         */
/* By putting the return value of syswrite, etc. in user_eax, it gets       */
/* popped back in sysentry.s and returned to user in eax                    */
/****************************************************************************/

void syscallc( int user_eax, int devcode, char *buff , int bufflen)
{
  int nargs;
  int syscall_no = user_eax;

  switch(nargs = sysent[syscall_no].sy_narg)
    {
    case 1:         /* 1-argument system call */
	user_eax = sysent[syscall_no].sy_call(devcode);   /* sysexit */
	break;
    case 3:         /* 3-arg system call: calls sysread or syswrite */
	user_eax = sysent[syscall_no].sy_call(devcode,buff,bufflen); 
	break;
    default: kprintf("bad # syscall args %d, syscall #%d\n",
		     nargs, syscall_no);
    }
} 

/****************************************************************************/
/* sysexit: this function for the exit syscall fuction */
/****************************************************************************/

int sysexit(int exit_code)  { 

  exitcodes[exitcodecount++] = exit_code;
  curproc->p_status = ZOMBIE;

  /* if there are any processes in RUN or BLOCKED state,
      call scheduler to pick the next process to run */
  if (!(proctab[1].p_status == ZOMBIE &&
        proctab[2].p_status == ZOMBIE &&
        proctab[3].p_status == ZOMBIE)) {
	  cli();			/* disable ints in CPU */
    schedule();
  }
  /* otherwise, all processes are ZOMBIE, so we shutdown */
  else {
    kprintf("\n\nAll processes are ZOMBIES");
    debug_log("(|3z-0)");
    kprintf("\nExit codes: uprog1 = %d, uprog2 = %d, uprog3 = %d, \n\n",
              exitcodes[0], exitcodes[1], exitcodes[2]);
	  shutdown();
  }
	return 0;    /* never happens, but keeps gcc happy */
}

/* append msg to memory log */
void debug_log(char *msg)
{
    strcpy(debug_record, msg);
    debug_record +=strlen(msg);
}


