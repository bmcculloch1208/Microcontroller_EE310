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