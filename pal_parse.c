/* * Last edited: Jul  3 16:42 1997 (deberg) */
/* $Id: pal_parse.c,v 1.4 1997/07/03 19:24:00 deberg Exp $ */

/* pal_parse.c - set of routines which parse PAL instructions into
   proto-code.  designed to be useful for pendasm as well */

/* parse_inst consults the table inst_fmt and determines if there is
  an error.  if not it parses requests for registers, literals, and
  labels, and then calls the dispatch function on them */

#include <strings.h>

#if defined (_WIN32) || defined (_WIN64)
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
#endif

#include <stdio.h>
#include <stdlib.h>
#include "pendvm.h"

struct LT *lt=0; /* exported label table */
extern MACHINE *m; /* machine from machine.c */

struct {
  char *inst;
  int args[3];
  int (*func)(WORD, WORD, WORD);
} instructions[]={
  /* name, {arg1, arg2, arg3}, handler */
  {"ADD", {REG, REG, NIL}, i_add},
  {"ADDI", {REG, IMM, NIL}, i_addi},
  {"ANDX", {REG, REG, REG}, i_andx},
  {"ANDIX", {REG, REG, IMM}, i_andix},
  {"BEQ", {REG, REG, OFF}, i_beq},
  {"BGEZ", {REG, OFF, NIL}, i_bgez},
  {"BGTZ", {REG, OFF, NIL}, i_bgtz},
  {"BLEZ", {REG, OFF, NIL}, i_blez},
  {"BLTZ", {REG, OFF, NIL}, i_bltz},
  {"BNE", {REG, REG, OFF}, i_bne},
  {"BRA", {LOFF, NIL, NIL}, i_bra},
  {"EXCH", {REG, REG, NIL}, i_exch},
  {"NORX", {REG, REG, REG}, i_norx},
  {"NEG", {REG, NIL, NIL}, i_neg},
  {"ORX", {REG, REG, REG}, i_orx},
  {"ORIX", {REG, REG, REG}, i_orix},
  {"RL", {REG, AMT, NIL}, i_rl},
  {"RLV", {REG, REG, NIL}, i_rlv},
  {"RR", {REG, AMT, NIL}, i_rr},
  {"RRV", {REG, REG, NIL}, i_rrv},
  {"SLLX", {REG, REG, AMT}, i_sllx},
  {"SLLVX", {REG, REG, REG}, i_sllvx},
  {"SLTX", {REG, REG, REG}, i_sltx},
  {"SLTIX", {REG, REG, IMM}, i_sltix},
  {"SRAX", {REG, REG, AMT}, i_srax},
  {"SRAVX", {REG, REG, REG}, i_sravx},
  {"SRLX", {REG, REG, AMT}, i_srlx},
  {"SRLVX", {REG, REG, REG}, i_srlvx},
  {"SUB", {REG, REG, NIL}, i_sub},
  {"XOR", {REG, REG, NIL}, i_xorx},
  {"XORI", {REG, IMM, NIL}, i_xorix},
  {"SWAPBR", {REG, NIL, NIL}, i_swapbr},
  {"RBRA", {LOFF, NIL, NIL}, i_rbra},
  {"OUT", {REG, NIL, NIL}, i_show},
  {"OUTPUT", {REG, NIL, NIL}, i_show},
  {"SHOW", {REG, NIL, NIL}, i_show},
  {"EMIT", {REG, NIL, NIL}, i_emit},
  {"START", {NIL, NIL, NIL}, i_start},
  {"FINISH", {NIL, NIL, NIL}, i_finish},
  /* fencepost */
  {NULL, {NIL, NIL, NIL}, NULL}};

int
parse_inst(char *label, char *inst, char args[][16])
{
  int i=0;
  int j;
  WORD a[3];
  int len;

  /* find the instruction */
  while( instructions[i].inst ) {
    if( !strcasecmp(instructions[i].inst,inst) ) break;
    i++;
  }

  /* check if we had a match */
  if( !instructions[i].inst ) {
    /* didn't find the instruction */
    pendvm_error("undefined instruction");
    return -1;
  }

  /* we have a match. process each of the 3 args */
  for(j=0;j<3;j++) {
    switch(instructions[i].args[j]) {
    case NIL:
      a[j]=0;
      break;
    case REG: {
      int reg=parse_reg(args[j]);
      if(reg==-1) {
	pendvm_error("expected register");
	return -1;
      } else if(reg==-2) {
	pendvm_error("register out of range");
	return -1;
      } else {
	a[j]=reg;
      }
      break;
    }
    case AMT:
    case IMM: {
      int *immed;

      if( instructions[i].args[j]==AMT )
	len=16;
      else
	len=5; /* ??? */

      immed=parse_immed(args[j],len);

      if( !immed ) {
	pendvm_error("bad immediate/amt");
	return -1;
      } else {
	a[j]=*immed;
      }
      break;
    }
    case OFF:
    case LOFF: {
      int *absolute;

      if( instructions[i].args[j]==OFF )
	len=16;
      else
	len=26; /* ??? */

      absolute=parse_immed(args[j],len);

      if( !absolute ) {
	pendvm_error("bad offset");
	return -1;
      } else {
	a[j]=*absolute - m->PC;
      }
      break;
    }
    } /* switch */
  } /* for (step through 3 args) */

  /* now do the right thing */
  return (*instructions[i].func)(a[0],a[1],a[2]);

}

/* returns 0 through 31 for valid register, returns -1 for bad format,
   returns -2 for out-of-range */
/* registers must be in the form $n or $nn, where n/nn is a decimal
   number in the range 0 - 31. */
int
parse_reg(char *reg)
{
  int r;
  
  if( reg[0]!='$' ) return -1; /* bad format */
  
  r=atoi(reg+1);
  if( r>=0 && r<=31 )
    return r; /* return value */
  else
    return -2; /* out of range */
}

/* parse label checks the label table and returns a pointer to the
   label's address. returns NULL if label not found */
/* supports labels of the form -LBL */
WORD *
parse_label(char *label)
{
  struct LT *p=lt;
  static WORD neg_lbl;

  while(p) {
    if( !strcmp(p->label,label) ) {
      return &(p->address);
    } else if( label[0]=='-' && !strcmp(p->label,label+1) ) {
      /* deal with negative labels.  this is strange but works as long
	 as we're not multithreaded :) */
      neg_lbl = -(p->address);
      return &neg_lbl; 
    } else {
      p=p->next;
    }
  }

  return NULL;
}

/* parse immediate returns a pointer to the immediate value or NULL if
   the immediate was invalid */
/* should check range on field size using len */
WORD *
parse_immed(char *immed, int len)
{
  static WORD value;
  char **endptr=NULL;
  WORD *temp;

  if( !immed ) return NULL; /* empty string */
  if( !strlen(immed) ) return NULL; /* also empty */

  /* check to see if label */
  if( temp=parse_label(immed) ) {
    value=*temp;
  } else {
    /* not label */
    value=strtol(immed, endptr, 0); /* machine dependent - this sucks */
    if(endptr) return NULL; /* bad format in immediate */
  }

  /* now value is correct, check for range */

  return &value;
}
