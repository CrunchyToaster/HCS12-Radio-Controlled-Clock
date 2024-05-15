/*  Radio signal clock - DCF77 Module

    Computerarchitektur 3
    (C) 2018 J. Friedrich, W. Zimmermann Hochschule Esslingen

    Author:   W.Zimmermann, Jun  10, 2016
    Modified: -
*/

/*
; A C H T U N G:  D I E S E  S O F T W A R E  I S T  U N V O L L S T � N D I G
; Dieses Modul enth�lt nur Funktionsrahmen, die von Ihnen ausprogrammiert werden
; sollen.
*/


#include <hidef.h>                                      // Common defines
#include <mc9s12dp256.h>                                // CPU specific defines
#include <stdio.h>

#include "dcf77.h"
#include "led.h"
#include "clock.h"
#include "lcd.h"

// Wrapper for LED functions
void initLED(void)
void setLED(int led) {
    asm {
        ldb led
    }
}
void clrLED(int led) {
    asm {
        ldb led
    }
}

// Global variable holding the last DCF77 event
DCF77EVENT dcf77Event = NODCF77EVENT;

// Modul internal global variables
static int  dcf77Year=2017, dcf77Month=1, dcf77Day=1, dcf77Hour=0, dcf77Minute=0;       //dcf77 Date and time as integer values

// Prototypes of functions simulation DCF77 signals, when testing without
// a DCF77 radio signal receiver
void initializePortSim(void);                   // Use instead of initializePort() for testing
char readPortSim(void);                         // Use instead of readPort() for testing

//***************************************************************************

initLED()

// ****************************************************************************
// Initalize the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     -
void initializePort(void)
{
    // Configure Port H.0 as input
    DDRH &= ~(0x01); // Clear bit 0 of DDRH to set PH0 as input
    
    // Enable pull-up resistor on Port H.0 if required
    PERH |= 0x01;    // Set bit 0 of PERH to enable pull-up resistor on PH0

    // Configure Port B.0, B.1, B.2, and B.3 as output for LEDs
    DDRB |= 0x0F;    // Set lower nibble (bits 0-3) of DDRB to configure PB0-PB3 as output
}


// ****************************************************************************
// Read the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     0 if signal is Low, >0 if signal is High
char readPort(void)
{
    // Read the value of Port H.0
    if (PTH & 0x01) {
        return 1; // Signal is High
    } else {
        return 0; // Signal is Low
    }
}


// ****************************************************************************
//  Initialize DCF77 module
//  Called once before using the module
void initDCF77(void)
{   setClock((char) dcf77Hour, (char) dcf77Minute, 0);
    displayDateDcf77();

    initializePort();
}

// ****************************************************************************
// Display the date derived from the DCF77 signal on the LCD display, line 1
// Parameter:   -
// Returns:     -
void displayDateDcf77(void)
{   char datum[32];

    (void) sprintf(datum, "%02d.%02d.%04d", dcf77Day, dcf77Month, dcf77Year);
    writeLine(datum, 1);
}

// ****************************************************************************
// Read and evaluate DCF77 signal and detect events
// Must be called by user every 10ms
// Parameter:  Current CPU time base in milliseconds
// Returns:    DCF77 event, i.e. second pulse, 0 or 1 data bit or minute marker
DCF77EVENT sampleSignalDCF77(int currentTime)
{
    static char lastSignal = 0;
    static int lastTime = 0;
    DCF77EVENT event = NODCF77EVENT;

    char signal = readPort();  // Read current signal state
    if (signal != lastSignal) {
        lastSignal = signal;
        setLED(0b00000010); // Signal changed
    }
    else {
        clrLED(0b00000010); // Signal unchanged
    }

    return event;
}


// ****************************************************************************
// Process the DCF77 events
// Contains the DCF77 state machine
// Parameter:   Result of sampleSignalDCF77 as parameter
// Returns:     -
void processEventsDCF77(DCF77EVENT event)
{
// --- Add your code here ----------------------------------------------------
// --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? ---

}

