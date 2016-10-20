/* $Id: memory.c,v 1.2 1997/06/13 06:04:39 deberg Exp $ */

/* memory.c - memory subsystem for the pendulum2 vm */

#include <stdlib.h>
#include <stdio.h>
#include "pendvm.h"
#include "memory.h"

extern int debug;
extern char *progname;

static MEMORY *the_mem=NULL; /* at first there's nothing in memory */

/* returns a handle into memory for the given address */
MEMORY *
mem_get(WORD address)
{
  MEMORY *p=the_mem;
  MEMORY *q;
      
  while(p) {
    if(p->address==address)
      return p;
    else
      p=p->next;
  }

  /* we've reached the end of memory and nothing is there.  create a
     new cell on the front and return it */

  q=malloc(sizeof(MEMORY));
  q->address=address;
  q->label[0]=0;
  q->type=MEM_EMPTY;
  q->breakpoint=0;
  q->value=0;
  q->next=the_mem;

  the_mem=q; /* link the node in */

  return q;
}

