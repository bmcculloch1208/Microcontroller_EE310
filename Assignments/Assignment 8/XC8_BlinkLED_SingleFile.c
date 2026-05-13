/*
 * ---------------------
 * Title: Simple LED Blinking 
 * ---------------------
 * Program Details:
 *  The purpose of this program is to create a hands free lock system
 * Inputs: Interrupt Button, RB0, RB1, RB2
 * Outputs: RD0-RD6, RA0, RA1
 * Setup: C - Simulator
 * Date: April 27th, 2026
 * File Dependencies / Libraries: It is required to include the 
 * Configuration Header File 
 * Compiler: xc8, 2.4
 * Author: Brandon McCulloch
 * Versions:
 *      V1.0: Original
 * Useful links:  
 *      Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/PIC18(L)F26-27-45-46-47-55-56-57K42-Data-Sheet-40001919G.pdf 
 *      PIC18F Instruction Sets: https://onlinelibrary.wiley.com/doi/pdf/10.1002/9781119448457.app4 
 *      List of Instrcutions: http://143.110.227.210/faridfarahmand/sonoma/courses/es310/resources/20140217124422790.pdf 
 */

// PIC18F46K42 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1L
#pragma config FEXTOSC = LP     // External Oscillator Selection (LP (crystal oscillator) optimized for 32.768 kHz; PFM set to low power)
#pragma config RSTOSC = EXTOSC  // Reset Oscillator Selection (EXTOSC operating per FEXTOSC bits (device manufacturing default))

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock out Enable bit (CLKOUT function is disabled)
#pragma config PR1WAY = ON      // PRLOCKED One-Way Set Enable bit (PRLOCK bit can be cleared and set only once)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // MCLR Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR )
#pragma config PWRTS = PWRT_OFF // Power-up timer selection bits (PWRT is disabled)
#pragma config MVECEN = ON      // Multi-vector enable bit (Multi-vector enabled, Vector table used for interrupts)
#pragma config IVT1WAY = ON     // IVTLOCK bit One-way set enable bit (IVTLOCK bit can be cleared and set only once)
#pragma config LPBOREN = OFF    // Low Power BOR Enable bit (ULPBOR disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled , SBOREN bit is ignored)

// CONFIG2H
#pragma config BORV = VBOR_2P45 // Brown-out Reset Voltage Selection bits (Brown-out Reset Voltage (VBOR) set to 2.45V)
#pragma config ZCD = OFF        // ZCD Disable bit (ZCD disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PPS1WAY = ON     // PPSLOCK bit One-Way Set Enable bit (PPSLOCK bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle)
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config DEBUG = OFF      // Debugger Enable bit (Background debugger disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Extended Instruction Set and Indexed Addressing Mode disabled)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period selection bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled; SWDTEN is ignored)

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4L
#pragma config BBSIZE = BBSIZE_512// Boot Block Size selection bits (Boot Block size is 512 words)
#pragma config BBEN = OFF       // Boot Block enable bit (Boot block disabled)
#pragma config SAFEN = OFF      // Storage Area Flash enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block write protection bit (Application Block not write protected)

// CONFIG4H
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write-protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
#pragma config WRTSAF = OFF     // SAF Write protection bit (SAF not Write Protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low voltage programming enabled. MCLR/VPP pin function is MCLR. MCLRE configuration bit is ignored)

// CONFIG5L
#pragma config CP = OFF         // PFM and Data EEPROM Code Protection bit (PFM and Data EEPROM code protection disabled)

#include <xc.h> // must have this
#include "MyConfigFile.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include "../../../../../Program Files/Microchip/xc8/v2.40/pic/include/proc/pic18f46k42.h"
//#include "pic18f47k42.h"
//#include "C:\Program Files\Microchip\xc8\v2.40\pic\include\proc\pic18f46k42"

#define PR1 PORTBbits.RB1
#define PR2 PORTBbits.RB2
#define emergencyButton PORTBbits.RB0
#define Buzzer LATAbits.LATA0
#define Motor LATAbits.LATA1

#define _XTAL_FREQ 4000000

// Function
unsigned char countDigitOne(void);
unsigned char countDigitTwo(void);
volatile unsigned char eSwitch = 0;
void segDisplay(unsigned char);
void correctCode(void);
void melodyTone(void);
void Reset_Switch(void);
void systemInit(void);

// Initialize each pin, enable weak pull ups for inputs, initialize interrupts
void systemInit(void) {
    OSCCON1 = 0x60;
    OSCFRQ = 0x02;
    
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELD = 0x00;
    ANSELC = 0x00;
    
    // Inputs
    TRISBbits.TRISB0 = 1; // Emergency Switch
    TRISBbits.TRISB1 = 1; // 1st photo resistor
    TRISBbits.TRISB2 = 1; // 2nd photo resistor
    
    // Outputs
    TRISAbits.TRISA0 = 0; // Buzzer
    TRISAbits.TRISA1 = 0; // Motor/Relay
    TRISD = 0;            // 7-segment. Set all pins as Outputs
    
    // Weak Pull Ups for photo resistors and eSwitch.
    WPUBbits.WPUB0 = 1;
    WPUBbits.WPUB1 = 1;
    WPUBbits.WPUB2 = 1;
    //WPUBbits.WPUB2 = 1;
    
    // emergency interrupt INT0
    INTCON0bits.INT0EDG = 0;
    PIR1bits.INT0IF = 0;
    PIE1bits.INT0IE = 1;
    INTCON0bits.GIEL = 1;
    INTCON0bits.GIEH = 1;
    
   

}

void __interrupt(irq(IRQ_INT0), base(8)) INT0_ISR(void)
{
    eSwitch = 1;  // Emergency flag
    Motor = 0;    // Set motor output pin to 0.
    Buzzer = 0;   // Set buzzer out pin to 0.
    melodyTone(); // Call the melody tone function to set off melody for buzzer
    PIR1bits.INT0IF = 0; //Reset the emergency flag 
}

const unsigned char segSetup[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x67, // 9
    
};

void segDisplay(unsigned char digitValue) {
        LATD = segSetup[digitValue]; // Output bits to 7-segment
        return;
}

unsigned char countDigitOne(void) {
    unsigned char digitOne = 0;     
    unsigned char prior_state;
    int idleCount = 0;
    unsigned char passive = 1;
    
    prior_state = 0;
    
    while (passive == 1) {
        if (PR1 == 1)
            passive = 0;
        __delay_ms(50);
    }
    
    // If the emergency switch flag is still 1, return.
    while (1) {
        if (eSwitch == 1) {
            return 0;
        }
        
        // If the system has been idling for long enough, return back to main
        if (idleCount == 50) {
            return digitOne;
        }
        
        // If PR1 is high and the prior state of PR1 was 0, increment 7-segment by 1
        if (PR1 == 1 && prior_state == 0)
        {
            // If 7-segment value is currently 9, rotate back to 0. Otherwise increment
            if (digitOne == 9){
                digitOne = 0;
                segDisplay(digitOne);
            }
            else {
                digitOne++;
                segDisplay(digitOne);
            }
            idleCount = 0;
            prior_state = 1;
            
            __delay_ms(130);
            
        }
        // If PR1 is high and it was high earlier, too soon to count again ... delay
        else if (PR1 == 1 && prior_state == 1) {
            __delay_ms(50);
        }
        // If PR1 is low and was previously low, nothing happened, delay and increase idlecount value 
        else if (PR1 == 0 && prior_state == 0) {
            __delay_ms(30);
            
            idleCount++;

        }
        // If PR1 is low but was previously high, reset the prior state to 0.
        else if (PR1 == 0 && prior_state == 1) {
            prior_state = 0;
 
        }
    }
    
}

unsigned char countDigitTwo(void) {
    unsigned char digitTwo;
    unsigned char prior_state;
    int idleCount = 0;
    
    prior_state = 0;
    
    digitTwo = 0;
    segDisplay(digitTwo);
    
    while (1) {
        if (eSwitch == 1)
            return 0;
        
        // If the system has been idling for long enough, return back to main
        if (idleCount == 50) {
            return digitTwo;
        }
        
        // If PR2 is high and the prior state of PR1 was 0, increment 7-segment by 1
        if (PR2 == 1 && prior_state == 0)
        {
            // If 7-segment value is currently 9, rotate back to 0. Otherwise increment
            if (digitTwo == 9){
                digitTwo = 0;
                segDisplay(digitTwo);
            }
            else {
                digitTwo++;
                segDisplay(digitTwo);
            }
            idleCount = 0;
            prior_state = 1;
            
            __delay_ms(130);

        }
        // If PR1 is high and it was high earlier, too soon to count again ... delay
        else if (PR2 == 1 && prior_state == 1) {
            __delay_ms(50);

        }
        // If PR1 is low and was previously low, nothing happened, delay and increase idlecount value 
        else if (PR2 == 0 && prior_state == 0) {
            __delay_ms(30);
            
            idleCount++;

        }
        // If PR1 is low but was previously high, reset the prior state to 0.
        else if (PR2 == 0 && prior_state == 1) {
            prior_state = 0;
 
        }
    }
    
}

// If code is correct, enable the motor for 3 seconds.
void correctCode(void) {
    Motor = 1;
    __delay_ms(3000);
    Motor = 0;
    return;
}

// If code is incorrect, enable the butter for 3 seconds.
void incorrectCode(void) {
    Buzzer = 1;
    __delay_ms(3000);
    Buzzer = 0;
    return;
}

// If the interrupt is triggered, play a melody on the buzzer for a few seconds.
void melodyTone(void) {
    int count = 0;
    while (count <= 4) {
        Buzzer = 1;
        __delay_ms(200);
        Buzzer = 0;
        __delay_ms(200);
        count++;
    }
}

void main(void)
{
   // WPUBbits.WPUB0;
   // LATDbits.LATD0 = 0;
   // PORTDbits.RD2 = 0; 
    
    unsigned char digitOne;
    unsigned char digitTwo;
    unsigned char correctDigitOne = 3;
    unsigned char correctDigitTwo = 1;
    
    systemInit();
    
    segDisplay(0);
    
    
    digitOne = countDigitOne();
    digitTwo = countDigitTwo();
    
    if (eSwitch == 1)
        while (1)
        {
        }
    
    // If the code is correct, call the correct code function. Otherwise call incorrect code.
    if (digitOne == correctDigitOne && digitTwo == correctDigitTwo)
        correctCode();
    else
        incorrectCode();
}

