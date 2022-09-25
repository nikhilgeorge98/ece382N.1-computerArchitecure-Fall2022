/*
	Name 1: Nikhil George 
	Name 2: Subramanya Mohit Kashyap Indumukhi 
	UTEID 1: ng25762 
	UTEID 2: mi5672
*/

#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <math.h>

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define Low16bits(x) ((x) & 0xFFFF)
//structure for label symbol table
typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1]; /* Question for the reader: Why do we need to add 1? */
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];
// TableEntry symbolTable[5] = {{12298, "test"}, {12312, "getchar"}, {12318, "output"}, {12328, "ascii"}, {12330, "ptr"}};


//structure for register table
typedef struct {
    char *regi;
    short bin[3];
} RegisterValue;
RegisterValue regTable[8] = {{"R0", {0,0,0}}, {"R1", {0,0,1}}, {"R2", {0,1,0}}, {"R3", {0,1,1}},
                            {"R4", {1,0,0}}, {"R5", {1,0,1}}, {"R6", {1,1,0}}, {"R7", {1,1,1}}};

// define return labels as int numbers 
enum
{
    DONE, OK, EMPTY_LINE
};

FILE* infile = NULL;
FILE* outfile = NULL;

int bin2int(short * bin){
    int i;
    int sum = 0;
    for(i = 0; i<16; i++){
        //printf("%d", bin[i]);
        sum+=bin[i]*pow(2,15-i);
        //printf("i:%d sum:%d", i, sum);
    }
    return sum;
}

//look for valid label addresses
int lookupLabelAddress(char * lArg){
    int i;
    for(i = 0; i<255; i++){
        if(strcmp(symbolTable[i].label, lArg) == 0)
            // labelAddress = symbolTable[i].address;
            return symbolTable[i].address;
    }
    printf("Label not found in lookup table");
    return -1;
}

//perform twos compliment 
short* twosCompliment(short * posbinary)
{
    int i;
    short * origBinary;
    origBinary = posbinary;
    for(i=0; i<16;i++)
    {
        if(posbinary[i] == 0)
        {
            posbinary[i] = 1;
        }
        else
        {
            posbinary[i] = 0;
        }
        //printf("%d", posbinary[i]);
    }
    //printf("\n");
    //printf("%d %d \n", posbinary[0], posbinary[15]);
    posbinary[15] = posbinary[15] + 1;
    //printf("%d\n", posbinary[15]);
    for(i=16;i>0;i--)
    {  
        //printf("%d\n", posbinary[i]);
        if(posbinary[i] > 1)
        {
            posbinary[i] = posbinary[i]%2;
            //printf("%d", posbinary[i]);
            posbinary[i-1] = posbinary[i-1] + 1;
        }
       
    }
    return(posbinary);
   
}

//convert integer to binary
short* int2bin(int num){
    short revbin[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static short bin[16];
    int i = 0;
    //printf("%d", num);
    if(num>=0){
        while(num>0)
        {  
            //printf("Entered the loop");
            revbin[i] = num%2;
            //printf("num:%d i:%d bin[i]:%d\n",num, i, revbin[i]);
            num = num/2;
            i++;
        }
        // int i, temp;
        int len = sizeof(revbin)/sizeof(revbin[0]);
        //printf("\n%d\n", len);
        for(i=0; i<16; i++){
           // printf("%d ",revbin[i]);
            bin[15-i] = revbin[i];
            //printf("%d\n", bin[15-i]);
        }
    }
    else if(num<0)
    {
        i = 0;
        //printf("inside if loop");
        int sign = -1;
        num = abs(num);
        //printf("\n%d\n", num);
        while(num>0)
        {
         //   printf("inside while loop \n");
            revbin[i] = num%2;
            //printf("num:%d i:%d bin[i]:%d\n",num, i, revbin[i]);
            num = num/2;
            i++;
        }
        // int i, temp;
        int len = sizeof(revbin)/sizeof(revbin[0]);
        //printf("%d\n", len);
        for(i=0; i<16; i++){
        //printf("%d ",revbin[i]);
        bin[15-i] = revbin[i];
        //printf("%d\n", bin[15-i]);
        }
        short * twosCompliBin = twosCompliment(bin);
        *bin = *twosCompliBin;
       
    }
    return bin;
}

//register lookup table
short* regLookup(char regNo){
    static short bin[3];
    switch(regNo){
        case '0':
            bin[0] = 0;
            bin[1] = 0;
            bin[2] = 0;
            break;
        case '1':
            bin[0] = 0;
            bin[1] = 0;
            bin[2] = 1;
            break;
        case '2':
            bin[0] = 0;
            bin[1] = 1;
            bin[2] = 0;
            break;
        case '3':
            bin[0] = 0;
            bin[1] = 1;
            bin[2] = 1;
            break;
        case '4':
            bin[0] = 1;
            bin[1] = 0;
            bin[2] = 0;
            break;
        case '5':
            bin[0] = 1;
            bin[1] = 0;
            bin[2] = 1;
            break;
        case '6':
            bin[0] = 1;
            bin[1] = 1;
            bin[2] = 0;
            break;
        case '7':
            bin[0] = 1;
            bin[1] = 1;
            bin[2] = 1;
            break;
    }
    return bin;
}

//is the label valid? Does it start with x or 0
int isValidLabel(char * lLabel)
{
printf("we are in isvalid function");    
if(lLabel[0]=='x' || isdigit(lLabel[0]))
        return -1;
    int i;
    for(i=0; i<strlen(lLabel); i++)
    {
        if(isalnum(lLabel[i])==0)
            return -1;
    }
    if(strcmp("in", lLabel) == 0 || strcmp("getc", lLabel) == 0 || strcmp("out", lLabel) == 0 || strcmp("puts", lLabel) == 0)
        return -1;
    if(strlen(lLabel)>12)
        return -1;
    return 1;
}

//is the opcode a valid one?
int isOpcode(char * lPtr)
{
    char* valid_opcodes[] = {"add", "and", "brn", "brz", "brp", "brnz", "brzp", "brnp", "brnzp", "br",
                "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa",
                "rti", "stb", "stw", "trap", "xor"};
    int i;
    int valid = 0;
    for(i = 0; i < 28; i++)
    {
        if(strcmp(lPtr, valid_opcodes[i]) == 0)
        {
            return 1;
        }
    }
    return -1;
}

//convert string to number 
int toNum(char * pStr)
{
    char * t_ptr;
    char * orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;
    orig_pStr = pStr;
    if( *pStr == '#' ) /* decimal */
    {
        pStr++;
        if( *pStr == '-' ) /* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isdigit(*t_ptr))
            {
                printf("Error: invalid decimal operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;
        return lNum;
    }
    else if( *pStr == 'x' ) /* hex */
    {
        pStr++;
        if( *pStr == '-' ) /* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isxdigit(*t_ptr))
            {
                printf("Error: invalid hex operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16); /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
        if( lNeg )
            lNum = -lNum;   
        return lNum;
    }
    else
    {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4); /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

//read and parse the assembley program into different variables
int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4)
{
    char * lRet, * lPtr;
    int i;
    if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
        return( DONE );
    for( i = 0; i < strlen( pLine ); i++ )
        pLine[i] = tolower( pLine[i] );
        /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);
    /* ignore the comments */
    lPtr = pLine;
    
    while( *lPtr != ';' && *lPtr != '\0' && *lPtr != '\n' )
        lPtr++;
    *lPtr = '\0';
    
    if( !(lPtr = strtok( pLine, "\t\n ," ) ) )
        return( EMPTY_LINE );
    
    if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
    {
        *pLabel = lPtr;
        if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) 
            return( OK );
    }
    
    *pOpcode = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    *pArg1 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    *pArg2 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    *pArg3 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    *pArg4 = lPtr;
    return( OK );
}


short * toBin(char * lLabel, char * lOpcode, char * lArg1, char * lArg2, char * lArg3, char * lArg4, int PC) //function returns 16-bit binary encoding of a single assembly statement
{
    printf("lLabel:%s lOpcode:%s lArg1:%s lArg2:%s lArg3:%s lArg4:%s PC:%d\n", lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4, PC);
    // char *bin = {};
    static short bin[16];
    short *arg1bin;
    short *arg2bin;
    short *arg3bin;
    //short *arg4bin;
    short *immbin;
    int labelAddress;
    short *PCoffset;

    if(strcmp(lOpcode, "add") == 0 || strcmp(lOpcode, "and") == 0){ // add or and
        if(strcmp(lOpcode, "add") == 0){ // add
            bin[0] = bin[1] = bin[2] = 0; 
            bin[3] = 1;
        }
        else{                        // and
            bin[0] = bin[2] = 0; 
            bin[1] = bin[3] = 1;
        }
        
        arg1bin = regLookup(lArg1[1]);
        bin[4] = arg1bin[0];
        bin[5] = arg1bin[1];
        bin[6] = arg1bin[2];
        
        arg2bin = regLookup(lArg2[1]);
        bin[7] = arg2bin[0];
        bin[8] = arg2bin[1];
        bin[9] = arg2bin[2];
        
        if(lArg3[0]=='r'){                          //register
            bin[10] = bin[11] = bin[12] = 0;
            arg3bin = regLookup(lArg3[1]);
            bin[13] = arg3bin[0];
            bin[14] = arg3bin[1];
            bin[15] = arg3bin[2];
        }
        else if(lArg3[0]=='x' || lArg3[0]=='#'){    //imm5
            //printf("We are here\n");
            bin[10] = 1;
            int num;
            num = toNum(lArg3);
	//if(num > 15 || num < -16){
          //  printf("Error: Instruction is too far\n");
            //exit(4);
	    //} 
            immbin = int2bin(num);
            //printf("Do we reach here?");
            //printf("%d",immbin[15]);
            int i;
            for(i=11;i<16;i++)
                bin[i]=immbin[i];
        }
        // else
        //     exit(4);
    }
    else if(strcmp(lOpcode, "jmp") == 0 || strcmp(lOpcode, "ret") == 0){ //jmp or ret
        bin[0] = bin[1] = 1; 
        bin[2] = bin[3] = 0;

        bin[4] = bin[5] = bin[6] = 0;

        bin[10] = bin[11] = bin[12] = bin[13] = bin[14] = bin[15] = 0;
        if(strcmp(lOpcode, "jmp") == 0){                            //jmp
            arg1bin = regLookup(lArg1[1]);
            bin[7] = arg1bin[0];
            bin[8] = arg1bin[1];
            bin[9] = arg1bin[2];
        }
        else{                                                   //ret
            bin[7] = bin[8] = bin[9] = 1;
        }
    }
    else if(strcmp(lOpcode, "lshf") == 0 || strcmp(lOpcode, "rshfl") == 0 || strcmp(lOpcode, "rshfa") == 0){ //lshf or rshfl or rshfa
        bin[0] = bin[1] = bin[3] = 1; 
        bin[2] = 0;
        
        arg1bin = regLookup(lArg1[1]);
        bin[4] = arg1bin[0];
        bin[5] = arg1bin[1];
        bin[6] = arg1bin[2];
        
        arg2bin = regLookup(lArg2[1]);
        bin[7] = arg2bin[0];
        bin[8] = arg2bin[1];
        bin[9] = arg2bin[2];

        //amount4
        int num;
        num = toNum(lArg3);
//	if(num > 15 || num < 0){
  //          printf("Error: Instruction is too far\n");
    //        exit(4);
//	    } 
        immbin = int2bin(num);
        int i;
        for(i=12;i<16;i++)
            bin[i]=immbin[i];
        
        if(strcmp(lOpcode, "lshf") == 0){           //lshf
            bin[10] = bin[11] = 0;
        }
        else if(strcmp(lOpcode, "rshfl") == 0){     //rshfl
            bin[10] = 0; bin[11] = 1;
        }
        else{                                   //rshfa
            bin[10] = bin[11] = 1;
        }
    }
    else if(strcmp(lOpcode, "xor") == 0 || strcmp(lOpcode, "not") == 0){ // xor or not
        bin[1] = bin[2] = 0; 
        bin[0] = bin[3] = 1;
        
        arg1bin = regLookup(lArg1[1]);
        bin[4] = arg1bin[0];
        bin[5] = arg1bin[1];
        bin[6] = arg1bin[2];
        
        arg2bin = regLookup(lArg2[1]);
        bin[7] = arg2bin[0];
        bin[8] = arg2bin[1];
        bin[9] = arg2bin[2];
        
        if(lArg3[0]=='r'){                                      // xor register
            bin[10] = bin[11] = bin[12] = 0;
            arg3bin = regLookup(lArg3[1]);
            bin[13] = arg3bin[0];
            bin[14] = arg3bin[1];
            bin[15] = arg3bin[2];
        }
        else if(lArg3[0]=='x' || lArg3[0]=='#'){                // xor imm5
            bin[10] = 1;
            int num;
//	if(num > 15 || num < -16){
  //          printf("Error: Instruction is too far\n");
    //        exit(4);
//	    } 
            num = toNum(lArg3);
            immbin = int2bin(num);
            int i;
            for(i=11;i<16;i++)
                bin[i]=immbin[i];
        }
        else if(strcmp(lOpcode, "not") == 0){                       // not
            bin[10] = bin[11] = bin[12] = bin[13] = bin[14] = bin[15] = 1;
        }
        // else
        //     exit(4);
    }
    else if(strcmp(lOpcode, "rti") == 0){                           // rti
        bin[0] = 1; 
        bin[1] = bin[2] = bin[3] = 0;
        int i;
        for(i = 4; i<16; i++)
            bin[i] = 0;
    }
    else if(strcmp(lOpcode, "ldb") == 0 || strcmp(lOpcode, "stb") == 0 || strcmp(lOpcode, "ldw") == 0 || strcmp(lOpcode, "stw") == 0){ // ldb or stb or ldw or stw
        if(strcmp(lOpcode, "ldb") == 0){                            // ldb
            bin[0] = bin[1] = bin[3] = 0; 
            bin[2] = 1;
        }
        else if(strcmp(lOpcode, "stb") == 0){                       // stb
            bin[0] = bin[1] = 0; 
            bin[2] = bin[3] = 1;
        }
        else if(strcmp(lOpcode, "ldw") == 0){                       // ldw
            bin[0] = bin[3] = 0; 
            bin[1] = bin[2] = 1;
        }
        else if(strcmp(lOpcode, "stw") == 0){                       // stw
            bin[0] = 0;  
            bin[1] = bin[2] = bin[3] = 1;
        }
        
        arg1bin = regLookup(lArg1[1]);
        bin[4] = arg1bin[0];
        bin[5] = arg1bin[1];
        bin[6] = arg1bin[2];
        
        arg2bin = regLookup(lArg2[1]);
        bin[7] = arg2bin[0];
        bin[8] = arg2bin[1];
        bin[9] = arg2bin[2];
        
        //boffset6 or offset6 (from examples i assume it only gives decimal or hex value. Need to confirm if this is true)
        int num;
        num = toNum(lArg3);
//	if(num > 31 || num < -32){
  //          printf("Error: Instruction is too far\n");
    //        exit(4);
//	    } 
        immbin = int2bin(num);
        int i;
        for(i=10;i<16;i++)
            bin[i]=immbin[i];
        // else
        //     exit(4);
    }
    else if(lOpcode[0]=='b' && lOpcode[1]=='r'){  // BR
        bin[0] = bin[1] = bin[2] = bin[3] = 0;

        labelAddress = lookupLabelAddress(lArg1);               // bits 7-15 = PCoffset9
   PC += 2;
    //    if((labelAddress - PC)/2 > 256 || (labelAddress - PC)/2 < -255){
      //      printf("Error: Instruction is too far\n");
        //    exit(4);
  //      }
        PCoffset = int2bin((labelAddress - PC)/2);
        int i;
        for(i = 7; i<16; i++)
            bin[i]=PCoffset[i];

        if(strcmp(lOpcode, "brn") == 0){                            // brn
            bin[4] = 1; 
            bin[5] = bin[6] = 0;
        }
        else if(strcmp(lOpcode, "brz") == 0){                       // brz
            bin[5] = 1; 
            bin[4] = bin[6] = 0;
        }
        else if(strcmp(lOpcode, "brp") == 0){                       // brz
            bin[6] = 1; 
            bin[4] = bin[5] = 0;
        }
        else if(strcmp(lOpcode, "brnz") == 0){                       // brnz
            bin[4] = bin[5] = 1;  
            bin[6] = 0;
        }
        else if(strcmp(lOpcode, "brnp") == 0){                       // brnp
            bin[4] = bin[6] = 1;  
            bin[5] = 0;
        }
        else if(strcmp(lOpcode, "brzp") == 0){                       // brnp
            bin[5] = bin[6] = 1;  
            bin[4] = 0;
        }
        else{                                                    // brnzp or br
            bin[4] = bin[5] = bin[6] = 1;  
        }
    }
    else if(strcmp(lOpcode, "jsr") == 0 || strcmp(lOpcode, "jsrr") == 0){                           // jsr or jsrr
        bin[0] = bin[2] = bin[3] = 0; 
        bin[1] = 1;
        
        if(strcmp(lOpcode, "jsr") == 0){                            //jsr
            bin[4] = 1;
            
            labelAddress = lookupLabelAddress(lArg1);           // bits 5-15 = PCoffset11
            PC += 2;
//	   if((labelAddress - PC)/2 > 1024 || (labelAddress - PC)/2 < -1023){
  //          printf("Error: Instruction is too far\n");
    //        exit(4);
//	    } 
            PCoffset = int2bin((labelAddress - PC)/2);
            int i;
            for(i = 5; i<16; i++)
                bin[i]=PCoffset[i];
        }
        else{                                                   //jsrr
            bin[4] = 0;

            bin[5] = bin[6] = 0;

            int i;
            for(i=10;i<16;i++)
                bin[i] = 0;

            arg1bin = regLookup(lArg1[1]);
            bin[7] = arg1bin[0];
            bin[8] = arg1bin[1];
            bin[9] = arg1bin[2];
        }
        
    }
    else if(strcmp(lOpcode, "lea") == 0){                        // lea
        bin[3] = 0; 
        bin[0] = bin[1] = bin[2] = 1;
        
        labelAddress = lookupLabelAddress(lArg2);           // bits 7-15 = PCoffset9
        PC += 2;

  //      if((labelAddress - PC)/2 > 256 || (labelAddress - PC)/2 < -255){
    //        printf("Error: Instruction is too far\n");
      //      exit(4);
       // }

        PCoffset = int2bin((labelAddress - PC)/2);
        int i;
        for(i = 7; i<16; i++)
            bin[i]=PCoffset[i];
        
        arg1bin = regLookup(lArg1[1]);
        bin[4] = arg1bin[0];
        bin[5] = arg1bin[1];
        bin[6] = arg1bin[2];        
    }
    else if(strcmp(lOpcode, "trap") == 0){                       // trap
        bin[0] = bin[1] = bin[2] = bin[3] = 1;

        bin[4] = bin[5] = bin[6] = bin[7] = 0;
        
        int num;
        num = toNum(lArg1);
        immbin = int2bin(num);
        int i;
        for(i=8;i<16;i++)
            bin[i]=immbin[i];      
    }
 
    return bin;
}


void funcPass(char* readFile, char* writeFile, int pass)
{
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
    int lRet;
    int PC = -1;
    int labelCount = -1;
    short *bin;
    FILE * lInfile;
    lInfile = fopen( readFile, "r" ); /* open the input file */
    FILE * pOutfile;
    pOutfile = fopen( writeFile, "w" );
    do
    {
        lRet = readAndParse( lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
            if(pass == 1){
                printf("%s\n", lOpcode);
   		if(strcmp(lOpcode, ".orig") == 0){
		//printf("we are here before .orig is read\n");
                    PC = toNum(lArg1);
		//printf("now we are here pc has taken larg1\n");
               }
		printf("%d\n", PC);
		printf("%s\n", lLabel);
		if(strlen(lLabel) != 0){
                printf("we should not be here");	
                if(isValidLabel(lLabel))
                {
			printf("WE ARE IN ISVALID LABEL");
                    labelCount++;
                    symbolTable[labelCount].address = PC;
                    strcpy(symbolTable[labelCount].label, lLabel);
                }
                else
                {
                    printf("Invalid label\n");
                    exit(4);
                }
		}
                    
                if(PC!=-1)
                    PC += 2;
            }

            else if(pass == 2){
                int lInstr;
                if(strcmp(lOpcode, ".orig") == 0)
                    PC = toNum(lArg1);
                
                if(strcmp(lOpcode, "halt") == 0){
                    bin = toBin(lLabel, "trap", "x25", NULL, NULL, NULL, PC);
                    lInstr = bin2int(bin);
                }
                else if(strcmp(lOpcode, "nop") == 0){
                    lInstr = 0;
                    //fprintf( pOutfile, "0x%.4X\n", lInstr );
                    //continue;
                }
                else if(lOpcode[0]!='.'){
                    bin = toBin(lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4, PC);
                    lInstr = bin2int(bin);
                }
                else if(strcmp(lOpcode, ".orig") == 0 || strcmp(lOpcode, ".fill") == 0 ){
                    lInstr = toNum(lArg1);
                }
                else if(strcmp(lOpcode, ".end") == 0)
                    break;
                    
                fprintf( pOutfile, "0x%.4X\n", Low16bits(lInstr));
                
                if(PC!=-1)
                    PC += 2;
            }
            
        }
        
    } while( lRet != DONE );
    fclose(lInfile);
    fclose(pOutfile);
}


int main(int argc, char* argv[]) {

    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;
    
    
    prgName = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

/* open the source file */
    infile = fopen(argv[1], "r"); 
    outfile = fopen(argv[2], "w");
    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }
    /* Do stuff with files */
    fclose(infile);
    fclose(outfile);

    funcPass(iFileName, oFileName, 1);
    funcPass(iFileName, oFileName, 2);

    return 0;

}
