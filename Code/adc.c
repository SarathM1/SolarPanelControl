

// PIC16F877A Configuration Bit Settings

// 'C' source line config statements

#include <xc.h>
#include <stdlib.h>

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ 20000000
#include "LCD.h"

void display_float(float val);
void adc_init()
{
    TRISA = 0xff;         // AN0
    ADCON1 = 0x00;
    ADCON0 = 0x81;
}

int read_adc(int channel)
{
    switch(channel)
    {
        case 0:
            CHS2 = CHS1 = CHS0 = 0;
            break;
        case 1:
            CHS2 = CHS1 = 0;
            CHS0 = 1;
            break;
        case 2:
            CHS2 = CHS0 = 0;
            CHS1 = 1;
            break;
    }
    __delay_ms(10);
    GO_nDONE = 1;
    while(GO_nDONE);
    return ((ADRESH<<2) + (ADRESL>>6));
}

float convertToVolt(int adc_value)
{
    float res;
    res = (float)adc_value;
    res = (res * 5/1023);
    return res;
}

void display_float(float val)
{
    char * buf;
    int status;
    buf = ftoa(val, &status);
    buf[5] = '\0';                  //Float precision Adjustment
    WriteStringToLCD(buf);
}

void main()
{
    int adc_value;
    float an0, an1, an2;
    adc_init();
    InitLCD();
    TRISB = 0x00;
    while(1)
    {
        WriteCommandToLCD(0x80);
        adc_value = read_adc(0);
        an0 = convertToVolt(adc_value);
        display_float(an0);
        //__delay_ms(10);
        
        WriteCommandToLCD(0x8A);
        adc_value = read_adc(1);
        an1 = convertToVolt(adc_value);
        display_float(an1);
        //__delay_ms(10);
        
        WriteCommandToLCD(0xC0);
        adc_value = read_adc(2);
        an2 = convertToVolt(adc_value);
        display_float(an2);
        //__delay_ms(10);
        
        //ClearLCDScreen();
    }
}
