#include <lock.h>
#include <kernel.h>
#include <proc.h>
	//int nextlock;
//struct  lentry  locks[NLOCK];

void linit()
{

      kprintf(" \n---------------in linit()---------\n");
      nextlock = NLOCK - 1;
      kprintf(" \n-----nextlock ---- = %d\n", nextlock);

      struct lentry *lptr = NULL;
      int i;
      for (i = 0; i < NLOCK; i++)
      {
            (lptr = &locks[i])->lstate = LFREE;
            lptr->lqtail = 1 + (lptr->lqhead = newqueue());
            lptr->lwqtail = 1 + (lptr->lwqhead = newqueue());
            lptr->lrqtail = 1 + (lptr->lrqhead = newqueue());
            lptr->lacq = NOT_ACQUIRED;
            lptr->lcnt = 0;
      }
}