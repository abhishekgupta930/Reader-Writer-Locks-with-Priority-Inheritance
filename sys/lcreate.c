#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

SYSCALL lcreate()
{
      STATWORD ps;
      int lck, i;
      kprintf(" \n---------------in lcreate()---------\n");
     	// Check for errors and boundary conditions

      if (nextlock == 0)
      {
            restore(ps);
            return (SYSERR);
      }

     	// Scan all the locks and alot an unused one

      for (i = 0; i < NLOCK; i++)
      {
            lck = nextlock--;
            if (nextlock < 0)
            {
                  restore(ps);
                  return (SYSERR);
            }

            if (locks[lck].lstate == LFREE)
            {

                  locks[lck].lstate = LUSED;
                  restore(ps);
                  return (lck);
            }
      }
}