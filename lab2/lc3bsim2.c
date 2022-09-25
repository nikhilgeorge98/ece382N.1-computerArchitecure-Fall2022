/*
    Name 1: Nikhil George 
    Name 2: Subramanya Mohit Kashyap Indumukhi
    UTEID 1: ng25762
    UTEID 2: mi5672
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000){
    cycle();
//break;
}
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
    printf("  0x%.4x (%d) : 0x%.2x%.2x (%d%d)\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0], MEMORY[address][1], MEMORY[address][0]);
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
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
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
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

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

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/
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

void decode (int opcode, int msb, int lsb)
{
	if(opcode == 1){ //add
	//get DR
	//printf("msb: %d 0x%.4x\n", msb, msb);
	//printf("lsb: %d 0x%.4x\n", lsb, lsb);
	int mask1 = 0x000e; //
	int dr = (msb & mask1)>>1;
	//printf("DR: %d\n", dr);
        //get SR1
	int mask2 = 0x0001;
	int mask3 = 0x00C0;
	int sr1MSB = (msb & mask2)<<2; // left shift msb by 2 bits
	int sr1LSB2 = (lsb& mask3)>>6; // right shift lsb by 6 bits
        int sr1 = sr1MSB + sr1LSB2; // add both and that will be the sr1 register
        //Check Bit 5 for REG or imm5
	int mask4 = 0x0020;
	int bit5 = (lsb & mask4) >>5;

		if(bit5 == 0){ // we have sr2
		int mask5 = 0x0007;
		int sr2 = (lsb & mask5);
		NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2]); //regs state modified
//		printf("\ndr: %d 0x%.4x", NEXT_LATCHES.REGS[dr], NEXT_LATCHES.REGS[dr] );
		//Now Check if DR Value is +ve or -ve
		int maskDR = 0x8000;
		int DRsign = (Low16bits(CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2]) & maskDR) >> 15;
//			printf("\n DR SIGN IS %d\n", DRsign);
			if(DRsign == 1){
//				printf("inside drsign =1 \n");
//				printf("dr value is %d 0x%.4x", NEXT_LATCHES.REGS[dr], NEXT_LATCHES.REGS[dr]);
		   		int add = (twosCompliment(NEXT_LATCHES.REGS[dr], 16));
//				printf("\nadd after sign extension is %d 0x%.4x\n", add, add);
		   		update_nzp(add);
		   
			}
			else {
//				printf("why am i here");
               			int add = NEXT_LATCHES.REGS[dr];
				if(add>0){
				NEXT_LATCHES.P = 1;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 0;
				}
				else if(add<0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 1;
				}
				else if (add==0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 1;
				NEXT_LATCHES.N = 0;			
				}
			}		
		}
		else{ // imm5 case 
		int mask6 = 0x001F;
		int imm5 = (mask6 & lsb);
		int signImm5 = imm5 >> 4;
                	if(signImm5 == 1){
			imm5 = twosCompliment(imm5, 5); 
			}
                NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + imm5); //update regs for immediate 5 case
	        int add = NEXT_LATCHES.REGS[dr];
		//check sign of DR	
		int maskDR = 0x8000;
		int DRsign = (Low16bits(CURRENT_LATCHES.REGS[sr1] + imm5) & maskDR) >> 15;
//		printf("Dr sign is %d 0x%0.4x", DRsign, DRsign);
			if(DRsign == 1){
		   		int add = (twosCompliment(NEXT_LATCHES.REGS[dr], 16));
		   		update_nzp(add);
		   
			}
			else {
			int add = NEXT_LATCHES.REGS[dr];
				if(add>0){
				NEXT_LATCHES.P = 1;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 0;
				}
				else if(add<0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 1;
				}
				else if (add==0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 1;
				NEXT_LATCHES.N = 0;			
				}
			}	
		} 
	NEXT_LATCHES.PC = CURRENT_LATCHES.PC +2;
	}
	else if(opcode ==5){ //and 
	//get DR
	int mask1 = 0x000e; //0000 111 000 0 00 000
	int dr = (msb & mask1)>>1;
        //get SR1
	int mask2 = 0x0001;
	int mask3 = 0x00C0;
	int sr1MSB = (msb & mask2)<<2; // left shift msb by 2 bits
	int sr1LSB2 = (lsb& mask3)>>6; // right shift lsb by 6 bits
        int sr1 = sr1MSB + sr1LSB2; // add both and that will be the sr1 register
        //Check Bit 5 for REG or imm5
	int mask4 = 0x0020;
	int bit5 = (lsb & mask4) >>5;

		if(bit5 == 0){ // we have sr2
		int mask5 = 0x0007;
		int sr2 = (lsb & mask5);
		NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2]); //regs state modified
//		printf("\nDR value is %d 0x%.4x", NEXT_LATCHES.REGS[dr], NEXT_LATCHES.REGS[dr]); 
        	//check for DR sign for nzp update
        	int DRmask = 0x8000;
		int DRsign =  (NEXT_LATCHES.REGS[dr] & DRmask) >> 15;
//		printf("\nDRsign is %d 0x%.4x", DRsign, DRsign);
		if( DRsign == 1){
//			printf("\ninside if dr is 1");
			int and = twosCompliment(NEXT_LATCHES.REGS[dr], 16);
//			printf("and value after compliment is %d", and);
			update_nzp(and);
		} 
		else {    
			int and = NEXT_LATCHES.REGS[dr];
				if(and>0){
				NEXT_LATCHES.P = 1;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 0;
				}
				else if(and<0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 1;
				}
				else if (and==0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 1;
				NEXT_LATCHES.N = 0;			
				}
			}				
		}
		else{ // imm5 case 
		int mask6 = 0x001F;
		int imm5 = (mask6 & lsb);
		int signImm5 = imm5 >> 4;
                	if(signImm5 == 1){
			imm5 = twosCompliment(imm5, 5); 
			}
                NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & imm5); //update regs for immediate 5 case
	        //check sign of DR to update nzp
	        int DRmask = 0x8000;
		int DRsign = (NEXT_LATCHES.REGS[dr] & DRmask)>>15;
		if(DRsign ==1){
			int and = twosCompliment(NEXT_LATCHES.REGS[dr], 16);
			update_nzp(and);
		}
		else { 
			int and = NEXT_LATCHES.REGS[dr];
				if(and>0){
				NEXT_LATCHES.P = 1;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 0;
				}
				else if(and<0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 1;
				}
				else if (and==0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 1;
				NEXT_LATCHES.N = 0;			
				}
		    }	
		} 
	NEXT_LATCHES.PC = CURRENT_LATCHES.PC +2;
	} 
	else if(opcode == 0){ //Branch Intructions or NOP
		if(msb == 0 && lsb ==0){ //NOP instruction
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;	 //Confirm with nikhil first
		}
		else {
		int maskMSBbit1 = 0x0001;
		int maskLSB = 0X00FF;
		//get pcoffset9
		int pcoffsetMSBbit9 = msb & maskMSBbit1;
	        int pcoffsetLSB = lsb & maskLSB;
	        int pcoffset9;
			if(pcoffsetMSBbit9==1){ // if pcoffset is negetive we have to sign extend
			pcoffset9 = twosCompliment(((pcoffsetMSBbit9 << 8) + pcoffsetLSB), 9);
			}
			else{ //calculate pc
			pcoffset9 = (pcoffsetMSBbit9 << 8) + pcoffsetLSB;	
			}
		//get nzp bits from instruction to test the condition
		int maskn = 0x0008;
		int maskz = 0x0004;
		int maskp = 0x0002;
		int nbit = (msb & maskn) >> 3;
		int zbit = (msb & maskz) >> 2;
		int pbit = (msb & maskp) >> 1;
			if((nbit & CURRENT_LATCHES.N) || (zbit & CURRENT_LATCHES.Z) || (pbit & CURRENT_LATCHES.P)){
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2 + (pcoffset9<<1);
			}
			else {
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2; //if branch is not met just increment the PC
			}
		}

	
	}
	else if(opcode == 4){ //JSR OR JSRR
	//NEXT_LATCHES.REGS[7] = CURRENT_LATCHES.PC +2; // confirm this with nikhil once
	int temp = CURRENT_LATCHES.PC +2;
	int maskbit11 = 0x0008; // get 11th bit 
	int bit8 = (msb & maskbit11) >> 3;
	int PCoffset11;
		if(bit8 == 1){ //JSR 
			int maskpcoffset11MSB = 0x0007;
			int maskpcoffset11SignBit = 0x0004;
			int signofPCoffset11 = (msb & maskpcoffset11SignBit) >> 2;
			if(signofPCoffset11 == 1){
				PCoffset11 = twosCompliment( (((msb & maskpcoffset11MSB) << 8) + lsb), 11 );		
			}
			else if(signofPCoffset11 ==0){
				PCoffset11 = (((msb & maskpcoffset11MSB) << 8) + lsb);
			}
			NEXT_LATCHES.PC = CURRENT_LATCHES.PC +2 + (PCoffset11 << 1);
    
		}
		else if(bit8 == 0){ //JSRR
			int maskbit8ofmsb = 0x0001;
			int maskbit76fromLSB = 0x00C0;
			int regbits = ((msb & maskbit8ofmsb)<<2) + ((lsb & maskbit76fromLSB)>>6);
			NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[regbits]; //confirm. rangeof PC is also important here.
		}
	NEXT_LATCHES.REGS[7] = 	temp;	
	}
	else if(opcode ==9){ //XOR & NOT
	//get DR
	int mask1 = 0x000e; //0000 111 000 0 00 000
	int dr = (msb & mask1)>>1;
        //get SR1
	int mask2 = 0x0001;
	int mask3 = 0x00C0;
	int sr1MSB = (msb & mask2)<<2; // left shift msb by 2 bits
	int sr1LSB2 = (lsb& mask3)>>6; // right shift lsb by 6 bits
        int sr1 = sr1MSB + sr1LSB2; // add both and that will be the sr1 register
        //Check Bit 5 for REG or imm5
	int mask4 = 0x0020;
	int bit5 = (lsb & mask4) >>5;

		if(bit5 == 0){ // we have sr2
		int mask5 = 0x0007;
		int sr2 = (lsb & mask5);
		NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ CURRENT_LATCHES.REGS[sr2]); //regs state modified
               	int DRmask = 0x8000;
		int DRsign = ((NEXT_LATCHES.REGS[dr] & DRmask) >> 15);
		if(DRsign == 1){
			int xor = twosCompliment(NEXT_LATCHES.REGS[dr],16);
			update_nzp(xor);
		}
		else {
			int xor = NEXT_LATCHES.REGS[dr];
				if(xor>0){
				NEXT_LATCHES.P = 1;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 0;
				}
				else if(xor<0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 1;
				}
				else if (xor==0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 1;
				NEXT_LATCHES.N = 0;			
			}
		    }		
		}
		else{ // imm5 case and NOT will become part of this 
		int mask6 = 0x001F;
		int imm5 = (mask6 & lsb);
		int signImm5 = imm5 >> 4;
                	if(signImm5 == 1){
			imm5 = twosCompliment(imm5, 5); 
			}
                NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ imm5); //update regs for immediate 5 case
	        int DRmask = 0x8000;
		int DRsign = ((NEXT_LATCHES.REGS[dr] & DRmask) >> 15);
		if(DRsign == 1){
			int xor = twosCompliment(NEXT_LATCHES.REGS[dr],16);
			update_nzp(xor);
		}
		else {
			int xor = NEXT_LATCHES.REGS[dr];
				if(xor>0){
				NEXT_LATCHES.P = 1;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 0;
				}
				else if(xor<0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 0;
				NEXT_LATCHES.N = 1;
				}
				else if (xor==0){
				NEXT_LATCHES.P = 0;
				NEXT_LATCHES.Z = 1;
				NEXT_LATCHES.N = 0;			
			}
	            }			
		} 
	NEXT_LATCHES.PC = CURRENT_LATCHES.PC +2;
	}

	    if(opcode == 2){ //ldb
        int mask = 0x000E; //00001110
        int dr = (msb & mask) >> 1;
//	printf("DR: %d 0x%.4x ", dr);

        mask = 0x0001; //00000001
        int mask2 = 0x00c0; //11000000
        int baseRmsb = msb & mask;
        int baseRlsb2 = (lsb & mask2) >> 6;
        int baseR = baseRmsb*pow(2,2) + baseRlsb2; // douible check
//	printf("baseR: %d 0x%.4x ", baseR);

        mask = 0x003f; //00111111
        int boffset = lsb & mask;
//	printf("boffset: %d 0x%.4x ", boffset);


        int msbmask = 0x0020; //00100000
        if(((boffset & msbmask)>>5) == 1) //if negative sign extend boffset here
            boffset = twosCompliment(boffset, 6);
            
        int res = MEMORY[(CURRENT_LATCHES.REGS[baseR] +  boffset)/2][(CURRENT_LATCHES.REGS[baseR] +  boffset)%2]; //verify the correctness

        msbmask = 0x0080; //10000000
        if(((res & msbmask)>>7) == 1) //if negative sign extend boffset here
            res = twosCompliment(res, 8);

        NEXT_LATCHES.REGS[dr] = Low16bits(res);// = Low16bits(res);??

        update_nzp(res);

        NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;

    }

    if(opcode == 3){ //stb
//	printf("we are in STB");
        int mask = 0x000E; //00001110
        int sr = (msb & mask) >> 1;

        mask = 0x0001; //00000001
        int mask2 = 0x00c0; //11000000
        int baseRmsb = msb & mask;
        int baseRlsb2 = (lsb & mask2) >> 6;
        int baseR = baseRmsb*pow(2,2) + baseRlsb2;

        mask = 0x003f; //00111111
        int boffset = lsb & mask;

        int msbmask = 0x0020; //00100000
        if(((boffset & msbmask)>>5) == 1) //if negative sign extend boffset here
            boffset = twosCompliment(boffset, 6);

        MEMORY[(CURRENT_LATCHES.REGS[baseR] +  boffset)/2][(CURRENT_LATCHES.REGS[baseR] +  boffset)%2] = CURRENT_LATCHES.REGS[sr] & 0xff; // verify correctness

        NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;

    }

    if(opcode == 6){ //ldw
        int mask = 0x000E; //00001110
        int dr = (msb & mask) >> 1;

//	printf("\n");
//	printf("dr: %d 0x%0.1x\n", dr, dr);

        mask = 0x0001; //00000001
        int mask2 = 0x00c0; //11000000
        int baseRmsb = msb & mask;
        int baseRlsb2 = (lsb & mask2) >> 6;
        int baseR = baseRmsb*pow(2,2) + baseRlsb2;

//	printf("\n");
//	printf("baseR: %d 0x%0.1x\n", baseR, baseR);

        mask = 0x003f; //00111111
        int offset = lsb & mask;

        int msbmask = 0x0020; //00100000
        if(((offset & msbmask)>>5) == 1) //if negative sign extend offset here
            offset = twosCompliment(offset, 6);
        offset = offset << 1;

//	printf("\n");
//	printf("offset(after left shift 1): %d 0x%0.4x\n", offset, offset);
            
        int res = (MEMORY[(CURRENT_LATCHES.REGS[baseR] +  offset)/2][1]<<8) +
                   MEMORY[(CURRENT_LATCHES.REGS[baseR] +  offset)/2][0]; //verify the correctness

//	printf("\n");
//	printf("mem msb: %d 0x%0.8x\n", MEMORY[(CURRENT_LATCHES.REGS[baseR] +  offset)/2][1], MEMORY[(CURRENT_LATCHES.REGS[baseR] +  offset)/2][1]);

//	printf("\n");
//	printf("mem lsb %d 0x%0.8x\n", MEMORY[(CURRENT_LATCHES.REGS[baseR] +  offset)/2][0], MEMORY[(CURRENT_LATCHES.REGS[baseR] +  offset)/2][0]);

//	printf("\n");
//	printf("mem msb: %d 0x%0.8x\n", res, res);


        NEXT_LATCHES.REGS[dr] = Low16bits(res);// = Low16bits(res);??
	//check sign of DR	
	int DRmask = 0x8000;
	int DRsign = (NEXT_LATCHES.REGS[dr] & DRmask)>>15;
	if(DRsign == 1)
	{
		int wordCompli = twosCompliment(NEXT_LATCHES.REGS[dr],16);
		update_nzp(wordCompli);
	}
	else {
	
        update_nzp(res);
	}
        NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;

    }

    if(opcode == 7){ //stw
//	printf("We are in STW\n");
        int mask = 0x000E; //00001110
        int sr = (msb & mask) >> 1;

//	printf("\n");
//	printf("sr: %d 0x%0.8x\n", sr, sr);

        mask = 0x0001; //00000001
        int mask2 = 0x00c0; //11000000
        int baseRmsb = msb & mask;
        int baseRlsb2 = (lsb & mask2) >> 6;
        int baseR = baseRmsb*pow(2,2) + baseRlsb2;

//	printf("\n");
//	printf("baseR: %d 0x%0.8x\n", baseR, baseR);


        mask = 0x003f; //00111111
        int offset = lsb & mask;

        int msbmask = 0x0020; //00100000
        if(((offset & msbmask)>>5) == 1) //if negative sign extend offset here
            offset = twosCompliment(offset, 6);
        offset = offset << 1;

//	printf("\n");
//	printf("offset(after left shift 1): %d 0x%0.8x\n", offset, offset);

//printf("\n");
//	printf("(CURRENT_LATCHES.REGS[baseR] +  offset): %d 0x%0.8x\n", (CURRENT_LATCHES.REGS[baseR] +  offset), (CURRENT_LATCHES.REGS[baseR] +  offset));

//printf("\n");
//	printf("(CURRENT_LATCHES.REGS[sr]&0xff00)>>8: %d 0x%0.8x\n", (CURRENT_LATCHES.REGS[sr]&0xff00)>>8, (CURRENT_LATCHES.REGS[sr]&0xff00)>>8);

//printf("\n");
//	printf("CURRENT_LATCHES.REGS[sr]&0x00ff: %d 0x%0.8x\n", CURRENT_LATCHES.REGS[sr]&0x00ff, CURRENT_LATCHES.REGS[sr]&0x00ff);


        MEMORY[(CURRENT_LATCHES.REGS[baseR] +  offset)/2][1] = (CURRENT_LATCHES.REGS[sr]&0xff00)>>8; // verify correctness
        MEMORY[(CURRENT_LATCHES.REGS[baseR] +  offset)/2][0] = CURRENT_LATCHES.REGS[sr]&0x00ff; // verify correctness

        NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;

    }

    if(opcode == 12){ //jmp/ret
        int mask = 0x0001; //00000001
        int mask2 = 0x00c0; //11000000
        int baseRmsb = msb & mask;
        int baseRlsb2 = (lsb & mask2) >> 6;
        int baseR = baseRmsb*pow(2,2) + baseRlsb2;

        NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[baseR]; // verify correctness LOWbits??

    }

    if(opcode == 13){ //shf
//	printf("We are in SHF\n");

        int mask = 0x000E; //00001110
        int dr = (msb & mask) >> 1;

//printf("\n");
//	printf("dr: %d 0x%0.1x\n", dr, dr);


        mask = 0x0001; //00000001
        int mask2 = 0x00c0; //11000000
        int srmsb = msb & mask;
        int srlsb2 = (lsb & mask2) >> 6;
        int sr = srmsb*pow(2,2) + srlsb2;

//	printf("\n");
//	printf("sr: %d 0x%0.1x\n", sr, sr);


        mask = 0x000f; //00001111
        int amount4 = lsb & mask;

//	printf("\n");
//	printf("amount4: %d 0x%0.4x\n", amount4, amount4);

//	printf("\n");
//	printf("CURRENT_LATCHES.REGS[sr]: %d 0x%0.4x\n", CURRENT_LATCHES.REGS[sr], CURRENT_LATCHES.REGS[sr]);

//printf("\n");
//	printf("CURRENT_LATCHES.REGS[sr] << amount4: %d 0x%.8x\n", CURRENT_LATCHES.REGS[sr] << amount4, CURRENT_LATCHES.REGS[sr] << amount4);

//printf("\n");
//	printf("CURRENT_LATCHES.REGS[sr] << amount4: %d 0x%.8x\n", Low16bits(CURRENT_LATCHES.REGS[sr] << amount4), Low16bits(CURRENT_LATCHES.REGS[sr] << amount4));


        mask = 0x0010; //00010000
//printf("lsb:%d 0x%.8x lsb & mask: %d 0x%.8x\n",lsb,lsb, lsb & mask, lsb & mask);

        if(((lsb & mask) >> 4) == 0){
            NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr] << amount4); //lowbits??
//		printf("are we even going here??\n");
	}
        else{
//	printf("why are we here?\n");
            mask = 0x0020; //00100000
            if(((lsb & mask) >> 5) == 0)
                NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr] >> amount4); //lowbits??
            else{
                int sr15 = (CURRENT_LATCHES.REGS[sr] & 0x8000)>>15;//1000000000000000
                NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr] >> amount4); //check for correctness //lowbits??
                if(sr15 == 1) //if negative sign extend offset here
                    NEXT_LATCHES.REGS[dr] = Low16bits(twosCompliment(NEXT_LATCHES.REGS[dr], 16 - amount4));//lowbits??
            }
//	printf("\n");
//	printf("CURRENT_LATCHES.REGS[sr] >> amount4: %d 0x%.8x\n", Low16bits(CURRENT_LATCHES.REGS[sr] >> amount4), Low16bits(CURRENT_LATCHES.REGS[sr] >> amount4));

//printf("\n");
//	printf("NEXT_LATCHES.REGS[dr]: %d 0x%.8x\n", NEXT_LATCHES.REGS[dr], NEXT_LATCHES.REGS[dr]);

        }
	//check DR sign
	int DRmask = 0x8000;
	int DRsign = (NEXT_LATCHES.REGS[dr] & DRmask) >> 15;
	if(DRsign == 1){
		int next_latch_dr = twosCompliment(NEXT_LATCHES.REGS[dr], 16);
		update_nzp(next_latch_dr);
	}
	else {
        update_nzp(NEXT_LATCHES.REGS[dr]);
	}

       NEXT_LATCHES.PC = CURRENT_LATCHES.PC +2; // verify correctness //lowbits??
    }

    if(opcode == 14){ //lea
        int mask = 0x000E; //00001110
        int dr = (msb & mask) >> 1;

        mask = 0x0001; //00000001
        int pcoffset9 = (msb & mask)*pow(2,8) + lsb; //check correctness
//	printf("%d 0x%.8x\n", pcoffset9, pcoffset9);
        int msbmask = 0x0100; //000100000000
        if(((pcoffset9 & msbmask)>>8) == 1) //if negative sign extend boffset here
            pcoffset9 = twosCompliment(pcoffset9, 9);
//	printf("%d 0x%.8x\n", pcoffset9, pcoffset9);

        pcoffset9 = pcoffset9 << 1;
//	printf("%d 0x%.8x\n", pcoffset9, pcoffset9);

//	printf("current PC: %d 0x%.8x\n", CURRENT_LATCHES.PC, CURRENT_LATCHES.PC);

        NEXT_LATCHES.REGS[dr] = CURRENT_LATCHES.PC + 2 + pcoffset9;//low16bit??

//	printf("current PC + 2: %d 0x%.8x\n", CURRENT_LATCHES.PC + 2, CURRENT_LATCHES.PC + 2);


//	printf("NEXT_LATCHES.REGS[dr] : %d 0x%.8x\n", NEXT_LATCHES.REGS[dr] , NEXT_LATCHES.REGS[dr] );

        NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
    }

    if(opcode == 15){ //trap
        int res = (MEMORY[(lsb << 1)/2][1]<<8) + MEMORY[(lsb << 1)/2][0]; // correctness check?

        NEXT_LATCHES.REGS[7] = CURRENT_LATCHES.PC + 2; //lowbits??
        
        NEXT_LATCHES.PC = res; //lowbits??
    }
    return;
}

void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     




//printf("%.2x%.2x\n%d%d \n", MEMORY[(CURRENT_LATCHES.PC)/2][1], MEMORY[(CURRENT_LATCHES.PC)/2][0], MEMORY[(CURRENT_LATCHES.PC)/2][1], MEMORY[(CURRENT_LATCHES.PC)/2][0]);
//NEXT_LATCHES.PC += 2;

//Fetch Process 
// Fetch MSB and LSB and perform int2bin
short* binary;
//short decode[16];

//printf("%d", MEMORY[(CURRENT_LATCHES.PC)/2][1]);
//printf("%d\n", MEMORY[(CURRENT_LATCHES.PC)/2][0]);
 
// Method 1 for getting optcodes.
//binary = int2bin(MEMORY[(CURRENT_LATCHES.PC)/2][0], MEMORY[(CURRENT_LATCHES.PC)/2][1]);
//printf("PC: %d, ", CURRENT_LATCHES.PC);
  int msb = MEMORY[CURRENT_LATCHES.PC/2][1];
 // printf("MSB = %d 0x%.2x ", msb, msb); 
  int lsb = MEMORY[CURRENT_LATCHES.PC/2][0];

  int mask = 0x00f0;
  int opcode = (msb & mask) >> 4;
  //printf("Int opcode: %d\n", opcode);
  decode(opcode, msb, lsb);
}