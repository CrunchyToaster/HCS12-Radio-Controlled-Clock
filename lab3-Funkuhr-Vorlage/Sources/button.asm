;********************************************************************
; Module: button.asm
; Description: This module handles the button inputs and toggles 
; between normal and set mode. It also increments hours, minutes, 
; and seconds based on button presses.
;********************************************************************

; Export symbols
    XDEF checkButtons  

; Import symbols
    XREF EST
    XREF delay_0_5_sec
; RAM: Variable data section
.data: SECTION

; ROM: Constant data
.const: SECTION

; ROM: Code section
.init: SECTION

; Include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

;********************************************************************
; Public interface function: checkButtons ... Checks button states
; and handles mode switching and time increments.
; Parameter: -
; Return:    -
; Note:      BRCLR needs to be changed to BRSET if program is used in Simulator
checkButtons:
    PSHD

    BRCLR PTH,#$08,changeMode   ; Check button for mode change

    PULD
    RTS
;********************************************************************
; Internal function: changeMode ... Toggles between normal and set mode.
; Parameter: -
; Return:    -
changeMode:
    LDAB EST
    EORB #$01
    STAB EST
    
    JSR delay_0_5_sec ; Prevents fast toggling

    PULD
    RTS

