/* $Id: pal_parse.h,v 1.2 1997/06/15 05:32:53 deberg Exp $ */

/* pal_parse.h - defines label table structure */

struct LT {
  char label[256];
  unsigned int address;
  struct LT *next;
};

#define NIL 0
#define REG 1
#define IMM 2
#define AMT 4
#define OFF 8
#define LOFF 16

int parse_reg(char *);
WORD *parse_label(char *);
WORD *parse_immed(char *, int);
