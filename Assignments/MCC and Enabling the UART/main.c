/*
;---------------------------------------------------------------
; Title: Interfacing with PIC18 via Serial
;---------------------------------------------------------------
; Program Details:
;   This program controls a servo using two buttons. If the first button is pressed, then the servo
;   turns left. When the 2nd button is pressed, the servo moves right.
;   When no buttons are pressed, the servo holds its position for a few seconds. 
;   The servo stops when it reaches it limit in either direction.
;
; Inputs:
;
; Outputs:
;   USB Connection
;
; Setup:
;   - PIC18F47K42 mounted on breadboard
;   - Two pushbuttons used as servo directional input
;   - One servo connected as an output
;
; Date:
;   May 13, 2026
;
; File Dependencies / Libraries:
;   - mcc_generated_files/system/system.h
;
; Compiler:
;   XC8 v3.10
;
; Author:
;   Brandon McCulloch
;
; Versions:
;   V1.0
;---------------------------------------------------------------
*/

#include "mcc_generated_files/system/system.h"
#include <stdio.h>
#include <time.h>

/*
    Main application
*/
int random_number(int min_num, int max_num)
{
    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num)
    {
        low_num = min_num;
        hi_num = max_num + 1;   // include max_num in output
    }
    else
    {
        low_num = max_num + 1;  // include max_num in output
        hi_num = min_num;
    }

    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

int main(void)
{
    SYSTEM_Initialize();
    UART2_Initialize();
    srand(time(NULL));

    while(1)
    {
    printf("Random values: ");

    for(int i = 0; i < 30; i++)
    {
        printf("%d ", random_number(1, 100));
    }

    printf("\r\n");   // only go to a new line after all 30 values

    __delay_ms(1000);
    PORTBbits.RB1 ^= 1;
    }
}