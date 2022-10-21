    .ORIG x1200
    
    ADD R6, R6, #-2 ;9
    STW R0, R6, #0  ;24

    ADD R6, R6, #-2 ;33
    STW R1, R6, #0  ;48

;DO THINGS HERE

    AND R0, R0, #0  ;57
    AND R1, R1, #0  ;66

    LEA R0, ADDR    ;66
    LDW R0, R0, #0  ;81
    LDW R1, R0, #0  ;96

    ADD R1, R1, #1  ;105

    STW R1, R0, #0  ;120

;FINISHED

    LDW R1, R6, #0  ;135
    ADD R6, R6, #2  ;144

    LDW R0, R6, #0  ;159
    ADD R6, R6, #2  ;168

    RTI
    HALT

ADDR    .FILL   x4000

    .END