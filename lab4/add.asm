        .ORIG X3000

        LEA R0, ADDR    ;9
        LDW R0, R0, #0  ;24
        LEA R1, NUM     ;33
        LDW R1, R1, #0  ;48
        STW R1, R0, #0  ;63

        AND R0, R0, #0  ;72
        AND R1, R1, #0  ;81
        AND R2, R2, #0  ;90
        AND R3, R3, #0  ;99

        LEA R0, SECADDR ;108
        LDW R0, R0, #0  ;XC000 123
        LEA R1, COUNT   ;132
        LDW R1, R1, #0  ;#20    147

LOOP    LDB R2, R0, #0  ;162    214     266     528
        ADD R0, R0, #1  ;171    223     275

        ADD R3, R3, R2  ;180    232     284
        ADD R1, R1, #-1 ;189    241     293     

        BRp LOOP        ;199    251     303

        STW R3, R0, #0  ;XC014

        LEA R0, SUMADDR  
        LDW R0, R0, #0  
        STW R3, R0, #0  ;XC017

        HALT
       
ADDR    .FILL x4000
NUM     .FILL #1

SECADDR .FILL xC000
COUNT   .FILL #20

SUMADDR .FILL x0000

        .END
