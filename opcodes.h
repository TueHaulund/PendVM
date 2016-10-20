/* opcodes.h -- defines for opcode values */

/* i'm abandoning the pendasm technique of reading the verilog file
   and parsing for the opcode values, and just hardcoding them here
   for simplicity. */

/* opcodes */
#define OP_SPECIAL 000

#define OP_J       001
#define OP_JR      002
#define OP_BAL     003
#define OP_BLTZ    004
#define OP_BGEZ    005
#define OP_BEQ     006
#define OP_BNE     007
#define OP_BLEZ    010
#define OP_BGTZ    011

#define OP_J_R       021
#define OP_JR_R      022
#define OP_BAL_R     023
#define OP_BLTZ_R    024
#define OP_BGEZ_R    025
#define OP_BEQ_R     026
#define OP_BNE_R     027
#define OP_BLEZ_R    030
#define OP_BGTZ_R    031

#define OP_ADDI    040
#define OP_SLTI    042
#define OP_ANDI    044
#define OP_ORI     045
#define OP_XORI    046
#define OP_USLTI   050
#define OP_UANDI   052
#define OP_UORI    053

#define OP_EXCH    060

#define OP_START   070
#define OP_FINISH  071

/* functions */
/* Arithmetic and logical. */
#define FN_ADD	 001
#define FN_SUB   003
#define FN_AND   005
#define FN_OR    006
#define FN_XOR   007
#define FN_NOR   010
#define FN_NEG   011
#define FN_SLT   012
#define FN_UAND  025
#define FN_UOR   026
#define FN_UNOR  030
#define FN_USLT  032

/* Shift instructions. */
#define FN_SLL   041
#define FN_SRL   042
#define FN_SRA   043
#define FN_SLLV  044
#define FN_SRLV  045
#define FN_SRAV  046
#define FN_USLL  061
#define FN_USRL  062
#define FN_USRA  063
#define FN_USLLV 064
#define FN_USRLV 065
#define FN_USRAV 066
#define FN_RL    047
#define FN_RR    050
#define FN_RLV   051
#define FN_RRV   052

/* these still seem to be around */
#define FN_PASS  077
#define FN_HIZ   000
