/*
*---------------------
* Title: ADC and Interfacing to an LCD
*---------------------
* Program Details:
* The purpose of this program is to utilize the built in ADC functionality of
* PIC18 as well as interact with an LCD display.
    
* Inputs: RB0-RB7, RC2
* Outputs: RD0-RD3
* Setup: The Curiosity Board, Accelerometer, LCD, LED

* Date: May 2nd, 2026
* File Dependencies / Libraries: It is required to include the
* MyConfigFile.inc in the Header Folder
* Compiler: XC-8, v3.10
* Author: Brandon McCulloch
* Versions:
*       V1.0: Original
 * Microcontroller: PIC18F47K42
*/

// PIC18F47K42 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1L
#pragma config FEXTOSC = OFF    // Disable external oscillator 
#pragma config RSTOSC = HFINTOSC_64MHZ // Use internal chip oscilator at 64MHz

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock out Enable bit (CLKOUT function is disabled)
#pragma config PR1WAY = ON      // PRLOCKED One-Way Set Enable bit (PRLOCK bit can be cleared and set only once)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // MCLR Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR )
#pragma config PWRTS = PWRT_OFF // Power-up timer selection bits (PWRT is disabled)
#pragma config MVECEN = OFF     // Multi-vector enable bit (Multi-vector enabled, Vector table used for interrupts)
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
//#include "../../../../../Program Files/Microchip/xc8/v2.40/pic/include/proc/pic18f46k42.h"
//#include "C:\Program Files\Microchip\xc8\v2.40\pic\include\proc\pic18f46k42"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define _XTAL_FREQ 64000000UL       // Fosc  frequency for the delay library
#define FCY    _XTAL_FREQ/4
#define RS LATDbits.LATD0           // RS pin on LCD connected to RD0
#define EN LATDbits.LATD1           // E pin on LCD connected to RD1
#define LCD_DATA LATB               // Port B contians all data pins from LCD

#define Vref 3.3 // voltage reference 
int digital; // holds the digital value 
float voltage; // hold the analog value (volt))
char data[10];
void ADC_Init(void);
void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Char(unsigned char dat);
void LCD_String(const char *msg);
void LCD_String_xy(unsigned char row, unsigned char pos, const char *msg);
void LCD_Clear(void);
void IOC_Init(void);
void Halt_For_10_Seconds(void);
volatile unsigned char haltRequested = 0;

/*This code block configures the ADC
for polling, VDD and VSS references, ADCRC
oscillator and AN0 input.
Conversion start & polling for completion
are included.
 */

void __interrupt() ISR(void) {
    if (IOCCFbits.IOCCF2)
    {
        haltRequested = 1;       // tell main loop to halt
        IOCCFbits.IOCCF2 = 0;    // clear RC2 IOC flag
        PIR0bits.IOCIF = 0;      // clear general IOC flag
    }
}

void main(void)
{
    // LCD data pins on PORTB
    ANSELB = 0x00;
    TRISB = 0x00;
    LATB = 0x00;

    // LCD control pins on RD0 and RD1
    ANSELDbits.ANSELD0 = 0;
    ANSELDbits.ANSELD1 = 0;
    TRISDbits.TRISD0 = 0;   // RS output
    TRISDbits.TRISD1 = 0;   // EN output

    // Accelerometer X input on RD2 / AND2
    TRISDbits.TRISD2 = 1;   // RD2 input
    ANSELDbits.ANSELD2 = 1; // RD2 analog mode
    
    
    // Set D0 to D3 outputs to value 0
    LATDbits.LATD0 = 0;
    LATDbits.LATD1 = 0;
    LATDbits.LATD3 = 0;

    
    //Initialize ADC, LCD, and Interrupt
    ADC_Init();
    LCD_Init();
    IOC_Init();

    // Display Accelerometer on the top portion of the LCD
    LCD_String_xy(1, 0, "Accelerometer");
    
    //Initialize previous acceleration value to 0
    float previous_g = 0.0;

    //Run the program indefinitely
    while (1) {
        if (haltRequested == 1) {   // If haltRequested was set to 1, reset and call delay Function.
            haltRequested = 0;
            Halt_For_10_Seconds();
        }
        char line1[17];
        char line2[17];

        //define variables
        float zeroVoltage = 1.65;    // around half of 3.3V for many analog accelerometers
        float sensitivity = 0.318;   // about 300 mV/g for ADXL335-style modules
        float accel_g;               
        float accel_ms2;
        float change;
        ADCON0bits.GO = 1;          // Start the ADC conversion
        while (ADCON0bits.GO);      // While until the conversion is complete

        digital = ((int) ADRESH * 256) | ADRESL; // Combine high and low ADC registers to get full 12 bit result.   
        voltage = digital * ((float) Vref / 4095.0); // Convert to voltage reading 

        accel_g = (voltage - zeroVoltage) / sensitivity;    // Obtain the acceleration in g's
        accel_ms2 = accel_g * 9.81;                         // Convert to m/s^2

        change = accel_g - previous_g;                      // Calculate the change in acceleration from the previous reading.
        if (change < 0) {
            change = -change;
        }

        // Define the status of the system based on the value of accel_g and change
        if (change > 0.60) {
            sprintf(line1, "Shake           ");
        } else if (accel_g > 0.25) {
            sprintf(line1, "Tilt Right      ");
        } else if (accel_g < -0.25) {
            sprintf(line1, "Tilt Left       ");
        } else {
            sprintf(line1, "Flat            ");
        }

        // Print the result to the LCD
        sprintf(line2, "%5.2f m/s2      ", accel_ms2); 

        LCD_String_xy(1, 0, line1);
        LCD_String_xy(2, 0, line2);

        previous_g = accel_g;           // Set current value to previous

        __delay_ms(250);
    }
    
}

// Initialize Interrupt Function
void IOC_Init(void)
{
     // RD3 = red LED output
    ANSELDbits.ANSELD3 = 0;
    TRISDbits.TRISD3 = 0;
    LATDbits.LATD3 = 0;

    // RC2 = button input
    ANSELCbits.ANSELC2 = 0;  // digital mode
    TRISCbits.TRISC2 = 1;    // input

    // Enable weak pull-up on RC2
    WPUCbits.WPUC2 = 1;

    // Interrupt-on-change on falling edge
    // Falling edge happens when button is pressed and RC2 goes HIGH -> LOW
    IOCCPbits.IOCCP2 = 0;    // disable rising edge
    IOCCNbits.IOCCN2 = 1;    // enable falling edge

    // Clear flags before enabling interrupt
    IOCCFbits.IOCCF2 = 0;
    PIR0bits.IOCIF = 0;

    // Enable IOC interrupt
    PIE0bits.IOCIE = 1;

    // Enable global interrupts
    INTCON0bits.GIE = 1;
}

//Initialize LCD Function
void LCD_Init(void)
{
    __delay_ms(500);

    LCD_Command(0x38);   // 8-bit mode, 2 lines, 5x8 font
    __delay_ms(20);

    LCD_Command(0x38);
    __delay_ms(20);

    LCD_Command(0x38);
    __delay_ms(20);

    LCD_Command(0x0C);   // Display ON, cursor OFF
    __delay_ms(20);

    LCD_Command(0x06);   // Entry mode: increment cursor
    __delay_ms(20);

    LCD_Command(0x01);   // Clear display
    __delay_ms(20);
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    __delay_ms(20);
}

//Instructions for LCD
void LCD_Command(unsigned char cmd)
{
    LCD_DATA = cmd;

    RS = 0;
    __delay_ms(1);

    EN = 1;
    __delay_ms(2);
    EN = 0;

    __delay_ms(5);
}

// Sending characters to LCD
void LCD_Char(unsigned char dat)
{
    LCD_DATA = dat;

    RS = 1;
    __delay_ms(1);

    EN = 1;
    __delay_ms(1);
    EN = 0;

    __delay_ms(2);
}

// Sending multiple characters to LCD (Repeatedly calling LCD_Char)
void LCD_String(const char *msg)
{
    while(*msg != '\0')
    {
        LCD_Char(*msg);
        msg++;
    }
}

//  Positioning where each character should be place
void LCD_String_xy(unsigned char row, unsigned char pos, const char *msg)
{
    unsigned char location;

    if(row == 1)
    {
        location = 0x80 + pos;
    }
    else
    {
        location = 0xC0 + pos;
    }

    LCD_Command(location);
    LCD_String(msg);
}

// Initializing ADC Function
void ADC_Init(void)
{
    ADCON0bits.FM = 1;      // Right justify ADC result
    ADCON0bits.CS = 1;      // Use ADCRC clock

    // RD2 is analog channel AND2
    TRISDbits.TRISD2 = 1;   
    ANSELDbits.ANSELD2 = 1;

    ADPCH = 0x1A;           // AND2 = RD2

    ADCLK = 0x00;

    ADREF = 0x00;           // Vref+ = VDD, Vref- = VSS

    ADRESH = 0x00;
    ADRESL = 0x00;

    ADPREL = 0x00;
    ADPREH = 0x00;

    ADACQL = 0x00;
    ADACQH = 0x00;

    ADCON0bits.ON = 1;      // Turn ADC on
}

// Function to delay the remaining of the program and flash the LED for 10 seconds
void Halt_For_10_Seconds(void) {

    for (int i = 0; i < 20; i++)
    {
        LATDbits.LATD3 = 1;
        __delay_ms(250);

        LATDbits.LATD3 = 0;
        __delay_ms(250);
    }

}