/*  Radio signal clock - DCF77 Module

    Computerarchitektur 3
    (C) 2018 J. Friedrich, W. Zimmermann Hochschule Esslingen

    Author:   W.Zimmermann, Jun  10, 2016
    Modified: -
*/

#include <hidef.h>                                      // Common defines
#include <mc9s12dp256.h>                                // CPU specific defines
#include <stdio.h>

#include "dcf77.h"
#include "led.h"
#include "clock.h"
#include "lcd.h"

// Global variable holding the last DCF77 event
// possible states:
//   NODCF77EVENT    - no event
//   VALIDSECOND     - second pulse detected
//   VALIDZERO       - valid zero bit detected
//   VALIDONE        - valid one bit detected
//   VALIDMINUTE     - minute marker detected
//   INVALID         - invalid signal detected
DCF77EVENT dcf77Event = NODCF77EVENT;

// Modul internal global variables for the received dcf77 signal
static int  dcf77Year=2017, dcf77Month=1, dcf77Day=1, dcf77Hour=0, dcf77Minute=0, dcf77Weekday=1;
// Weekday names will use the dcf77Weekday variable as index (1=Monday, 7=Sunday)
static char dcf77WeekdayNames[7][4] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

// calculated EST time
// Modul internal global variables for EST time
static int estYear=2017, estMonth=1, estDay=1, estHour=0, estWeekday=1;
static int monthDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

// *******************************************************************
// internal function: setESTWithDCF77 ... This function sets the estyear, 
// estmonth, estday, estweekday and esthour as well as esthour
// Parameter:   -
// Returns:     -
// Note:        It uses the dcf77 values for calculation. 
//              Thus they must be set correctly.
void setESTWithDCF77(void) {
    estYear = dcf77Year;
    estMonth = dcf77Month;
    estDay = dcf77Day;
    estWeekday = dcf77Weekday;
    // calculate EST time
    estHour = dcf77Hour - 6;
    // if the hour is negative, subtract 1 from the day and set the hour to 24 - hour
    if (estHour < 0) {
        estHour += 24;
        estDay = dcf77Day - 1;
        // set the weekday
        estWeekday = dcf77Weekday - 1;
        if (estWeekday == 0) {
            estWeekday = 7;
        }
        // if the day goes below 1, set it to the last day of the previous month
        if (estDay == 0) {
            estMonth = dcf77Month - 1;
            // if the month goes below 1, go to the last month of the previous year
            if (estMonth == 0) {
                estMonth = 12;
                estYear = dcf77Year - 1;
                estDay = monthDays[11];
            // if the month is February, set the day to 28 and check for leap year
            } else if (estMonth == 2) {
                estDay = 28;
                if (estYear % 4 == 0) {
                    if (estYear % 100 != 0 || estYear % 400 == 0) {
                        estDay = 29;
                    }
                }
            // if the month is not February, set the day to the last day of the previous month
            } else {
                estDay = monthDays[estMonth - 1];
            }
        }
    }
}

// Variables for the DCF77 state machine
static int currentBit = 0;  // Current bit position in the DCF77Buffer
static char dcf77Buffer[59];  // Buffer for the DCF77 signal bits
static char ERROR = 1;  // Error flag
static char paritySum = 0;  // Parity sum (will be used for parity check)

char EST = 0;  // Flag for EST time

// Counter for for-loops because they cannot be declared
// inside the for-loop in this version of C
int i = 0;

// Prototypes of functions simulation DCF77 signals, when testing without
// a DCF77 radio signal receiver
void initializePortSim(void);                   // Use instead of initializePort() for testing
char readPortSim(void);                         // Use instead of readPort() for testing

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
{   
    
    setClock((char) dcf77Hour, (char) dcf77Minute, 0);
    displayDateDcf77();

    initializePort();
}

// ****************************************************************************
// Display the date derived from the DCF77 signal on the LCD display, line 1
// Parameter:   -
// Returns:     -
void displayDateDcf77(void)
{   char datum[32];

    // update EST time
    setESTWithDCF77();

    (void) sprintf(datum, "%s%02d.%02d.%04d%s", EST ? dcf77WeekdayNames[estWeekday-1] : dcf77WeekdayNames[dcf77Weekday-1], EST ? estDay : dcf77Day, EST ? estMonth : dcf77Month, EST ? estYear : dcf77Year, EST ? "US" : "EU");

    writeLine(datum, 1);
}

// *******************************************************************
// Public function: sampleSignalDCF77 ... Read and evaluate 
// DCF77 signal and detect events
// Parameter:  Current CPU time base in milliseconds
// Returns:    DCF77 event, i.e. second pulse, 0 or 1 data 
//             bit or minute marker
// Note:       Must be called by user every 10ms
//             If the signal is low, the function will toggle LED B.1
DCF77EVENT sampleSignalDCF77(int currentTime)
{
    static char lastSignal = 0;
    static int lastTime = 0;
    DCF77EVENT event = NODCF77EVENT;

    char signal = readPort();  // Read current signal state

    // Detect edges and measure pulse lengths
    if (signal != lastSignal) {
        if (signal == 0) {
            // Falling edge detected
            int pulseLength = currentTime - lastTime;
            if (pulseLength >= 700 && pulseLength <= 1300) {
                event = VALIDSECOND;
            } else if (pulseLength >= 1700 && pulseLength <= 2300) {
                event = VALIDMINUTE;
            } else {
                event = INVALID;
            }
        } else {
            // Rising edge detected
            int lowLength = currentTime - lastTime;
            if (lowLength >= 70 && lowLength <= 130) {
                event = VALIDZERO;
            } else if (lowLength >= 170 && lowLength <= 230) {
                event = VALIDONE;
            } else {
                event = INVALID;
            }
        }
        lastTime = currentTime;
        lastSignal = signal;
    } else {
        event = NODCF77EVENT;
    }

    // Toggle LED B.1 when the signal is low
    if (event != NODCF77EVENT) {
        PORTB ^= 0x02; // Toggle LED B.1
    }

    return event;
}

// ********************************************************************
// Public function: processEventsDCF77 ... Process the DCF77 
// events and decode the time and date

// Contains the DCF77 state machine
// Parameter:   Result of sampleSignalDCF77 as parameter
// Returns:     -
// Note:        Must be called by user after sampleSignalDCF77().
//              On error (Invalid data or parity) the error flag is set
//              and the error LED B.2 is turned on and the time and date 
//              is not updated.
//              On valid data the error flag is cleared and 
//              the error LED B.2 is turned off, 
//              LED B.3 is turned on and the time and date is updated
//              It also uses the EST flag to correctly display 
//              the time for the European and US time zones.
void processEventsDCF77(DCF77EVENT event)
{
// --- Add your code here ----------------------------------------------------
    switch (event)
    {
    case VALIDSECOND:
        currentBit++;
        if (currentBit > 58)
        {
            currentBit = 0;
            ERROR = 1;
        }
        break;
    case VALIDZERO:
        dcf77Buffer[currentBit] = 0;
        break;
    case VALIDONE:
        dcf77Buffer[currentBit] = 1;
        break;
    case VALIDMINUTE:
        if (currentBit == 58) {
            currentBit = 0;
            // check parity
            // 21 - 27
            paritySum = 0;
            for (i = 21; i <= 28; i++) {
                paritySum += dcf77Buffer[i];
            }
            if (paritySum % 2 != 0) {
                ERROR = 1;
                break;
            }
            // 29 - 34
            paritySum = 0;
            for (i = 29; i <= 35; i++) {
                paritySum += dcf77Buffer[i];
            }
            if (paritySum % 2 != 0) {
                ERROR = 1;
                break;
            }
            // 36 - 57
            paritySum = 0;
            for (i = 36; i <= 58; i++) {
                paritySum += dcf77Buffer[i];
            }
            if (paritySum % 2 != 0) {
                ERROR = 1;
                break;
            }
            // decode minutes
            dcf77Minute = 0;
            dcf77Minute += dcf77Buffer[21] * 1;
            dcf77Minute += dcf77Buffer[22] * 2;
            dcf77Minute += dcf77Buffer[23] * 4;
            dcf77Minute += dcf77Buffer[24] * 8;
            dcf77Minute += dcf77Buffer[25] * 10;
            dcf77Minute += dcf77Buffer[26] * 20;
            dcf77Minute += dcf77Buffer[27] * 40;
            // check if minutes are valid
            if (dcf77Minute > 59) {
                ERROR = 1;
                break;
            }

            // decode hours
            dcf77Hour = 0;
            dcf77Hour += dcf77Buffer[29] * 1;
            dcf77Hour += dcf77Buffer[30] * 2;
            dcf77Hour += dcf77Buffer[31] * 4;
            dcf77Hour += dcf77Buffer[32] * 8;
            dcf77Hour += dcf77Buffer[33] * 10;
            dcf77Hour += dcf77Buffer[34] * 20;
            // check if hours are valid
            if (dcf77Hour > 23) {
                ERROR = 1;
                break;
            }

            // decode day
            dcf77Day = 0;
            dcf77Day += dcf77Buffer[36] * 1;
            dcf77Day += dcf77Buffer[37] * 2;
            dcf77Day += dcf77Buffer[38] * 4;
            dcf77Day += dcf77Buffer[39] * 8;
            dcf77Day += dcf77Buffer[40] * 10;
            dcf77Day += dcf77Buffer[41] * 20;
            // check if day is valid
            if (dcf77Day > 31 || dcf77Day == 0) {
                ERROR = 1;
                break;
            }

            // decode weekday
            dcf77Weekday = 0;
            dcf77Weekday += dcf77Buffer[42] * 1;
            dcf77Weekday += dcf77Buffer[43] * 2;
            dcf77Weekday += dcf77Buffer[44] * 4;
            // check if weekday is valid
            if (dcf77Weekday > 7 || dcf77Weekday == 0) {
                ERROR = 1;
                break;
            }

            // decode month
            dcf77Month = 0;
            dcf77Month += dcf77Buffer[45] * 1;
            dcf77Month += dcf77Buffer[46] * 2;
            dcf77Month += dcf77Buffer[47] * 4;
            dcf77Month += dcf77Buffer[48] * 8;
            dcf77Month += dcf77Buffer[49] * 10;
            // check if month is valid
            if (dcf77Month > 12 || dcf77Month == 0) {
                ERROR = 1;
                break;
            }

            // decode year
            dcf77Year = 0;
            dcf77Year += dcf77Buffer[50] * 1;
            dcf77Year += dcf77Buffer[51] * 2;
            dcf77Year += dcf77Buffer[52] * 4;
            dcf77Year += dcf77Buffer[53] * 8;
            dcf77Year += dcf77Buffer[54] * 10;
            dcf77Year += dcf77Buffer[55] * 20;
            dcf77Year += dcf77Buffer[56] * 40;
            dcf77Year += dcf77Buffer[57] * 80;
            dcf77Year += 2000;
            // check if year is valid
            if (dcf77Year > 2099) {
                ERROR = 1;
                break;
            }

        } else {
            currentBit = 0;
            ERROR = 1;
            break;
        }
        ERROR = 0;

        // set EST time
        setESTWithDCF77();

        setClock(EST ? (char) estHour : (char) dcf77Hour, (char) dcf77Minute, 0);

        break;
    case INVALID:
        ERROR = 1;
        break;
    default:
        break;
    }

    if (ERROR == 1) {
        // TURN ON LED B.2
        PORTB |= 0x04;
        // TURN OFF LED B.3
        PORTB &= ~0x08;
    } else {
        // TURN OFF LED B.2
        PORTB &= ~0x04;
        // TURN ON LED B.3
        PORTB |= 0x08;
    }
}
