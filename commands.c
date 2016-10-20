/* $Id: commands.c,v 1.2 1997/06/13 06:04:19 deberg Exp $ */

/* commands.c - implements user interface to pendvm */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "pendvm.h"

extern int debug;
extern char *progname;
extern MACHINE *m;

COMMAND commands[]={
  /*  {"assemble", com_assemble, "assemble <address> \"<inst>\" - assemble into imem"}, */
  {"break", com_break, "break [address] - set breakpoint"},
  {"clear", com_clear, "clear <address> - clear memory location"},
  {"continue", com_continue, "continue - continue execution"},
  {"dir", com_dir, "dir [D] - set processor direction"},
  {"read", com_read, "read <address> [n] - read memory location(s)"},
  {"reg", com_reg, "reg - display registers"},
  {"run", com_run, "run - begin execution"},
  {"set", com_set, "set <register> <value> - set value of register"},
  {"state", com_state, "state - shows the state of the processor"},
  {"step", com_step, "step [n] - step 1 (or n) instructions"},
  {"unbreak", com_unbreak, "unbreak [address] - clear breakpoint"},
  /*  {"write", com_write, "write <i/d> <address> <value> - write memory"}, */
  {"help", com_help, ""},
  {"quit", com_quit, "quit - exit pendvm"},
  {NULL, NULL, NULL} /* end of command list */
};

int com_quit(char args[][64], int num_args)
{
  char buffer[10];

  printf("Exit Pendulum VM? (y/n) ");
  fgets(buffer,10,stdin);
  if(buffer[0]=='y' || buffer[0]=='Y') exit(0); /* get out of here */

  return 0;
} /* com_quit */

int com_help(char args[][64], int num_args)
{
  int i=0;

  while(commands[i].name) {
    printf("%s\n", commands[i].help);
    i++;
  }

  return 0;
}

int com_break(char args[][64], int num_args)
{
  MEMORY *mem;

  WORD address;

  if(num_args==0) /* break on current line */
    address=m->PC;
  else /* break on specified line */
    address=strtol(args[0], (char **)NULL, 0); /* get first argument */

  mem=mem_get(address);
  mem->breakpoint=1;
  printf("Breakpoint set at address 0x%.8X\n",address);

  return 0;
}

int com_clear(char args[][64], int num_args)
{
  WORD address;
  MEMORY *mem;

  if(num_args<1) { /* must have address */
    printf("Must specify address.\n");
    return -1;
  }
  address=strtol(args[0], (char **)NULL, 0);
  mem=mem_get(address);
  mem->type=0;
  mem->label[0]=0;
  mem->inst[0]=0;
  mem->args[0][0]=0;
  mem->args[1][0]=0;
  mem->args[1][0]=0;

  return 0;
}

int com_continue(char args[][64], int num_args)
{
  /*if(m->reset) {
    printf("Program not currently running.\n");
    return 1;
  } -removed by mpf */
  step_processor(-1);
  printf("Processor halted.\n");
  display_state();
  
  return 0;
}

BOOL oppositedir(BOOL dir) {
  return (dir == FORWARD)?REVERSE:FORWARD;
}

int com_dir(char args[][64], int num_args) 
{
  if(num_args>0) /* direction is indicated */
    switch(args[0][0]) {
    case 'f':
    case 'F':
      if( m->dir==REVERSE ) {
	m->externaldir = oppositedir(m->externaldir);
	m->dir=FORWARD;
	m->BR=-m->BR; /* in case we are in the middle of a branch -mpf */ 
	adjust_pc(); /* don't know about this call now */ /*it's OK -mpf */
      }
      break;
    case 'r':
    case 'R':
      if( m->dir==FORWARD ) {
	m->externaldir = oppositedir(m->externaldir);
	m->dir=REVERSE;
	m->BR=-m->BR;
	adjust_pc(); /* don't know about this call now */
      }
      break;
    default:
      printf("Illegal direction. Must be FORWARD or REVERSE.\n");
      return -1; /* go back to loop */
    } /* switch */
  else { /* nothing specified - we toggle direction */
    m->dir = oppositedir(m->dir);
    m->externaldir = oppositedir(m->externaldir);
    m->BR=-m->BR;
    adjust_pc(); /* don't know about this call now */
  }
  display_state();

  return 0;
}

int com_read(char args[][64], int num_args)
{
  int address, iter, i;
  char *source_line;

  if(num_args<1) { /* must have address */
    printf("Must specify address.\n");
    return -1;
  }
  if(num_args==2) /* we are given a count */
    iter=strtol(args[1],(char **)NULL, 0);
  else /* default is 16 */
    iter=16;
      
  address=strtol(args[0], (char **)NULL, 0);

  for(i=address; i<address+iter; i++){
    MEMORY *mem = mem_get(i);
    WORD word = mem->value;
    printf("%08x ",word);
    if ((i+1)%8 == 0) {printf("\n");}
  }

  return 0;
}

int com_reg(char args[][64], int num_args)
{
  int i=0,j=0;

  while(i<MAX_REG) {
    printf("$%.2d=%.8X\t",i,m->reg[i]);
    i++; j++;
    if(j==4) {
      j=0;
      printf("\n");
    }
  }

  return 0;
}

int com_run(char args[][64], int num_args)
{
  char buffer[10];

  if(!m->reset) { /* verify that we want to restart program */
    printf("The program being simulated has been started already.\n");
    printf("Start it from the beginning? (y or n) ");
    fgets(buffer,10,stdin);
    if(buffer[0]=='y' || buffer[0]=='Y') { /* go ahead and run */
      m->PC=0; /* reset PC */
      m->BR=0; /* reset BR */
      m->dir=FORWARD; m->externaldir=FORWARD;
      m->time = 0;
      step_processor(-1);
      printf("Processor halted.\n");
      display_state();

      return 0;
    }
    return 1; /* aborted */
  }
}

int com_set(char args[][64], int num_args)
{
  if(num_args<2) {
    printf("Must specify <register> and <value>.\n");
    return -1;
  } 

  if(!strcasecmp(args[0],"pc")) { /* set PC */
    m->PC=strtol(args[1], NULL, 0);

  } else if(!strcasecmp(args[0],"br")) { /* set BR */
    m->BR=strtol(args[1], NULL, 0);

  } else if(args[0][0]=='$') { /* set register */
    m->reg[strtol(args[0]+1, NULL, 0)]=strtol(args[1], NULL, 0);

  } else printf("Must set PC or register.\n"); /* bad register */

  return 0;
}
  
int com_state(char args[][64], int num_args)
{
  display_state();

  return 0;
}

int com_step(char args[][64], int num_args)
{
  if(num_args>0) /* see if a number of steps was specified */
    step_processor(strtol(args[0],(char **)NULL,0));
  else
    step_processor(1);
  display_state();

  return 0;
}

int com_unbreak(char args[][64], int num_args)
{
  MEMORY *mem;

  WORD address;

  if(num_args==0) /* break on current line */
    address=m->PC;
  else /* break on specified line */
    address=strtol(args[0], (char **)NULL, 0); /* get first argument */

  mem=mem_get(address);
  if( mem->breakpoint ) {
    mem->breakpoint=0;
    printf("Breakpoint cleard at address 0x%.8X\n",address);
  } else {
    printf("No breakpoint currently set at address 0x%.8X\n",address);
  }

  return 0;
}

int com_write(char args[][64], int num_args)
{
  int address, value;

  if(num_args<2) { /* must have address, value */
    printf("Must specify address and value.\n");
    return -1;
  }

  address=strtol(args[0], NULL, 0);
  value=strtol(args[1], NULL, 0);
  printf("Thanks, but writes aren't implemented right now.\n");

  return 0;
}
