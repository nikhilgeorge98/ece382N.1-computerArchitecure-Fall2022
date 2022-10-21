/*
    Name 1: Nikhil George
    UTEID 1: ng25762
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

int input_Gate_MARMUX;
int input_Gate_PC;
int input_Gate_ALU;
int input_Gate_SHF;
int input_Gate_MDR;

int cycle_count = 0;

int mem_read;
// int mem_write;

// int we1;
// int we0;

int input_Gate_PSR;
int input_Gate_IVTADDER;
int input_Gate_USP;
int input_Gate_SSP;
int input_Gate_R62;

int setBit(int, int);
int resetBit(int, int);
int twosCompliment(int, int);
void update_nzp(int);
void initilize_everything();

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)
#define Low1bit(x) ((x) & 0x0001)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* SOME CONSTANTS I NEED                                       */
/***************************************************************/
#define TIMERINTVEC 0x01
#define PROTECTEXCVEC 0x02
#define UNALIGNEXCVEC 0x03
#define UNKNOWNEXCVEC 0x04

#define IVTBASEADDR 0x0200

#define SYSTEMSPACE_LOWER 0x0000
#define SYSTEMSPACE_UPPER 0x2FFF

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND2, COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX0,
    SR1MUX1, SR1MUX0,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
/* MODIFY: you have to add all your new control signals */
    LD_PSR,
    GATE_PSR,
    PSRMUX,
    LD_IEV,
    LSHF1_N,
    GATE_IVTADDER,
    IEVMUX1, IEVMUX0,
    GATE_USP,
    GATE_SSP,
    LD_USP,
    LD_SSP,
    USPMUX,
    SSPMUX,
    REGMUX1, REGMUX0,
    GATE_R62,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + x[DRMUX0]); }
int GetSR1MUX(int *x)        { return((x[SR1MUX1] << 1) + x[SR1MUX0]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */
int GetLD_PSR(int *x)        { return(x[LD_PSR]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetPSRMUX(int *x)        { return(x[PSRMUX]); }
int GetLD_IEV(int *x)        { return(x[LD_IEV]); }
int GetLSHF1_N(int *x)       { return(x[LSHF1_N]); }
int GetGATE_IVTADDER(int *x) { return(x[GATE_IVTADDER]); }
int GetIEVMUX(int *x)        { return((x[IEVMUX1] << 1) + x[IEVMUX0]); }
int GetGATE_USP(int *x)      { return(x[GATE_USP]); }
int GetGATE_SSP(int *x)      { return(x[GATE_SSP]); }
int GetLD_USP(int *x)        { return(x[LD_USP]); }
int GetLD_SSP(int *x)        { return(x[LD_SSP]); }
int GetUSPMUX(int *x)        { return(x[USPMUX]); }
int GetSSPMUX(int *x)        { return(x[SSPMUX]); }
int GetREGMUX(int *x)        { return((x[REGMUX1] << 1) + x[REGMUX0]); }
int GetGATE_R62(int *x)      { return(x[GATE_R62]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */

int PSR;
int USP;
int IEV;
int I;
int E;

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  if(CYCLE_COUNT == 300){
    NEXT_LATCHES.INTV = Low16bits(TIMERINTVEC);
    NEXT_LATCHES.I = 1;
    // printf("The interrupt has been generated - I = %d\n", NEXT_LATCHES.I);
  }

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 2]);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

    CURRENT_LATCHES.PSR = 0X8002; //1000 0000 0000 0010
    CURRENT_LATCHES.USP = 0xFE00;

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/


int mar, mdr, ir, ir11, ir10, ir9, ir5, ir4, ird, mar0, cond, cond0, cond1, ben, r, j, j5, j4, j3, j2, j1, j0, rw, ready, mio_en, datasize;
int sr1mux, sr1, a, d, amt4, sr1_15;
int dr, drmux;
int sr2;
int imm5, offset6, pcoffset9, pcoffset11, zextandlshf1, zero = 0;
int sr2muxOut, aluk, addr1mux, addr2mux, lshf1, marmux;
int addr1muxout, addr2muxout, lshf1out, adderout;  
int pc, pc2, pcmux;
int gatePC, gateMARMUX, gateALU, gateSHF, gateMDR;
int ldmar, ldmdr, ldir, ldben, ldreg, ldcc, ldpc;

int cond2;
int psr, psr15, psrmux;
int i, e;
int intv, excv, iev;
int ievmux;
int gatePSR, gateIVTADDER, gateUSP, gateSSP, gateR62;
int ldpsr, ldiev, ldusp, ldssp;
int usp, ssp;
int uspmux, sspmux;
int uspmuxout, sspmuxout;
int pcminus2;
int r62, r6minus2;
int regmux, regmuxout;
int lshf1_n, lshf1_nout, ivtadderout;

int iropcode;
int flag = 0;

int setBit(int num, int bit){
    int set = 1 << bit;
    num = num | set;
    return num;
}

int resetBit(int num, int bit){
    int reset = ~(1 << bit);
    num = num & reset;
    return num;
}

int twosCompliment(int num, int curbits){
    //printf("\nnum and currbits inside 2scompli is %d %d" , num, curbits);
    int newnum = num - pow(2,curbits);
    //printf("\nnewnum is %d", newnum);
    return newnum;
}

void update_nzp(int res){
    if(res>0){
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 1;
    }
    if(res<0){
        NEXT_LATCHES.N = 1;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 0;
    }
    if(res==0){
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 1;
        NEXT_LATCHES.P = 0;
    }
    return;
}

void initilize_everything(){
    ird = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
    cond = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
    ben = CURRENT_LATCHES.BEN;
    r = CURRENT_LATCHES.READY;
    j = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    ir = CURRENT_LATCHES.IR;

    cond2 = (cond & 0x0004)>>2; //0000 0000 0000 0100
    cond1 = (cond & 0x0002)>>1; //0000 0000 0000 0010
    cond0 = cond & 0x0001; //0000 0000 0000 0001
    j5 = (j & 0x0020)>>5; //0000 0000 0010 0000
    j4 = (j & 0x0010)>>4; //0000 0000 0001 0000
    j3 = (j & 0x0008)>>3; //0000 0000 0000 1000
    j2 = (j & 0x0004)>>2; //0000 0000 0000 0100
    j1 = (j & 0x0002)>>1; //0000 0000 0000 0010
    j0 = j & 0x0001; //0000 0000 0000 0001

    ir11 = (ir & 0x0800) >> 11; //0000 1000 0000 0000
    ir10 = (ir & 0x0400) >> 10; //0000 0100 0000 0000
    ir9 = (ir & 0x0200) >> 9; //0000 0010 0000 0000
    ir5 = (ir & 0x0020) >> 5; //0000 0000 0010 0000
    ir4 = (ir & 0x0010) >> 4; //0000 0000 0001 0000

    iropcode = (ir & 0xF000) >> 12; //1111 0000 0000 0000

    mar = CURRENT_LATCHES.MAR;
    mdr = CURRENT_LATCHES.MDR;

    rw = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
    ready = CURRENT_LATCHES.READY;
    mio_en = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);

    datasize = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
    mar0 = mar & 0x0001; //0000 0000 0000 0001

    sr1mux = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if(sr1mux == 1){
        sr1 = (ir & 0x01C0) >> 6; //0000 0001 1100 0000
    }
    else if(sr1mux == 0){
        sr1 = (ir & 0x0E00) >> 9; //0000 1110 0000 0000 
    }
    else if(sr1mux == 2){
        sr1 = 6; //110 = R6 
    }
    a = ir5;
    d = ir4;
    amt4 = ir & 0x000f; //0000 0000 0000 1111
    sr1_15 = (CURRENT_LATCHES.REGS[sr1] & 0x8000)>>15; //1000 0000 0000 0000

    drmux = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    // printf("drmux 0x%.2x\n",drmux);
    if(drmux == 1){
        dr = 7; //111
    }
    else if(drmux == 0){
        dr = (ir & 0x0E00) >> 9; //0000 1110 0000 0000 
    }
    else if(drmux == 2){
        dr = 6; //110 = 6 
    }

    imm5 = ir & 0x001f; //0000 0000 0001 1111
    if((imm5 >> 4) == 1){
        imm5 = twosCompliment(imm5, 5);
    }

    offset6 = ir & 0x003f; // 0000 0000 0011 1111
    if(offset6 >> 5 == 1){
        offset6 = twosCompliment(offset6, 6);
    }
    pcoffset9 = ir & 0x01ff; // 0000 0001 1111 1111
    if(pcoffset9 >> 8 == 1){
        pcoffset9 = twosCompliment(pcoffset9, 9);
    }
    pcoffset11 = ir & 0x07ff; // 0000 0111 1111 1111
    if(pcoffset11 >> 10 == 1){
        pcoffset11 = twosCompliment(pcoffset11, 11);
    }

    zextandlshf1 = (ir & 0x00ff) << 1; //0000 0000 1111 1111

    if(ir5 == 1){
        sr2muxOut = imm5;
    }
    else if(ir5 == 0){
        sr2 = ir & 0x0007; //0000 0000 0000 0111
        sr2muxOut = CURRENT_LATCHES.REGS[sr2];
    }

    aluk = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);

    addr1mux = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    addr2mux = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    lshf1 = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
    marmux = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);

    pc = CURRENT_LATCHES.PC;
    pc2 = CURRENT_LATCHES.PC + 2;
    pcminus2 = CURRENT_LATCHES.PC - 2;
    pcmux = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    
    gatePC = GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION);
    gateMARMUX = GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    gateALU = GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION);
    gateSHF = GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION);
    gateMDR = GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION);

    ldmar = GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION);
    ldmdr = GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
    ldir = GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION);
    ldben = GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION);
    ldreg = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
    ldcc = GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION);
    ldpc = GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION);

    psr = CURRENT_LATCHES.PSR;
    psr15 = (psr & 0x8000) >> 15; //1000 0000 0000 0000
    psrmux = GetPSRMUX(CURRENT_LATCHES.MICROINSTRUCTION);

    if(CURRENT_LATCHES.STATE_NUMBER == 18){
        i = CURRENT_LATCHES.I;    
    }
    else{
        i = 0;
    }
    // printf("i is currently %d in state %d and cycle_count = %d\n", i, CURRENT_LATCHES.STATE_NUMBER, cycle_count);
    e = CURRENT_LATCHES.E;

    intv = CURRENT_LATCHES.INTV;
    excv = CURRENT_LATCHES.EXCV;

    iev = CURRENT_LATCHES.IEV;

    ievmux = GetIEVMUX(CURRENT_LATCHES.MICROINSTRUCTION);

    gatePSR = GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION);
    gateIVTADDER = GetGATE_IVTADDER(CURRENT_LATCHES.MICROINSTRUCTION);
    gateUSP = GetGATE_USP(CURRENT_LATCHES.MICROINSTRUCTION);
    gateSSP = GetGATE_SSP(CURRENT_LATCHES.MICROINSTRUCTION);
    gateR62 = GetGATE_R62(CURRENT_LATCHES.MICROINSTRUCTION);

    ldpsr = GetLD_PSR(CURRENT_LATCHES.MICROINSTRUCTION);
    ldiev = GetLD_IEV(CURRENT_LATCHES.MICROINSTRUCTION);
    ldusp = GetLD_USP(CURRENT_LATCHES.MICROINSTRUCTION);
    ldssp = GetLD_SSP(CURRENT_LATCHES.MICROINSTRUCTION);

    usp = CURRENT_LATCHES.USP;
    ssp = CURRENT_LATCHES.SSP;

    uspmux = GetUSPMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    sspmux = GetSSPMUX(CURRENT_LATCHES.MICROINSTRUCTION);

    r62 = CURRENT_LATCHES.REGS[6] + 2;
    r6minus2 = CURRENT_LATCHES.REGS[6] - 2;

    regmux = GetREGMUX(CURRENT_LATCHES.MICROINSTRUCTION);

    lshf1_n = GetLSHF1_N(CURRENT_LATCHES.MICROINSTRUCTION);

}

void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */

    initilize_everything();

    int next_state_address;

    if(i==1){
        // printf("r %d e %d ird %d cond2 %d cond1 %d cond0 %d\n", r, e, ird, cond2, cond1, cond0);
    }
    // printf("r %d e %d ird %d cond2 %d cond1 %d cond0 %d\n", r, e, ird, cond2, cond1, cond0);
    if(e == 1){
        next_state_address = 42; // any time there is an exception go to state 42
        NEXT_LATCHES.E = 0;
        flag = 1;
    }
    else{
        if(ird == 1){
            next_state_address = (ir & 0xf000) >> 12; //1111 0000 0000 0000
        }
        else{
            next_state_address = (j5<<5) + 
                                ((j4 | (cond2 & (~cond1) & cond0 & psr15)) << 4) + 
                                ((j3 | (cond2 & (~cond1) & (~cond0) & i)) << 3) + 
                                ((j2 | ((~cond2) & cond1 & (~cond0) & ben)) << 2) + 
                                ((j1 | ((~cond2) & (~cond1) & cond0 & r)) << 1) + 
                                (j0 | ((~cond2) & cond1 & cond0 & ir11));
            // printf("%d ", (j5<<5));
            // printf("%d ", ((j4 | (cond2 & (~cond1) & cond0 & psr15)) << 4));
            // printf("%d ", ((j3 | (cond2 & (~cond1) & (~cond0) & i)) << 3));
            // printf("%d ", ((j2 | ((~cond2) & cond1 & (~cond0) & ben)) << 2));
            // printf("%d ", ((j1 | ((~cond2) & (~cond1) & cond0 & r)) << 1));
            // printf("%d \n", (j0 | ((~cond2) & cond1 & cond0 & ir11)));
        }
    }
    // printf("next state %d\n",next_state_address);
    NEXT_LATCHES.STATE_NUMBER = next_state_address;

    if(NEXT_LATCHES.STATE_NUMBER == 41){
        NEXT_LATCHES.I = 0;
    }

    if(NEXT_LATCHES.STATE_NUMBER == 10 || NEXT_LATCHES.STATE_NUMBER == 11){
        // printf("Unkonw opcode exc \n");
        NEXT_LATCHES.E = 1;
        NEXT_LATCHES.EXCV = Low16bits(UNKNOWNEXCVEC);
    }

    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

}


void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */

    if(mio_en == 1){
        if(rw == 1 && CURRENT_LATCHES.READY == 1){
            if(datasize == 1){
                MEMORY[mar/2][1] = (mdr & 0xff00)>>8;
                MEMORY[mar/2][0] = mdr & 0x00ff;
            }
            else if(datasize == 0){
                if(mar0 = 1){
                    MEMORY[mar/2][mar%2] = mdr & 0x00ff;
                }
                else{
                    MEMORY[mar/2][mar%2] = mdr & 0x00ff;
                }
            }            
        }
        else if(rw == 0 && CURRENT_LATCHES.READY == 1){
            mem_read = (MEMORY[mar/2][1]<<8) + MEMORY[mar/2][0];
        }
        cycle_count++;
    }

    if(cycle_count == 4){
        NEXT_LATCHES.READY = 1;
    }

    if(cycle_count == 5){
        NEXT_LATCHES.READY = 0;
        cycle_count = 0;
    }

}



void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */

    input_Gate_PC = CURRENT_LATCHES.PC;

    if(d==0){
        input_Gate_SHF = CURRENT_LATCHES.REGS[sr1] << amt4;
    }
    else{
        if(a==0){
            input_Gate_SHF = CURRENT_LATCHES.REGS[sr1] >> amt4;
        }
        else{
            input_Gate_SHF = CURRENT_LATCHES.REGS[sr1] >> amt4; 
            if(sr1_15 == 1) //if negative sign extend offset here
                input_Gate_SHF = twosCompliment(input_Gate_SHF, 16 - amt4);
        }
    }
    
    if(aluk == 0){
        // input_Gate_ALU = Low16bits(CURRENT_LATCHES.REGS[sr1] + sr2muxOut);
        input_Gate_ALU = CURRENT_LATCHES.REGS[sr1] + sr2muxOut;
    }
    else if(aluk == 1){
        // input_Gate_ALU = Low16bits(CURRENT_LATCHES.REGS[sr1] & sr2muxOut);
        input_Gate_ALU = CURRENT_LATCHES.REGS[sr1] & sr2muxOut;
    }
    else if(aluk == 2){
        // input_Gate_ALU = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ sr2muxOut);
        input_Gate_ALU = CURRENT_LATCHES.REGS[sr1] ^ sr2muxOut;
    }
    else if(aluk == 3){
        input_Gate_ALU = CURRENT_LATCHES.REGS[sr1];
    }

    int sign = (input_Gate_ALU & 0x8000) >> 15;
    if(sign == 1){
        // input_Gate_ALU = Low16bits(twosCompliment(input_Gate_ALU, 16));
        input_Gate_ALU = twosCompliment(input_Gate_ALU, 16);
    }

    // addr1MUX
    if(addr1mux == 0){
        addr1muxout = pc;
    }
    else if(addr1mux == 1){
        addr1muxout = CURRENT_LATCHES.REGS[sr1]; //check
    }

    // addr2MUX
    if(addr2mux == 0){
        addr2muxout = zero;
    }
    else if(addr2mux == 1){
        addr2muxout = offset6;
    }
    else if(addr2mux == 2){
        addr2muxout = pcoffset9;
    }
    else if(addr2mux == 3){
        addr2muxout = pcoffset11;
    }

    // lshf1
    if(lshf1 == 1){
        lshf1out = addr2muxout << 1;
    }
    else{
        lshf1out = addr2muxout;
    }

    //adder
    adderout = addr1muxout + lshf1out;
    if((adderout & 0x8000) >> 15){
        adderout = twosCompliment(adderout, 16);
    }

    //marmux
    if(marmux == 0){
        input_Gate_MARMUX = zextandlshf1;
    }
    else if(marmux == 1){
        input_Gate_MARMUX = adderout;
    }

    if(datasize == 1){
        input_Gate_MDR = mdr;
        if((input_Gate_MDR & 0x8000) >> 15){
            input_Gate_MDR = twosCompliment(input_Gate_MDR, 16);
        }
    }
    else{
        if(mar0 == 1){
            input_Gate_MDR = (mdr & 0xff00) >> 8;
            if((input_Gate_MDR & 0x0080) >> 7){
                input_Gate_MDR = twosCompliment((mdr & 0xff00) >> 8, 8);
            }
        }
        else{
            input_Gate_MDR = mdr & 0x00ff;
            if((input_Gate_MDR & 0x0080) >> 7){
                input_Gate_MDR = twosCompliment(mdr & 0x00ff, 8);
            }
        }
    }

    input_Gate_PSR = CURRENT_LATCHES.PSR;

    lshf1_nout = iev << 1;
    ivtadderout = lshf1_nout + IVTBASEADDR;
    if((ivtadderout & 0x8000) >> 15){
        ivtadderout = twosCompliment(ivtadderout, 16);
    }
    input_Gate_IVTADDER = ivtadderout;

    input_Gate_USP = CURRENT_LATCHES.USP;

    input_Gate_SSP = CURRENT_LATCHES.SSP;

    if((r6minus2 & 0x8000) >> 15){
        r6minus2 = twosCompliment(r6minus2, 16);
    }
    input_Gate_R62 = r6minus2;

}


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */

    if(gatePC){
        BUS = Low16bits(input_Gate_PC);
    }
    else if(gateALU){
        BUS = Low16bits(input_Gate_ALU);
    }
    else if(gateMARMUX){
        BUS = Low16bits(input_Gate_MARMUX);
    }
    else if(gateMDR){
        BUS = Low16bits(input_Gate_MDR);
    }
    else if(gateSHF){
        BUS = Low16bits(input_Gate_SHF);
    }
    else if(gatePSR){
        BUS = Low16bits(input_Gate_PSR);
    }
    else if(gateIVTADDER){
        BUS = Low16bits(input_Gate_IVTADDER);
    }
    else if(gateUSP){
        BUS = Low16bits(input_Gate_USP);
    }
    else if(gateSSP){
        BUS = Low16bits(input_Gate_SSP);
    }
    else if(gateR62){
        BUS = Low16bits(input_Gate_R62);
    }
    else{
        BUS = 0;
    }
}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */

    //LD.IR
    if(ldir){
        // printf("LD.IR\n");
        NEXT_LATCHES.IR = BUS;
    }
    
    //LD.MAR
    if(ldmar){
        // printf("LD.MAR\n");
        NEXT_LATCHES.MAR = BUS;
    }
    
    //LD.CC
    if(ldcc){
        // printf("LD.CC\n");
        if(BUS>>15 == 1)
            update_nzp(twosCompliment(BUS, 16));
        else
            update_nzp(BUS);
    }
    
    //LD.PC
    if(ldpc){
        if(pcmux == 0){
            NEXT_LATCHES.PC = pc2;
        }
        else if(pcmux == 1){
            NEXT_LATCHES.PC = BUS;
        }
        else if(pcmux == 2){
            NEXT_LATCHES.PC = adderout;
        }
        else if(pcmux == 3){
            NEXT_LATCHES.PC = pcminus2;
        }
    }

    //LD.REG
    if(ldreg){
        // printf("loading dr = %d\n", dr);
        if(regmux == 0){
            NEXT_LATCHES.REGS[dr] = BUS;
        }
        else if(regmux == 1){
            NEXT_LATCHES.REGS[dr] = r62;
        }
        else if(regmux == 2){
            NEXT_LATCHES.REGS[dr] = r6minus2;
        }
    }

    //LD.MDR
    if(ldmdr){

        int logicout;
        if(mio_en == 1){
            NEXT_LATCHES.MDR = Low16bits(mem_read);
        }
        else{
            if(mar0 == 1){
                logicout = ((BUS & 0x00ff) << 8) + (BUS & 0x00ff); 
                    logicout = ((BUS & 0x00ff) << 8) + (BUS & 0x00ff); 
                logicout = ((BUS & 0x00ff) << 8) + (BUS & 0x00ff); 
            }
            else{
                logicout = BUS;
            }
            NEXT_LATCHES.MDR = Low16bits(logicout);
        }
    }
    
    //LD.BEN
    if(ldben){
        NEXT_LATCHES.BEN = (ir11 & CURRENT_LATCHES.N) |
                           (ir10 & CURRENT_LATCHES.Z) |
                           (ir9 & CURRENT_LATCHES.P);
    }

    if(ldpsr){
        if(psrmux == 0){
            NEXT_LATCHES.PSR = BUS;
        }
        else if(psrmux == 1){
            NEXT_LATCHES.PSR = Low16bits(resetBit(psr, 15));
            psr15 = 0;
        }
    }

    if(ldiev){
        if(ievmux == 0){
            NEXT_LATCHES.IEV = 0;
        }
        else if(ievmux == 1){
            NEXT_LATCHES.IEV = Low16bits(intv);
        }
        else if(ievmux == 2){
            NEXT_LATCHES.IEV = Low16bits(excv);
        }
    }

    if(ldusp){
        if(uspmux == 0){
            NEXT_LATCHES.USP = 0xFE00;
        }
        else if(uspmux == 1){
            NEXT_LATCHES.USP = CURRENT_LATCHES.REGS[6];
        } 
    }

    if(ldssp){
        if(uspmux == 0){
            NEXT_LATCHES.SSP = 0x3000;
        }
        else if(uspmux == 1){
            NEXT_LATCHES.SSP = CURRENT_LATCHES.REGS[6];
        }
    }

    
    //(psr15 == 1 && NEXT_LATCHES.MAR >= SYSTEMSPACE_LOWER && NEXT_LATCHES.MAR <= SYSTEMSPACE_UPPER)
    // (mio_en == 1 && datasize == 1 && mar0 == 1)

    if(psr15 == 1 && NEXT_LATCHES.MAR >= SYSTEMSPACE_LOWER && NEXT_LATCHES.MAR <= SYSTEMSPACE_UPPER && iropcode != 15 && iropcode != 8 && flag==0){
        // printf("psr15 %d mar 0x%.8x priority exc \n", psr15, NEXT_LATCHES.MAR);
        NEXT_LATCHES.E = 1;
        NEXT_LATCHES.EXCV = Low16bits(PROTECTEXCVEC);
    }
    else if(mio_en == 1 && datasize == 1 && mar0 == 1){
        // printf("Unaligned access exc MAR = 0x%.8x \n", mar);
        NEXT_LATCHES.E = 1;
        NEXT_LATCHES.EXCV = Low16bits(UNALIGNEXCVEC);
    }
}