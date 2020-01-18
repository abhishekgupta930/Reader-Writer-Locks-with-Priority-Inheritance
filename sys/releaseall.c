#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

void release_highest_lock_priority_process(int lck, int hpid)
{
      int prio;
      dequeue(hpid);
      printf("\n........RELEASE: process being released from Lock's queue. process pid = %d........\n", hpid);
      prio = (proctab[hpid].pinh != -1) ? (proctab[hpid].pinh) : (proctab[hpid].pprio);
      set_pcb_on_release(hpid, lck);
      insert(hpid, rdyhead, prio);
}

void set_pcb_on_release(int hpid, int lck)
{
      proctab[hpid].pstate = PRREADY;
      proctab[hpid].plocks[lck].blocking_time = 0;
      proctab[hpid].plocks[lck].waiting = FALSE;

}

execute_defaultSteps(int lck, struct lentry *lptr)
{

      int newpid, next_writer_pid, next_reader_pid, writer_blocking_time;

      proctab[currpid].plocks[lck].status = L_DELETED;

     	//if No writer but only Reader in Queue
      if (is_writer_queue_empty(lptr) && !(is_reader_queue_empty(lptr)))
      {

            kprintf("\n........Only Readers in Lock's Reader Queue(). No Writer in Lock's Writer's Queue........\n");
            int tmp;
            do {
                  newpid = q[lptr->lrqtail].qprev;
                  tmp = q[newpid].qprev;
                 	// kprintf("\nreader being released %d\n", newpid);
                  release_highest_lock_priority_process(lck, newpid);
                  lptr->lcnt++;
                  newpid = tmp;
            } while (newpid != lptr->lrqhead);
            lptr->lacq = ACQUIRED_BY_READER;
      }

     	//Get highest priority writer from Lock's writer Queue

      next_writer_pid = get_next_writer(lck, lptr->lwqhead, lptr->lwqtail);
      kprintf("\n........Next Writer pid = %d........\n", next_writer_pid);

     	//Get highest priority reader from Lock's writer Queue

      next_reader_pid = get_next_reader(lck, lptr->lrqhead, lptr->lrqtail);
      kprintf("\n........Next Reader pid = %d........\n", next_reader_pid);

     	// Only Writers, No reades left

      if (is_reader_queue_empty(lptr) && !(is_writer_queue_empty(lptr)))
      {
            release_highest_lock_priority_process(lck, next_writer_pid);
            lptr->lacq = ACQUIRED_BY_WRITER;
      }

     	//If Both reader and Writers are present highest priority process will be released 
      if (!(is_writer_queue_empty(lptr)) && !(is_reader_queue_empty(lptr)))
      {
            kprintf("\n........Both Reader and Writers are present in Lock's Reader and Writer Queue ........\n");
            kprintf("\n........next_writer_pid.priority = %d,  next_reader_pid.priority = %d........\n", q[next_writer_pid].qkey, q[next_reader_pid].qkey);
            if (proctab[next_writer_pid].plocks[lck].priority > proctab[next_reader_pid].plocks[lck].priority)
            {
                  kprintf("\n........Writer being released as it has more priority than Reader........\n");
                  release_highest_lock_priority_process(lck, next_writer_pid);
                  lptr->lacq = ACQUIRED_BY_WRITER;
            }
            else if (proctab[next_writer_pid].plocks[lck].priority < proctab[next_reader_pid].plocks[lck].priority)
            {
                  kprintf("........admit all reader which have priority more than writer or in case of equal priority have waiting time more than writer\n........");
                  int tmp;
                  newpid = q[lptr->lrqtail].qprev;
                  do {
                        tmp = q[newpid].qprev;
                        kprintf("\n........Reader being evaluted %d, proctab[newpid].plocks[lck].priority = %d, writer_priority = %d........\n", newpid, proctab[newpid].plocks[lck].priority, proctab[next_writer_pid].plocks[lck].priority);
                        if ((proctab[newpid].plocks[lck].priority > proctab[next_writer_pid].plocks[lck].priority) ||
                              ((proctab[newpid].plocks[lck].priority == proctab[next_writer_pid].plocks[lck].priority) &&
                                    (proctab[newpid].plocks[lck].blocking_time < writer_blocking_time)))
                        {
                              release_highest_lock_priority_process(lck, newpid);
                              lptr->lcnt++;
                        }
                        newpid = tmp;
                  } while (newpid != lptr->lrqhead);
                  lptr->lacq = ACQUIRED_BY_READER;
            }
            else if (proctab[next_writer_pid].plocks[lck].priority == proctab[next_reader_pid].plocks[lck].priority)
            {
                  kprintf("........Next Reader and Next Writer both have equal priority........");

                  if (proctab[next_writer_pid].plocks[lck].blocking_time < proctab[next_reader_pid].plocks[lck].blocking_time)
                  {
                        release_highest_lock_priority_process(lck, next_writer_pid);
                        lptr->lacq = ACQUIRED_BY_WRITER;
                  }
                  else
                  {

                        unsigned long writer_blocking_time = proctab[next_writer_pid].plocks[lck].blocking_time + 1000;
                       	//releasing out all the readers which satisfy certain conditions
                        int tmp;
                        newpid = q[lptr->lrqtail].qprev;
                        do {
                              tmp = q[newpid].qprev;
                              kprintf("\n........Reader being evaluted = %d Reader blocking time = %d Reader Priority = %d Writer blocking time =%d Writer Priority= %d\n........",
                                    newpid, proctab[newpid].plocks[lck].blocking_time, proctab[newpid].plocks[lck].priority, writer_blocking_time, proctab[next_writer_pid].plocks[lck].priority);
                              if (proctab[next_writer_pid].plocks[lck].priority == proctab[newpid].plocks[lck].priority && proctab[newpid].plocks[lck].blocking_time <= writer_blocking_time)
                              {
                                   	//kprintf("\n Vandy reades being released = %d\n", newpid);
                                    release_highest_lock_priority_process(lck, newpid);
                                    lptr->lcnt++;
                              }
                              newpid = tmp;
                        } while (newpid != lptr->lrqhead);
                        lptr->lacq = ACQUIRED_BY_READER;
                  }
            }
      }
}
SYSCALL releaseall(int numlocks, int args)
{
      STATWORD ps;
      register struct lentry * lptr;
      int i, lck, pid;
      int *lckptr = (int*)(&args) + (numlocks - 1);
      disable(ps);

     	//Intialize lock ptr
      while (numlocks > 0)
      {

            int *lckptr = (int*)(&args) + (numlocks - 1);

            lck = *lckptr;
            lptr = &locks[*lckptr];

           	//Check for bad lock

            if (isbadlock(lck) || lptr->lstate == LFREE || proctab[currpid].plocks[lck].status != L_CREATED)
            {
                  return SYSERR;
            }

            switch (lptr->lacq)
            {

                  case ACQUIRED_BY_READER:

                        kprintf("\n........Lock being released by Reader. Reader currpid = %d........\n", currpid);
                        if (lptr->lcnt > 1)
                        {
                             	// Decrement reader count and update process control block
                              lptr->lcnt--;
                              proctab[currpid].plocks[lck].status = L_DELETED;
                              break;
                        }

                        if (lptr->lcnt == 1 && is_writer_queue_empty(lptr) && is_reader_queue_empty(lptr))
                        {
                              kprintf("\n........Last Reader being released........\n");
                              lptr->lcnt = 0;
                              proctab[currpid].plocks[lck].status = L_DELETED;
                              lptr->lacq = NOT_ACQUIRED;
                              break;
                        }

                        execute_defaultSteps(lck, lptr);
                        break;

                  case ACQUIRED_BY_WRITER:

                        kprintf("\n........Lock being released by Writer. Writer currpid = %d........\n", currpid);
                       	//Last Writer
                        if (is_writer_queue_empty(lptr) && is_reader_queue_empty(lptr))
                        {
                              kprintf("\n........Last Writer being released........\n");
                              lptr->lcnt = 0;
                              proctab[currpid].plocks[lck].status = L_DELETED;
                              lptr->lacq = NOT_ACQUIRED;
                              break;
                        }

                        execute_defaultSteps(lck, lptr);
                        break;
            }
            --numlocks;
            --lckptr;
      }
      return OK;
}