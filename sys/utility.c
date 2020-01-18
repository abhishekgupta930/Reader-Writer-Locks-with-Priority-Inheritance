#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>


int  is_reader_queue_empty(struct lentry* lptr)
 {
	int node;
	node = q[lptr->lrqtail].qprev;
	if (node==lptr->lrqhead)
	   return TRUE;
	else 
	   return FALSE;

}

int is_writer_queue_empty(struct lentry* lptr)
{
    int node;    
    node = q[lptr->lwqtail].qprev;
        if (node==lptr->lwqhead)
           return TRUE;
        else
           return FALSE;

}
int get_next_writer(int lck, int lwhead, int lwtail)
{
  int pid;
  pid = q[lwtail].qprev;
  while (pid!=lwhead)
	{
 	  return pid;	
        }
  return SYSERR;

}
int get_next_reader(int lck, int lrhead, int lrtail)
{

  int pid;
  pid = q[lrtail].qprev;
  while (pid!=lrhead)
        {
          return pid;
        }
  return SYSERR;

}
