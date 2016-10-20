/* $Id: pendvm.h,v 1.2 1997/07/03 19:51:03 deberg Exp $ */

/* pendsim.h - include file, defines a bunch of stuff */

/* prevent multiple inclusions */
#ifndef PENDSIM_H
#define PENDSIM_H

#define WORD unsigned int

#include "memory.h"
#include "pal_parse.h"

#ifdef DEBUG 
#define IF_DEBUG(x) if (debug) x;
#else
#define IF_DEBUG(x) ;
#endif

#define VERSION "2.0"
#define PENDASM "/home/joule/deberg/pendulum/pendasm/pendasm"

/* some random types */
#define BOOL char
#define BYTE unsigned char

#define TRUE 1
#define FALSE 0

/* processor directions */
#define FORWARD 1
#define REVERSE -1

/* execute_instruction return codes */
#define EXEC_NORMAL 1
#define EXEC_BREAK 2
#define EXEC_FINISH 3
#define EXEC_ERROR 4
#define EXEC_INVALID_INST 5
#define EXEC_REV_BROKEN 6

/* output type specifiers */
#define PTYPE_MASK    0xFF00

#define PTYPE_NONE    -1

#define PTYPE_INT     0x0000
#define INT_SIGN_MASK   0x01
#define INT_SIGNED      0x00
#define INT_UNSIGNED    0x01
#define INT_BASE_MASK   0x10
#define INT_BASE_10     0x00
#define INT_BASE_16     0x10

#define PTYPE_FLOAT         0x0200
#define FLOAT_NOTATION_MASK   0x01
#define FLOAT_FIXED           0x00
#define FLOAT_EXP             0x01

#define PTYPE_STRING  0x0100
#define STRING_CHAR0  0x000000FF
#define STRING_CHAR1  0x0000FF00
#define STRING_CHAR2  0x00FF0000
#define STRING_CHAR3  0xFF000000

#define PTYPE_NEWLINE 0xFF00


/* command line options */
#define OPTION_DEBUG "--debug"
#define OPTION_HELP "--help"
#define OPTION_VERSION "--version"

/* define some machine constants */
#define MAX_REG 32	/* number of GPR's */

/* snag bits from instruction */
/* #define EXTRACT(num,low,high) (num % power(2,high+1)) / power(2,(low)) */

/* machine structure */
typedef struct {
  unsigned int PC;
  unsigned int BR;
  BOOL dir,externaldir;
  BOOL reset;
  int reg[MAX_REG];
  int time;
} MACHINE;

/* structure for pendsim commands */
typedef struct {
  char *name; /* name that the user types in */
  int (*func)(char args[][64], int); /* function to call */
  char *help; /* short information for help */
} COMMAND;

/* machine.c */
void init_machine(void);
void load_imem(char *);
void loop(void);
void pendsim_error(char *);
void display_state(void);
int step_processor(int);
int adjust_pc(void);
int execute_instruction(void);

/* instruction handlers (in machine.c) */
int i_add(WORD, WORD, WORD);
int i_addi(WORD, WORD, WORD);
int i_andx(WORD, WORD, WORD);
int i_andix(WORD, WORD, WORD);
int i_beq(WORD, WORD, WORD);
int i_bgez(WORD, WORD, WORD);
int i_bgtz(WORD, WORD, WORD);
int i_blez(WORD, WORD, WORD);
int i_bltz(WORD, WORD, WORD);
int i_bne(WORD, WORD, WORD);
int i_bra(WORD, WORD, WORD);
int i_exch(WORD, WORD, WORD);
int i_norx(WORD, WORD, WORD);
int i_neg(WORD, WORD, WORD);
int i_orx(WORD, WORD, WORD);
int i_orix(WORD, WORD, WORD);
int i_rl(WORD, WORD, WORD);
int i_rlv(WORD, WORD, WORD);
int i_rr(WORD, WORD, WORD);
int i_rrv(WORD, WORD, WORD);
int i_sllx(WORD, WORD, WORD);
int i_sllvx(WORD, WORD, WORD);
int i_sltx(WORD, WORD, WORD);
int i_sltix(WORD, WORD, WORD);
int i_srax(WORD, WORD, WORD);
int i_sravx(WORD, WORD, WORD);
int i_srlx(WORD, WORD, WORD);
int i_srlvx(WORD, WORD, WORD);
int i_sub(WORD, WORD, WORD);
int i_xorx(WORD, WORD, WORD);
int i_xorix(WORD, WORD, WORD);
int i_swapbr(WORD, WORD, WORD);
int i_rbra(WORD, WORD, WORD);
int i_show(WORD, WORD, WORD);
int i_emit(WORD, WORD, WORD);
int i_start(WORD, WORD, WORD);
int i_finish(WORD, WORD, WORD);

/* pendsim command functions */
int com_break(char args[][64], int);
int com_clear(char args[][64], int);
int com_continue(char args[][64], int);
int com_dir(char args[][64], int);
int com_read(char args[][64], int);
int com_reg(char args[][64], int);
int com_run(char args[][64], int);
int com_set(char args[][64], int);
int com_state(char args[][64], int);
int com_step(char args[][64], int);
int com_unbreak(char args[][64], int);
int com_write(char args[][64], int);
int com_help(char args[][64], int);
int com_quit(char args[][64], int);

/* main.c */
unsigned int power(unsigned int, unsigned int);
int sign_extend(int, int);
void *my_malloc(size_t);
char *parse_command_line(int, char**);
int usage(void);

#endif /* ifndef PENDSIM_H */
