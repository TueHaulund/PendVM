/* * Last edited: Jul  3 16:42 1997 (deberg) */
/* $Id: machine.c,v 1.3 2000/04/20 15:56:10 mpf Exp mpf $ */

/* machine.c - code to initialize the machine and execute instructions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include "memory.h"
#include "pendvm.h"
#include "opcodes.h"

extern int debug;
extern char *progname;
extern COMMAND commands[];
extern int output_radix;

/* Determines how to display next output word.
   Uses output type specifiers in pendvm.h */
int output_type = PTYPE_NONE;


extern struct LT *lt; /* the label table */
MACHINE *m; /* our machine */

void strip_comments(char buffer[]);

/* init_machine - initializes a machine, setting the correct flags.
   this is a generic machine, the run-time SP and PC aren't set until
   later */
void
init_machine(void)
{
  int i;

  m=malloc(sizeof(MACHINE));

  /* initialize values */
  m->PC=0;
  m->BR=0;
  m->dir=FORWARD; m->externaldir=FORWARD;
  m->reset=TRUE;
  m->time=0;			/* Time step #0 */
  for(i=0;i<MAX_REG;i++) m->reg[i]=0; /* clear the registers */
}

/* load_imem - loads the input file into core. opens the file and
   loads the instructions into memory beginning at offset 0. if the
   input file does not begin with the PAL header, then load_imem
   aborts and exits pendvm. */
void load_imem(char *input)
{
  FILE *file;
  char buffer[256];
  char tmp[5][16];
  WORD address=-1;
  int line=1; /* start w/ line 2 of the file */
  int fields;
  int inst_offset;
  MEMORY *mem;
  int error_flag=0;
  struct LT *lt_new;
  char *start_point=NULL;

  /* load in the instruction memory */
  file=fopen(input, "r");
  if(!file) {
    printf("%s: Unable to open input file %s.\n",progname,input);
    exit(1);
  }
  
  /* make sure file is in valid pendulum format */
  fgets(buffer,256,file); /* get first line */
  if( strncmp(buffer, ";; pendulum pal file", 20) ) { /* compare with known header */
    printf("Input file not in in Pendulum pal format.\n");
    exit(1);
  }
  
  /* loop to load instruction memory */
  while(fgets(buffer,256,file)) {
    line++;
    strip_comments(buffer);
    fields=sscanf(buffer,"%s%s%s%s%s",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4]);
    
    if( fields==0 || fields==EOF ) continue;

    /* check for directives */
    if( !strcasecmp(tmp[0],".start") ) {
      /* save the start location */
      start_point=strdup(tmp[1]);
      continue;
    }
    
    address++; /* something is on this line */
    
    /* check if label */
    if( tmp[0][strlen(tmp[0])-1]==':' ) {
      /* we have a label */
      mem=mem_get(address);
      strncpy(mem->label, tmp[0], strlen(tmp[0])-1);
      mem->label[strlen(tmp[0])-1]=0;

      /* stuff the label table */
      lt_new=malloc(sizeof(struct LT));
      strcpy(lt_new->label, mem->label);
      lt_new->address=address;
      lt_new->next=lt;
      lt=lt_new;

      inst_offset=1; /* increase the offset into the other args */
      
      if( fields==1 ) {
	/* the label was by itself. hack to make address ok and get next line */
	address--;
	continue;
      }
    } else {
      /* no label */
      inst_offset=0;
    }
    
    /* we def have an instruction at this point */
    mem=mem_get(address);
    
    /* check for dataword (dw) */
    if( !strcasecmp(tmp[inst_offset],"data") ) {
      /* stuff the value into memory */
      mem->type=MEM_DATA;
      if( fields-inst_offset!=2 ) {
	load_err(line, "poorly formatted dw declaration");
	error_flag++;
	continue;
      }
      mem->value=strtol(tmp[inst_offset+1], NULL, 0);
      continue;
    } else {
      /* regular instruction */
      int i;
      
      mem->type=MEM_INST;
      strcpy(mem->inst, tmp[inst_offset]);
      /* fill the args */
      for( i=1+inst_offset; i<fields; i++ )
	strcpy(mem->args[i-1-inst_offset],tmp[i]);
      /* zero out the rest of the args */
      for( ; i<3; i++ )
	mem->args[i-1-inst_offset][0]=0;
      continue;
    }
    /* UNREACHABLE */
  }
  
  /* done loading file. abort if errors */
  fclose(file);
  if( error_flag ) {
    printf("Unable to load file, there were %d errors.\n", error_flag);
    exit(1);
  }

  /* check to see if we had a saved PC */
  if( start_point ) {
    WORD *result;
    result = parse_immed(start_point,32);
    if( !result ) {
      printf("Error in .start directive in file.\n");
      exit(1);
    }
    m->PC=*result;
  }
}      

int
load_err(int line, char *message)
{
  printf("ERROR in line %d: %s\n", line, message);
}

void
strip_comments(char buffer[])
{
  int i=0;

  while( buffer[i] ) {
    if( buffer[i]==';' ) {
      buffer[i]=0; /* terminate line */
      return;
    }
    i++;
  }
}

/* loop: this is the main user interface loop. it displays the prompt,
   reads the command, and dispatches it... */
void loop()
{
  char buffer[64]; /* input */
  char oldbuffer[64]; /* previous input */
  char command[64]; /* command name */
  char param[5][64]; /* command params */
  int num_args; /* number of params */
  int i;
  int (*func)(char param[][64], int); /* dispatch function */

  while(1) { /* main loop */
    printf("(pendvm) "); /* display prompt */
    fgets(buffer,64,stdin);
    if(!strcmp(buffer,"\n")) /* if user hit enter */
      strcpy(buffer, oldbuffer); /* repeat last command */
    else 
      strcpy(oldbuffer, buffer); /* save command */

    /* parse command, set num_args to number of additional arguments */
    num_args=sscanf(buffer,"%s%s%s%s%s%s",command,param[0],param[1],param[2],
		    param[3], param[4])-1;

    /* find command */
    func=NULL;
    i=0;
    while(commands[i].name) { /* stop when you get to the end of the list */
      if( strcasecmp(commands[i].name, command) )
	i++; /* nope, get next name */
      else {
	func=commands[i].func; /* set the correct func */
	break; /* get out of loop */
      }
    } /* if none of the commands matched, then func==NULL */

    /* call command */
    if(func) 
      (*func)(param, num_args);
    else
      printf("Invalid command. Type \"help\" for help.\n"); /* bad command */

  } /* while(1) */
} /* loop */

void
pendvm_error(char *message)
{
  printf("ERROR at address %04X: %s\n", m->PC, message);
}

/* display_state - displays the state of the processor, including PC,
   halt, DIR, imem(PC), the description associated with imem(PC)
   (usually the source code line), and if there is a breakpoint set at
   that location */
void display_state()
{
  MEMORY *mem=mem_get(m->PC);
  int i=0;

  com_reg(0,0);

  printf("\nSTEP#: %d\tPC: %.8X\tBR: %.8X\tDIR: %s\n", m->time, m->PC, m->BR,
	 m->dir==FORWARD ? "FORWARD" : "REVERSE");

  /* MEM(PC) better be an instruction or we're in trouble */
  printf("MEM(PC): %s", mem->inst); /* print out current inst */
  while(mem->args[i][0]) printf(" %s", mem->args[i++]);

  /* if there was nothing there, indicate as such */
  if (mem->type != MEM_INST) {
    printf("--NO INSTRUCTION--   ");
  }
  
  if( mem->breakpoint ) printf("\t*BREAK*");
  printf("\n");
}

/* step_processor - steps the processor interations number of
   times. halts on breakpoint. if called w/ iterations<0 then it goes
   until end of program or breakpoint.  it will ignore a breakpoint on
   the initial PC location (standard behavior of course) */
int step_processor(int iterations)
{
  int result;
  int loop=1;
  MEMORY *mem;
  
  if( iterations<0 ) {
    loop=0; /* don't check iteration count */
    iterations=1; /* force loop to take place */
  }
  m->reset = FALSE;
  while(iterations>0) {
    result=execute_instruction(); /* also updates PC */
    mem=mem_get(m->PC);

    if(loop) iterations--;

    if( result == EXEC_FINISH ) { /* hit start/finish instruction */
      return EXEC_FINISH;
    }

    if( mem->breakpoint ) { /* breakpoint */
      return EXEC_BREAK;
    }

    if( result != EXEC_NORMAL ) { /* unspecified error */
      return EXEC_ERROR;
    }
  }
  return EXEC_NORMAL;
}

/* adjust_pc - increments or decrements the PC by 1, depending on the
   direction of the processor. returns final value of PC. */
int adjust_pc()
{
  if( !m->BR ) {
    /* normal mode */
    if(m->dir==FORWARD) m->PC++; /* forward direction, inc pc */
    else m->PC--; /* reverse direction, dec pc */

  } else {
    /* branching - add BR into PC */
    m->PC += m->BR;
  }
  /* Keep track of program time */
  if ( m->externaldir == FORWARD )
    m->time++;
  else
    m->time--;
}

/* execute_instruction - executes instruction on the virtual prp. this
   simulates all aspects of the process, including instruction fetch
   and decode, pc increment/decrement, execute, and writeback. it
   simulates all of the destructive reads and so on. this is the meat
   of the program. */
int execute_instruction()
{
  int status;
  MEMORY *mem;

  /* fetch instruction */
  mem=mem_get(m->PC);

  /* check to make sure it's not data */
  if( mem->type!=MEM_INST ) {
    pendvm_error("no instruction");
    return EXEC_INVALID_INST;
  }

  /* execute it */
  status=parse_inst(mem->label, mem->inst, mem->args);
  
  if( status == 0 ) {
    /* successful execution */
    adjust_pc();
    return EXEC_NORMAL;

  } else if( status == -1 ) {
    /* generic parse error */
    return EXEC_INVALID_INST;

  } else if( status == -2 ) {
    /* problem w/ contents of register */
    return EXEC_REV_BROKEN;
  } else if( status == -3 ) {
    /* normal start/finish */
    return EXEC_FINISH;
  } else if( status == -4) {
    /* "exchange with instruction" error */
    return EXEC_ERROR;
  }
}

int
i_add(WORD rsd, WORD rt, WORD u1)
{
  m->reg[rsd] += (m->dir)*(m->reg[rt]);
  return 0;
}

int
i_addi(WORD rsd, WORD imm, WORD u1)
{
  m->reg[rsd] += (m->dir)*imm;
  return 0;
}

int
i_andx(WORD rd, WORD rs, WORD rt)
{
  m->reg[rd] ^= (m->reg[rs] & m->reg[rt]);
  return 0;
}

int
i_andix(WORD rd, WORD rs, WORD imm)
{
  m->reg[rd] ^= (m->reg[rs] & imm);
  return 0;
}

int
i_beq(WORD ra, WORD rb, WORD off)
{
  if( m->reg[ra] == m->reg[rb] ) m->BR += off;
  return 0;
}

int
i_bgez(WORD rb, WORD off, WORD u1)
{
  if( m->reg[rb] >= 0 ) m->BR += off;
  return 0;
}

int
i_bgtz(WORD rb, WORD off, WORD u1)
{
  if( m->reg[rb] > 0 ) m->BR += off;
  return 0;
}

int
i_blez(WORD rb, WORD off, WORD u1)
{
  if( m->reg[rb] <= 0 ) m->BR += off;
  return 0;
}

int
i_bltz(WORD rb, WORD off, WORD u1)
{
  if( m->reg[rb] < 0 ) m->BR += off;
  return 0;
}

int
i_bne(WORD ra, WORD rb, WORD off)
{
  if( m->reg[ra] != m->reg[rb] ) m->BR += off;
  return 0;
}

int
i_bra(WORD loff, WORD u1, WORD u2)
{
  m->BR += loff;
  return 0;
}

int
i_exch(WORD rd, WORD ra, WORD u1)
{
  WORD tmp;
  MEMORY *loc = mem_get(m->reg[ra]);

  if (loc->type == MEM_INST) {
    pendvm_error("exch with instruction locations not yet supported");
    return -4; /* "exchange with instruction" error */
  }

  tmp=m->reg[rd];
  m->reg[rd]=loc->value;
  loc->value=tmp;
  return 0;
}

int
i_norx(WORD rd, WORD rs, WORD rt)
{
  m->reg[rd] ^= ~(m->reg[rs] | m->reg[rt]);
  return 0;
}

int
i_neg(WORD rsd, WORD u1, WORD u2)
{
  m->reg[rsd] = -(m->reg[rsd]);
  return 0;
}

int
i_orx(WORD rd, WORD rs, WORD rt)
{
  m->reg[rd] ^= (m->reg[rs] | m->reg[rt]);
  return 0;
}

int
i_orix(WORD rd, WORD rs, WORD imm)
{
  m->reg[rd] ^= (m->reg[rs] | imm);
  return 0;
}

int
i_rl(WORD rsd, WORD amt, WORD u1)
{
  if( m->dir==FORWARD ) {
    if( amt != 0 ) 
      m->reg[rsd] = (m->reg[rsd] << amt) | EXTRACT(m->reg[rsd],32-amt,31);

  } else {
    if( amt != 0 )
      m->reg[rsd] = (((unsigned)m->reg[rsd]) >> amt) | (EXTRACT(m->reg[rsd],0,amt-1) << (32-amt) );
  }
  return 0;
}

int
i_rlv(WORD rsd, WORD rt, WORD u1)
{
  if( m->dir==FORWARD ) {
    if( m->reg[rt] != 0 ) 
      m->reg[rsd] = (m->reg[rsd] << m->reg[rt]) | EXTRACT(m->reg[rsd],32-m->reg[rt],31);

  } else {
    if( m->reg[rt] != 0 )
      m->reg[rsd] = (((unsigned)m->reg[rsd]) >> m->reg[rt]) | (EXTRACT(m->reg[rsd],0,m->reg[rt]-1) << (32-m->reg[rt])  );
  }
  return 0;
}

int
i_rr(WORD rsd, WORD amt, WORD u1)
{
  if( m->dir==FORWARD ) {
    if( amt != 0 )
      m->reg[rsd] = (((unsigned)m->reg[rsd]) >> amt) | (EXTRACT(m->reg[rsd],0,amt-1) << (32-amt) );

  } else {
    if( amt != 0 ) 
      m->reg[rsd] = (m->reg[rsd] << amt) | EXTRACT(m->reg[rsd],32-amt,31);
  }
  return 0;
}

int
i_rrv(WORD rsd, WORD rt, WORD u1)
{
  if( m->dir==FORWARD ) {
    if( m->reg[rt] != 0 )
      m->reg[rsd] = (((unsigned)m->reg[rsd]) >> m->reg[rt]) | (EXTRACT(m->reg[rsd],0,m->reg[rt]-1) << (32-m->reg[rt]) );

  } else {
    if( m->reg[rt] != 0 ) 
      m->reg[rsd] = (m->reg[rsd] << m->reg[rt]) | EXTRACT(m->reg[rsd],32-m->reg[rt],31);
  }
  return 0;
}

int
i_sllx(WORD rd, WORD rs, WORD amt)
{
  m->reg[rd] ^= (m->reg[rs] << amt);
  return 0;
}

int
i_sllvx(WORD rd, WORD rs, WORD rt)
{
  m->reg[rd] ^= (m->reg[rs] << m->reg[rt]);
  return 0;
}

int
i_sltx(WORD rd, WORD rs, WORD rt)
{
  if(m->reg[rs] < m->reg[rt]) m->reg[rd] ^= 1;
  return 0;
}

int
i_sltix(WORD rd, WORD rs, WORD imm)
{
  if(m->reg[rs] < imm) m->reg[rd] ^= 1;
  return 0;
}

int
i_srax(WORD rd, WORD rs, WORD amt)
{
  /* the absurdity of c forces me to do this */
  WORD i=0;
  WORD tmp;

  tmp = m->reg[rs] >> amt;
  if( EXTRACT(m->reg[rs],31,31) ) i=( power(2,amt)-1 ) << (32-amt);
  tmp |= i;

  m->reg[rd] ^= tmp;
  return 0;
}

int
i_sravx(WORD rd, WORD rs, WORD rt)
{
  /* the absurdity of c forces me to do this */
  WORD i=0;
  WORD tmp;

  tmp = m->reg[rs] >> m->reg[rt];
  if( EXTRACT(m->reg[rs],31,31) ) i=( power(2,m->reg[rt])-1 ) << (32-m->reg[rt]);
  tmp |= i;

  m->reg[rd] ^= tmp;
  return 0;
}

int
i_srlx(WORD rd, WORD rs, WORD amt)
{
  m->reg[rd] ^= (m->reg[rs] >> amt);
  return 0;
}

int
i_srlvx(WORD rd, WORD rs, WORD rt)
{
  m->reg[rd] ^= (m->reg[rs] >> m->reg[rt]);
  return 0;
}

int
i_sub(WORD rsd, WORD rt, WORD u1)
{
  m->reg[rsd] -= (m->dir)*(m->reg[rt]);
  return 0;
}

int
i_xorx(WORD rsd, WORD rt, WORD u1)
{
  m->reg[rsd] ^= m->reg[rt];
  return 0;
}

int
i_xorix(WORD rsd, WORD imm, WORD u1)
{
  m->reg[rsd] ^= imm;
  return 0;
}

int
i_swapbr(WORD r, WORD u1, WORD u2)
{
  WORD tmp;
  int dirsign = (m->dir==FORWARD)?1:-1;

  /* swap a directionless image of BR with a register */
  tmp = m->BR * dirsign;
  m->BR = m->reg[r] * dirsign;
  m->reg[r] = tmp;

  return 0;
}

int
i_show(WORD r, WORD u1, WORD u2)
{
  char str[5];
  int ptype;
  
  if ( output_type == PTYPE_NONE ) {
    ptype = output_type;
  }
  else {
    ptype = output_type & PTYPE_MASK;
  }

  switch(ptype) {

    case PTYPE_NONE:
      if ( m->reg[r] == PTYPE_NEWLINE ) {
        printf("\n");
      }
	  else {
        output_type = m->reg[r];
      }

      break;

    case PTYPE_INT:
      if ( (output_type & INT_BASE_MASK) == INT_BASE_16 ) {
        printf("%x", m->reg[r]);
      }
      else if ( (output_type & INT_SIGN_MASK) == INT_SIGNED ) {
        printf("%d", m->reg[r]);
      }
      else {
        printf("%u", m->reg[r]);
      }

      output_type = PTYPE_NONE;
      break;

    case PTYPE_FLOAT:
      if ( (output_type & FLOAT_NOTATION_MASK) == FLOAT_FIXED ) {
        printf("%f", m->reg[r]);
      }
      else {
        printf("%e", m->reg[r]);
      }

      output_type = PTYPE_NONE;
      break;

    case PTYPE_STRING:
      str[0] = (m->reg[r] & STRING_CHAR0);
      str[1] = (m->reg[r] & STRING_CHAR1)>>8;
      str[2] = (m->reg[r] & STRING_CHAR2)>>16;
      str[3] = (m->reg[r] & STRING_CHAR3)>>24;
      str[4] = '\0';

      printf("%s", str);
 
      if ( ((m->reg[r] & STRING_CHAR3)>>24) == 0 )
	    output_type = PTYPE_NONE;
      break;

	default:
      printf("unknown type: %x\n", m->reg[r]);
      break;
  }


  /* this is a fake instruction to output information to console 
  if( output_radix==16 ) {
    printf("%x\n",m->reg[r]);
  } else {
    printf("%d\n",m->reg[r]);
  }
  */
  return 0;
}

int
i_rbra(WORD loff, WORD u1, WORD u2)
{
  m->BR += loff;
  m->dir = -(m->dir);
  return 0;
}

int
i_emit(WORD r, WORD u1, WORD u2)
{
  /* emit garbage to output device */
  if( output_radix==16 ) {
    printf("%08x\n",m->reg[r]);
  } else {
    printf("%08d\n",m->reg[r]);
  }
  m->reg[r]=0;
  return 0;
}

int
i_start(WORD u1, WORD u2, WORD u3)
{
  return (m->dir == 1)?0:-3;
}

int
i_finish(WORD u1, WORD u2, WORD u3)
{
  return (m->dir == -1)?0:-3;
}

