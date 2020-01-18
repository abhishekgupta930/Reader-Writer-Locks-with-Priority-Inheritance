/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

        kprintf("\n *******In kill  currpid  =%d********\n",currpid);

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);

  	int i;
        //int args[NLOCK];
        for(i=NLOCK-1;i>=0;i--) {
                if (proctab[pid].plocks[i].status == L_CREATED) {
                        kprintf("\n ++++++++++Raj+++++++++++\n");
                        if (proctab[pid].plocks[i].waiting == FALSE) {
                                kprintf("\n\nkill release currpid = %d lock id =  %d\n\n",pid, i);
                                releaseall(1, i);
                        }
                        else{
					
                                kprintf("\n\n dequeue operation : lock : %d , currpid - %d , pid :  %d\n\n", i,currpid, pid);
                                kprintf("\n Abhinab\n");
                                dequeue(pid);
                                kprintf("\n before setprio in kill\n");
                                set_lprio(i);
                        }
                }
        }


	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
