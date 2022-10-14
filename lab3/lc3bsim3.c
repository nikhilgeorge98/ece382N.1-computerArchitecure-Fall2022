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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
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
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
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
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

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

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
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

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
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
void initialize(char *ucode_filename, char *files[], int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	    load_program(files[i]);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

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

    initialize(argv[1], &argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
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
    a = ir5;
    d = ir4;
    amt4 = ir & 0x000f; //0000 0000 0000 1111
    sr1_15 = (CURRENT_LATCHES.REGS[sr1] & 0x8000)>>15; //1000 0000 0000 0000

    drmux = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if(drmux == 1){
        dr = 7; //111
    }
    else if(drmux == 0){
        dr = (ir & 0x0E00) >> 9; //0000 1110 0000 0000 
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
}

void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */

    // int ir = 0xF331; //0000 0000 0000 0000 1111 0011 0011 0001
    // int ird = 0x0000;
    // int cond = 0x0002; //0000 0000 0000 0010
    // int ben = 0x0001;
    // int r = 0x0000;
    // int j = 0x0017; //0000 0000 0001 0111
    // printf("in eval_micro_sequencer:\n");

    initilize_everything();

    int next_state_address;
    
    // printf("Cond1:%d 0x%.8x Cond0:%d 0x%.8x\n", cond1, cond1, cond0, cond0);
    // printf("j5:%d 0x%.8x j4:%d 0x%.8x j3:%d 0x%.8x j2:%d 0x%.8x j1:%d 0x%.8x j0:%d 0x%.8x\n", j5, j5, j4, j4, j3, j3, j2, j2, j1, j1, j0, j0);
    // printf("Not Cond1:%d 0x%.8x\n", (~cond1)&0x0001, (~cond1)&0x0001);
    // printf("Not Cond1:%d 0x%.8x\n", Low1bit(~cond1), Low1bit(~cond1));
    // printf("ORing 2 1's %d 0x%.8x\n", (1 | 1), (1 | 1));
    // printf("bit5: %d 0x%.8x bit5afterleftshift: %d 0x%.8x\n", j5, j5, (j5<<5), (j5<<5));
    // printf("bit4: %d 0x%.8x bit4afterleftshift: %d 0x%.8x\n", j4, j4, (j4<<4), (j4<<4));
    // printf("bit3: %d 0x%.8x bit4afterleftshift: %d 0x%.8x\n", j3, j3, (j3<<3), (j3<<3));
    // printf("bit2: %d 0x%.8x\n", (Low1bit(j2 | Low1bit(cond1 & Low1bit(~cond0) & ben)) << 2), (Low1bit(j2 | Low1bit(cond1 & Low1bit(~cond0) & ben)) << 2));
    // printf("bit1: %d 0x%.8x\n", (Low1bit(j1 | Low1bit(Low1bit(~cond1) & cond0 & r)) << 1), (Low1bit(j1 | Low1bit(Low1bit(~cond1) & cond0 & r)) << 1));
    // printf("bit0: %d 0x%.8x\n", Low1bit(j0 | Low1bit(cond1 & cond0 & ir11)), Low1bit(j0 | Low1bit(cond1 & cond0 & ir11)));
    // printf("bit2: %d 0x%.8x\n", ((j2 | (cond1 & (~cond0) & ben)) << 2), ((j2 | (cond1 & (~cond0) & ben)) << 2));
    // printf("bit1: %d 0x%.8x\n", ((j1 | ((~cond1) & cond0 & r)) << 1), ((j1 | ((~cond1) & cond0 & r)) << 1));
    // printf("bit0: %d 0x%.8x\n", (j0 | (cond1 & cond0 & ir11)), (j0 | (cond1 & cond0 & ir11)));

    if(ird == 1){
        next_state_address = (ir & 0xf000) >> 12; //1111 0000 0000 0000
    }
    else{
        // next_state_address = (j5<<5) + 
        //                      (j4<<4) + 
        //                      (j3<<3) + 
        //                      (Low1bit(j2 | Low1bit(cond1 & Low1bit(~cond0) & ben)) << 2) + 
        //                      (Low1bit(j1 | Low1bit(Low1bit(~cond1) & cond0 & r)) << 1) + 
        //                      Low1bit(j0 | Low1bit(cond1 & cond0 & ir11));
        next_state_address = (j5<<5) + 
                             (j4<<4) + 
                             (j3<<3) + 
                             ((j2 | (cond1 & (~cond0) & ben)) << 2) + 
                             ((j1 | ((~cond1) & cond0 & r)) << 1) + 
                             (j0 | (cond1 & cond0 & ir11));
    }
    // printf("ird = %d\n", ird);
    // printf("ir = %d 0x%.8x\n", ir, ir);
    // printf("(ir & 0xf000) = %d 0x%.8x\n", (ir & 0xf000), (ir & 0xf000));
    // printf("Current state adress: %d 0x%.8x\n", CURRENT_LATCHES.STATE_NUMBER, CURRENT_LATCHES.STATE_NUMBER);
    // printf("Next state adress: %d 0x%.8x\n", next_state_address, next_state_address);
    
    NEXT_LATCHES.STATE_NUMBER = next_state_address;

    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

    // printf("End of eval_micro_sequencer:*****************************\n\n");

}


void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */

  //READY compute
  //WE0/WE1 <- Assign value
    // GetDATA_SIZE(), CURRENT_LATCHES.MAR[0], MIO.EN

  //if(we1 == 1)
  //{MEM[][1] = MDR[15:8];}
  // MEM_VALUE will be stored in this function

//   printf("In cycle memory:\n");

    // int mar = CURRENT_LATCHES.MAR;
    // int mdr = CURRENT_LATCHES.MDR;

    // int rw = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
    // int ready = CURRENT_LATCHES.READY;
    // int mio_en = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);

    // int datasize = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
    // int mar0 = mar & 0x0001;

    // cycle_count = CYCLE_COUNT % 5;
    // printf("my CYCLE count: %d\n", cycle_count);
    // printf("mdr: %d 0x%.8x\n", mdr, mdr);
    // printf("mar: %d 0x%.8x\n", mar, mar);
    // printf("rw: %d 0x%.8x\n", rw, rw);
    // printf("ready: %d 0x%.8x\n", CURRENT_LATCHES.READY, CURRENT_LATCHES.READY);
    // printf("mio_en: %d 0x%.8x\n", mio_en, mio_en);
    // printf("datasize: %d 0x%.8x\n", datasize, datasize);
    // printf("mar0: %d 0x%.8x\n", mar0, mar0);


    if(mio_en == 1){
        if(rw == 1 && CURRENT_LATCHES.READY == 1){
            if(datasize == 1){
                // printf("READY TO WRITE TO MEMORY");
                // printf("mioen = 1, rw = 1, datasize = 1\n");
                // printf("(mdr & 0xff00)>>8: %d 0x%.8x\n", (mdr & 0xff00)>>8, (mdr & 0xff00)>>8);
                // printf("mdr & 0x00ff: %d 0x%.8x\n", mdr & 0x00ff, mdr & 0x00ff);
                MEMORY[mar/2][1] = (mdr & 0xff00)>>8;
                MEMORY[mar/2][0] = mdr & 0x00ff;
                // mem_write = mdr;
            }
            else if(datasize == 0){
                if(mar0 = 1){
                    // printf("mioen = 1, rw = 1, datasize = 0, mar0 = 1\n");
                    // printf("(mdr & 0xff00)>>8: %d 0x%.8x\n", (mdr & 0xff00)>>8, (mdr & 0xff00)>>8); //1111 1111 0000 0000
                    // MEMORY[mar/2][mar%2] = (mdr & 0xff00) >> 8;
                    MEMORY[mar/2][mar%2] = mdr & 0x00ff;
                    // mem_write = (mdr & 0xff00) >> 8;
                }
                else{
                    // printf("mioen = 1, rw = 1, datasize = 0, mar0 = 0\n");
                    // printf("mdr & 0x00ff: %d 0x%.8x\n", mdr & 0x00ff, mdr & 0x00ff); //0000 0000 1111 1111
                    MEMORY[mar/2][mar%2] = mdr & 0x00ff;
                    // mem_write = mdr & 0x00ff;
                }
            }            
        }
        else if(rw == 0 && CURRENT_LATCHES.READY == 1){
            // printf("READY TO WRITE TO MEMORY");
        // else if(rw == 0){
            // printf("mioen = 1, rw = 0, ready = 1\n");
            // printf("(MEMORY[mar/2][1]<<8) + MEMORY[mar/2][0]: %d 0x%.8x\n", (MEMORY[mar/2][1]<<8) + MEMORY[mar/2][0], (MEMORY[mar/2][1]<<8) + MEMORY[mar/2][0]);
            mem_read = (MEMORY[mar/2][1]<<8) + MEMORY[mar/2][0];
        }
        cycle_count++;
    }

    // printf("my CYCLE count after mioen = 1: %d\n", cycle_count);

    if(cycle_count == 4){
        NEXT_LATCHES.READY = 1;
        // printf("my CYCLE count if cycle 4 = 1: %d\n", cycle_count);
    }

    if(cycle_count == 5){
        NEXT_LATCHES.READY = 0;
        cycle_count = 0;
        // printf("my CYCLE count if cycle 5 = 0: %d\n", cycle_count);
    }

    // if(NEXT_LATCHES.STATE_NUMBER!=CURRENT_LATCHES.STATE_NUMBER)
    //     cycle_count = 0;

    // printf("my CYCLE count at the end of cycle memory: %d\n", cycle_count);

    // printf("End of cycle memory:\n\n");        

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

  // input_marmux, input_pc, input_alu, input_shf, input_mdr    
  // PC Increment

    // int input_Gate_MARMUX;
    // int input_Gate_PC;
    // int input_Gate_ALU;
    // int input_Gate_SHF;
    // int input_Gate_MDR;
    // printf("In eval bus drivers:*************************\n");

    // int ir = CURRENT_LATCHES.IR;

    input_Gate_PC = CURRENT_LATCHES.PC;

    // int sr1mux = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    
    // int sr1;
    // if(sr1mux == 1){
    //     sr1 = (ir & 0x01C0) >> 6; //0000 0001 1100 0000
    // }
    // else if(sr1mux == 0){
    //     sr1 = (ir & 0x0E00) >> 9; //0000 1110 0000 0000 
    // }
    // int a = (ir & 0x0020) >> 5;
    // int d = (ir & 0x0010) >> 4;
    // int amt4 = ir & 0x000f;

    if(d==0){
        // input_Gate_SHF = Low16bits(CURRENT_LATCHES.REGS[sr1] << amt4);
        input_Gate_SHF = CURRENT_LATCHES.REGS[sr1] << amt4;
    }
    else{
        if(a==0){
            // input_Gate_SHF = Low16bits(CURRENT_LATCHES.REGS[sr1] >> amt4);
            input_Gate_SHF = CURRENT_LATCHES.REGS[sr1] >> amt4;
        }
        else{
            // int sr1_15 = (CURRENT_LATCHES.REGS[sr1] & 0x8000)>>15;//1000 0000 0000 0000
            // input_Gate_SHF = Low16bits(CURRENT_LATCHES.REGS[sr1] >> amt4); 
            input_Gate_SHF = CURRENT_LATCHES.REGS[sr1] >> amt4; 
            if(sr1_15 == 1) //if negative sign extend offset here
                // input_Gate_SHF = Low16bits(twosCompliment(input_Gate_SHF, 16 - amt4));
                input_Gate_SHF = twosCompliment(input_Gate_SHF, 16 - amt4);
        }
    }

    // int imm5 = ir & 0x001f;
    // if((imm5 >> 4) == 1){
    //     imm5 = twosCompliment(imm5, 5);
    // }
    
    // int sr2muxOut;

    // // int ir5 = (ir & 0x0020) >> 5; //0000 0000 0010 0000
    // if(ir5 == 1){
    //     sr2muxOut = imm5;
    // }
    // else if(ir5 == 0){
    //     int sr2 = ir & 0x0007; //0000 1110 0000 0000
    //     sr2muxOut = CURRENT_LATCHES.REGS[sr2];
    // }

    // int aluk = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
    
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
    
    // update_nzp(input_Gate_ALU);

    // int pc = CURRENT_LATCHES.PC;

    // int zero = 0;
    // int offset6 = ir & 0x003f; // 0000 0000 0011 1111
    // if(offset6 >> 5 == 1){
    //     offset6 = twosCompliment(offset6, 6);
    // }
    // int pcoffset9 = ir & 0x01ff; // 0000 0001 1111 1111
    // if(pcoffset9 >> 8 == 1){
    //     pcoffset9 = twosCompliment(pcoffset9, 9);
    // }
    // int pcoffset11 = ir & 0x07ff; // 0000 0111 1111 1111
    // if(pcoffset11 >> 10 == 1){
    //     pcoffset11 = twosCompliment(pcoffset11, 11);
    // }

    // int zextandlshf1 = (ir & 0x00ff) << 1;

    // int addr1mux = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    // int addr2mux = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    // int lshf1 = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
    // int marmux = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);

    // int addr1muxout;
    // int addr2muxout;
    // int lshf1out;
    // int adderout;
    // printf("*****************MARMUX**********************\n");
    // printf("addr1mux: %d 0x%.8x\n", addr1mux,addr1mux);
    // printf("addr2mux: %d 0x%.8x\n", addr2mux,addr2mux);
    // printf("lshf1: %d 0x%.8x\n", lshf1,lshf1);
    // printf("marmux: %d 0x%.8x\n", marmux,marmux);
    // printf("ir: %d 0x%.8x\n", ir,ir);
    // printf("zextandlshf1: %d 0x%.8x\n", zextandlshf1,zextandlshf1);

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
    // adderout = Low16bits(addr1muxout + lshf1out);
    adderout = addr1muxout + lshf1out;
    if((adderout & 0x8000) >> 15){
        adderout = twosCompliment(adderout, 16);
    }

    // printf("addr1muxout: %d 0x%.8x\n", addr1muxout,addr1muxout);
    // printf("addr2muxout: %d 0x%.8x\n", addr2muxout,addr2muxout);
    // printf("lshf1out: %d 0x%.8x\n", lshf1out,lshf1out);
    // printf("adderout: %d 0x%.8x\n", adderout,adderout);

    //marmux
    if(marmux == 0){
        input_Gate_MARMUX = zextandlshf1;
    }
    else if(marmux == 1){
        input_Gate_MARMUX = adderout;
    }

    // printf("input_Gate_MARMUX: %d 0x%.8x\n", input_Gate_MARMUX,input_Gate_MARMUX);
    // printf("****************************************************************\n");

    // int datasize = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
    // int mdr = CURRENT_LATCHES.MDR;
    // int mar0 = CURRENT_LATCHES.MAR & 0x0001;

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

    // printf("input_Gate_ALU: %d 0x%.8x\n", input_Gate_ALU, input_Gate_ALU);
    // printf("input_Gate_MARMUX: %d 0x%.8x\n", input_Gate_MARMUX, input_Gate_MARMUX);
    // printf("input_Gate_MDR: %d 0x%.8x\n", input_Gate_MDR, input_Gate_MDR);
    // printf("input_Gate_PC: %d 0x%.8x\n", input_Gate_PC, input_Gate_PC);
    // printf("input_Gate_SHF: %d 0x%.8x\n", input_Gate_SHF, input_Gate_SHF);
    // printf("Ended eval bus drivers*********************************\n\n");
    // NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;

}


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */

//   if(Gate_PC)
//     bus = input_pc

    // printf("In drive bus*********************************\n");

    // int gatePC = GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION);
    // int gateMARMUX = GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    // int gateALU = GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION);
    // int gateSHF = GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION);
    // int gateMDR = GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION);

    if(gatePC){
        // printf("GatePC is 1\n");
        BUS = Low16bits(input_Gate_PC);
    }
    else if(gateALU){
        // printf("gateALU is 1\n");
        BUS = Low16bits(input_Gate_ALU);
    }
    else if(gateMARMUX){
        // printf("gateMARMUX is 1\n");
        BUS = Low16bits(input_Gate_MARMUX);
    }
    else if(gateMDR){
        // printf("gateMDR is 1\n");
        BUS = Low16bits(input_Gate_MDR);
    }
    else if(gateSHF){
        // printf("gateSHF is 1\n");
        BUS = Low16bits(input_Gate_SHF);
    }
    else{
        // printf("All tristate buffers are closed\n");
        BUS = 0;
    }
    // printf("BUS: %d 0x%.8x\n", BUS, BUS);
    // printf("Ended drive bus\n\n");
    
}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */
         
         //LATCH.IR = BUS;
         //LATCH.REG
    // printf("In lathc datapath\n");

    // int ldmar = GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION);
    // int ldmdr = GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
    // int ldir = GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION);
    // int ldben = GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION);
    // int ldreg = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
    // int ldcc = GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION);
    // int ldpc = GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION);

    // int ir = CURRENT_LATCHES.IR;

    // int sr1mux = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    
    // int sr1;
    // if(sr1mux == 1){
    //     sr1 = (ir & 0x01C0) >> 6; //0000 0001 1100 0000
    // }
    // else if(sr1mux == 0){
    //     sr1 = (ir & 0x0E00) >> 9; //0000 1110 0000 0000 
    // }

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
        // printf("LD.PC\n");
        // int pc = CURRENT_LATCHES.PC;

        // int pc2 = CURRENT_LATCHES.PC + 2;

        // int zero = 0;
        // int offset6 = ir & 0x003f; // 0000 0000 0011 1111
        // if(offset6 >> 5 == 1){
        //     offset6 = twosCompliment(offset6, 6);
        // }
        // int pcoffset9 = ir & 0x01ff; // 0000 0001 1111 1111
        // if(pcoffset9 >> 8 == 1){
        //     pcoffset9 = twosCompliment(pcoffset9, 9);
        // }
        // int pcoffset11 = ir & 0x07ff; // 0000 0111 1111 1111
        // if(pcoffset11 >> 10 == 1){
        //     pcoffset11 = twosCompliment(pcoffset11, 11);
        // }

        // int addr1mux = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
        // int addr2mux = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
        // int lshf1 = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
        // int marmux = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);

        // int addr1muxout;
        // int addr2muxout;
        // int lshf1out;
        // int adderout;

        // addr1MUX
        // if(addr1mux == 0){
        //     addr1muxout = pc;
        // }
        // else if(addr1mux == 1){
        //     addr1muxout = CURRENT_LATCHES.REGS[sr1]; //check
        // }

        // addr2MUX
        // if(addr2mux == 0){
        //     addr2muxout = zero;
        // }
        // else if(addr2mux == 1){
        //     addr2muxout = offset6;
        // }
        // else if(addr2mux == 2){
        //     addr2muxout = pcoffset9;
        // }
        // else if(addr2mux == 3){
        //     addr2muxout = pcoffset11;
        // }

        // lshf1
        // if(lshf1 == 1){
        //     lshf1out = addr2muxout << 1;
        // }
        // else{
        //     lshf1out = addr2muxout;
        // }

        //adder
        // adderout = Low16bits(addr1muxout + addr2muxout);
        // if((adderout & 0x8000) >> 1){
        //     adderout = twosCompliment(adderout, 16);
        // }

        // int pcmux = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
        if(pcmux == 0){
            NEXT_LATCHES.PC = pc2;
        }
        else if(pcmux == 1){
            NEXT_LATCHES.PC = BUS;
        }
        else if(pcmux == 2){
            NEXT_LATCHES.PC = adderout;
        }
    }

    //LD.REG
    if(ldreg){
        // printf("LD.REG\n");
        // int drmux = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);

        // int dr;
        // if(drmux == 1){
        //     dr = 7; //111
        // }
        // else if(drmux == 0){
        //     dr = (ir & 0x0E00) >> 9; //0000 1110 0000 0000 
        // }

        NEXT_LATCHES.REGS[dr] = BUS;
    }

    //LD.MDR
    if(ldmdr){
        // printf("LD.MDR******************************************\n");
        // int mio_en = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);
        // printf("mio_en: %d 0x%.8x\n", mio_en, mio_en);

        // int mar = CURRENT_LATCHES.MAR;
        // printf("mar: %d 0x%.8x\n", mar, mar);

        // int datasize = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
        // int mar0 = mar & 0x0001;
        // printf("datasize: %d 0x%.8x\n", datasize, datasize);
        // printf("mar0: %d 0x%.8x\n", mar0, mar0);

        // printf("BUS: %d 0x%.8x\n", BUS, BUS);
        // printf("(BUS & 0xff00) >> 8: %d 0x%.8x\n", (BUS & 0xff00) >> 8, (BUS & 0xff00) >> 8);
        // printf("BUS & 0x00ff: %d 0x%.8x\n", BUS & 0x00ff, BUS & 0x00ff);
        // printf("((BUS & 0x00ff) << 8) + (BUS & 0x00ff): %d 0x%.8x\n", ((BUS & 0x00ff) << 8) + (BUS & 0x00ff), ((BUS & 0x00ff) << 8) + (BUS & 0x00ff));

        int logicout;
        // int memout = (MEMORY[CURRENT_LATCHES.REGS[mar]/2][1]<<8) + MEMORY[CURRENT_LATCHES.REGS[mar]/2][0];

        // if(datasize == 1){
        //     logicout = BUS;
        // }
        // else{
        //     // logicout = BUS;
        //     if(mar0 == 1){
        //         logicout = ((BUS & 0x00ff) << 8) + (BUS & 0x00ff); //sign extend?
        //     }
        //     else{
        //         logicout = BUS;
        //     }
        // }
        // printf("Current.LATCH.READY: %d", CURRENT_LATCHES.READY);
        // if(CURRENT_LATCHES.READY){
            if(mio_en == 1){
                NEXT_LATCHES.MDR = Low16bits(mem_read);
            }
            else{
                if(mar0 == 1){
                    logicout = ((BUS & 0x00ff) << 8) + (BUS & 0x00ff); 
                }
                else{
                    logicout = BUS;
                }
                NEXT_LATCHES.MDR = Low16bits(logicout);
            }
        // }
        

        // printf("NEXT_LATCHES.MDR: %d 0x%.8x\n", NEXT_LATCHES.MDR, NEXT_LATCHES.MDR);
        // printf("End LD.MDR********************************!!!!!!!!!!!!!***********\n");

    }
    
    //LD.BEN
    if(ldben){
        // int ir11 = (ir & 0x0800) >> 11; //0000 1000 0000 0000
        // int ir10 = (ir & 0x0400) >> 10; //0000 0100 0000 0000
        // int ir9 = (ir & 0x0200) >> 9; //0000 0010 0000 0000
        // int ir1512 = (ir & 0xf000) >> 12; //1111 0000 0000 0000 

        NEXT_LATCHES.BEN = (ir11 & CURRENT_LATCHES.N) |
                           (ir10 & CURRENT_LATCHES.Z) |
                           (ir9 & CURRENT_LATCHES.P);
    }
    

    // printf("Ended lathc datapath\n\n");

}