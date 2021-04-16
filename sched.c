/*********************************************************************
*
*       file:           sched.c
*       author:         cole maxwell
*
*       non-preemptive scheduler that picks a process to run and runs it
*       until it exits or blocks (on I/O or waiting for another process)
*
*/
#include "proc.h"
#include "sched.h"
#include "stdio.h"

extern void ustart1(void);
extern void ustart2(void);
extern void ustart3(void);
extern int asmswtch();
extern int debug_log(char *msg);

void schedule() {

    /* loop through the PEntry table while there are
        any processes in RUN or BLOCKED states */
    while (!(proctab[1].p_status == ZOMBIE &&
             proctab[2].p_status == ZOMBIE &&
             proctab[3].p_status == ZOMBIE)) {

        /* locate the first process in RUN state */

        if (proctab[1].p_status == RUN) {
            curproc =  &proctab[1];
            debug_log("|(0-1)");
            /* call asmswtch() to switch from process 0 (kernel) to process 1
                letting process 1 call exit to break out of loop */
            asmswtch(&proctab[0], &proctab[1]);
        }
        else if (proctab[2].p_status == RUN) {
            curproc =  &proctab[2];
            debug_log("|(1z-2)");
            /* call asmswtch() to switch from process 1 to process 2
                letting process 2 call exit to break out of loop */
            asmswtch(&proctab[1], &proctab[2]);
        }
        else if (proctab[3].p_status == RUN) {
            curproc =  &proctab[3];
            debug_log("|(2z-3)");
            /* call asmswtch() to switch from process 2 to process 3
                letting process 3 call exit to break out of loop */
            asmswtch(&proctab[2], &proctab[3]);
        }
    }
    /* once we exit the while loop, all processes are ZOMBIES,
        so transfer control back to kernel */
    curproc = &proctab[0];
    debug_log("|(3z-0)");
    /* call asmswtch() to switch from process 3 to process 0 (kernel)
        letting kernel call exit and shutdown system */
    asmswtch(&proctab[3], &proctab[0]);
}

void sleep () {
    for (int i = 1; i < 4; i++) {
        if (proctab[i].p_status == RUN) {
            proctab[i].p_status = BLOCKED;
        }
    }
}

void wakeup() {
    for (int i = 1; i < 4; i++) {
        if (proctab[i].p_status == BLOCKED) {
            proctab[i].p_status = RUN;
        }
    }
}
