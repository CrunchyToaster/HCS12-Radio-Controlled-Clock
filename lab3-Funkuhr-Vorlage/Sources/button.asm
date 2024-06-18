;********************************************************************
; Module: button.asm
; Description: This module handles the button inputs and toggles 
; the EST variable to switch between Amercian and European time
; when the third button is pressed.
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
; Public interface function: checkButtons ... Checks the button inputs
; and toggles the EST variable if the third button is pressed.
; Parameter: -
; Return:    -
; Note:      BRCLR needs to be changed to BRSET if program is used in Simulator
checkButtons:
    PSHD

    BRCLR PTH,#$08,changeMode   ; Check button for mode change

    PULD
    RTS
;********************************************************************
; Internal function: changeMode ... Toggles the EST variable
; Parameter: -
; Return:    -
; Note:      To avoid fast toggling, a delay is added before the
;            function returns.
changeMode:
    LDAB EST
    EORB #$01
    STAB EST
    
    JSR delay_0_5_sec ; Prevents fast toggling

    PULD
    RTS

