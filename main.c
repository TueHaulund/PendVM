/* $Id: main.c,v 1.3 1997/06/15 05:29:54 deberg Exp $ */

/* 
 PAL pendulum VM, written by matt debergalis <deberg@ai.mit.edu>
 
 modified from original pendsim 1.0 by matt debergalis, which ran BIN code.

 partly based on code written by mike frank <mpf@ai.mit.edu>.

 pendvm 2.0 uses unfinished spec for a reversible language by mike frank.
 pendsim 1.0 used the PISA spec as the authoritative machine definition. 

 for complete documentation, see pendvm.tex (or pendvm.{dvi,ps})
*/

#include <stdio.h>
#include <stdlib.h>
#include "pendvm.h"

/* exported globals */
char *progname; /* argv[0] */
int output_radix=10;

/* static globals */
int interactive=0; /* 1 if --debug specified */

int
main(int argc, char *argv[])
{
  char *input; /* input filename (program to run) */

  progname=argv[0];
  input=parse_command_line(argc, argv); /* parse options, get input filename */
  init_machine(); /* initialize machine */
  load_imem(input); /* load the instructions */

  if( interactive ) {
    printf("pendvm %s -- matt debergalis <deberg@ai.mit.edu>\n",VERSION);
    loop(); /* never returns */

  } else {
    int result=step_processor(-1); /* just go on until halt */
    
    if( !result ) return 0; /* all was good */
    
    /* something went wrong - give 'coredump' and exit */

    printf("\n"); /* already printed error message */
    com_reg(NULL, 0); /* this is wrong... */
    display_state();
    printf("\n");

    return 1;
  }
}

/* EXTRACT(nu,low,high) -

   Extracts from num its bits in positions LOW through HIGH, shifted
   right to occupy the lowest bits of the result word.
*/

unsigned int EXTRACT(unsigned int num,unsigned int low, unsigned int high)
{
  unsigned int a,b;

  a=num >> low;
  b=(2 << (high-low)) - 1;

  return a & b;
}

unsigned int power(unsigned int a, unsigned int n)
{
  unsigned int res=1;

  while(n--) res*=a;
  return res;
}

int sign_extend(int value, int length)
{
  int result=0;

  if( length==32 ) return value;

  if( EXTRACT(value,length-1,length-1) ) 
    result=power(2,32-length) << length;

  return( result | ( value << (32-length) >> (32-length) ) );
}

char *parse_command_line(int argc, char *argv[])
{
  register char **current;
  char *input=NULL;
  
  for (current=argv+1; *current; current++) {
    if (!strcmp(*current, OPTION_HELP))
      usage();
    else if (!strcmp(*current, OPTION_VERSION)) {
      printf("pendvm version %s compiled on %s.\n",VERSION,__DATE__);
      exit(1);
    }
    else if (!strcmp(*current, OPTION_DEBUG))
      interactive = 1;
    else if (!strcmp(*current, "--radix")) {
      int radix;
      current++;
      radix=atoi(*current);
      if( radix==10 ) output_radix=10;
      else if( radix==16 ) output_radix=16;
      else printf("Ignoring bad output radix (must be 10 or 16).\n");
    }
    else if(input) /* we already have a filename */
      usage();
    else /* get the input filename */
      input=*current;
  }
  if (!input) usage(); /* we never got a filename */
  return input;
} /* parse_command_line */

int usage(void)
{
  printf("USAGE: pendvm <PRP-PAL-file>\n");
  printf("OPTIONS:\n");
  printf(" --radix r -- output using specified radix, which should be 10 or 16\n");
  printf(" --debug -- use interactive debugging mode\n");
  printf(" --version -- show detailed version information\n");
  printf(" --help -- this message\n");
  exit(1);
}

