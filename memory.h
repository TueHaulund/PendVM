/* $Id: memory.h,v 1.1.1.1 1997/06/13 05:08:50 deberg Exp $ */

/* memory.h - defines for the memory subsystem */

/* prevent multiple inclusions */
#ifndef PENDSIM_MEMORY
#define PENDSIM_MEMORY

#define MEM_EMPTY 0
#define MEM_DATA 1
#define MEM_INST 2

#define WORD unsigned int

/* linked list structure - one for each used location in memory */
/* address and value _must_ be 32 bits in order for this to work... */
struct MEM_T {
  WORD address;
  char label[256];
  char type; /* 0 for unused, 1 for raw data, 2 for instruction */
  char breakpoint; /* 0 for no, 1 for yes */
  WORD value; /* for data */
  char inst[16]; /* for instruction */
  char args[3][16]; /* for instruction */
  struct MEM_T *next;
};

typedef struct MEM_T MEMORY;

MEMORY *mem_get(WORD);

#endif /* ifndef PENDSIM_MEMORY */
