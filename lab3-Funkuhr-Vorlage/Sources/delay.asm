;********************************************************************
; Module: delay.asm
; Description: This module provides a delay function for half a second.
;********************************************************************

; Export symbols
            XDEF   delay_0_5_sec        ; Export the symbol delay_0_5_sec

; Include processor definitions (if necessary)
            INCLUDE 'mc9s12dp256.inc'

; Defines
        SPEED:  EQU     2048                   ; Change this number to change counting speed

; ROM: Assembler program code in RAM
.init: SECTION

;********************************************************************
; Public interface function: delay_0_5_sec ... Provides a delay of 0.5 seconds.
; Parameter: -
; Return:    -
delay_0_5_sec:
        PSHD
        PSHX
        PSHY

        LDX  #SPEED                      ; Delay loop to control toggle Frequency 
waitO:  LDY  #SPEED                      ; (Uses two nested counter loops with registers X and Y)
waitI:  DBNE Y, waitI                   ; --- Decrement Y and branch to waitI if not equal to 0
        DBNE X, waitO                   ; --- Decrement X and branch to waitO if not equal to 0

        PULY
        PULX
        PULD
        RTS