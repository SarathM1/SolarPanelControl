

// PIC16F877A Configuration Bit Settings

// 'C' source line config statements

#include <xc.h>
#include "LCD.h"
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

	
//NAMING PORT PINS 
static bit str_1		@ ((unsigned)&PORTA*8+3); 
static bit str_2		@ ((unsigned)&PORTA*8+5); 
static bit relay		@ ((unsigned)&PORTA*8+2); 

#define tol	1

unsigned int DC_V, count;

unsigned char relay_count;

signed int comp_value;

bit first_time;

static unsigned char sin_out = 0;

static unsigned char sin_tab_index = 0;

const unsigned char sin_tab[25] = 
{
	  0,  31,  62,  92,  120, 146, 170, 192,
	210, 225, 237, 245, 249, 249, 245, 237,
	224, 210, 192, 170, 146,  120,  92,  62,
	31
};


bit read_ADC, disp_flag;

void func_disp(void);
void main()
{
	//ANSEL = 0b00000011;//RA0,RA1 are set as analog,all other port pins are digital I/O pins
    ADCON1 = 0x00;
    ADCON0 = 0x81;
	TRISA = 0b00000001;//0x01
	PORTA = 0;
	//ANSELH = 0x00;	//all portB pins are digital
	TRISB = 0x00;
	PORTB = 0x00;
	TRISC = 0x00;
	PORTC = 0x00;

//CONFIGURING TIMER2 TO GENERATE 20KHz PWM 
	T2CON = 0b00111000;	//prescaler 1:1, post scaler 1:8,Timer2 off.
	PR2 = 249;	//to generate 50 microsec(20Khz) 
//ENABLING PWM MODULE FOR BUCK CONVERTER*********
	CCP2CON = 0b00001100;
	CCPR2L = 125;// PWM dutycycle register. 0 to 249
	

//ENABLING ENHANCED PWM FOR GENERATING SINE WAVE
	CCP1CON = 0b10001100;	//Pa, Pb are active high,half bridge mode selected
	//PWM1CON = 0b00001010;	//2 microseconds dead time inserted

//str_1 = 1;

// Enabling timer2 interrupt****
	TMR2IF = 0;
	TMR2IE = 1;
	PEIE = 1;

//ADC module configuration
	ADCON1 =0x80;	//ADRES right aligned,internal voltage ref. selected

	TMR2ON = 1;

	OPTION_REG = 0x05;	//prescaler 1:64
	GIE = 1;
	T0IE = 1;

	InitLCD();

	WriteStringToLCD("SOLAR H.C SYSTEM");
  	while(1)
	{
		if(relay_count == 10)
			relay = 1;
		if(read_ADC)
		{
			ADCON0 = 0b10000001;	//Tad = Fosc/32, AN0 selected,adc module on,conversion not started.
			__delay_us(20);	//waiting for acquiring charge
			GO_nDONE = 1;		//initiating convertion
			while(GO_nDONE)	//waiting for convertion complete
				continue;
			DC_V = (ADRESH*256 + ADRESL)*7/100;
	
			if((DC_V < 30-tol)&& CCPR2L < 249)
				CCPR2L++;
			else if((DC_V > 30)&& CCPR2L > 0)
				CCPR2L--;
            
			read_ADC = 0;
		}
		if(disp_flag)
		{
			comp_value = 100-(CCPR2L*13/25);
			if(comp_value < 0)
				comp_value = 0;
			func_disp();
			disp_flag = 0;		
		}
	}
}																													

static void interrupt isr (void)
{
	if(T0IF)
	{
		T0IF = 0;
		read_ADC = 1;
		if(++count == 305)
		{
			disp_flag = 1;
			count = 0;
			if(relay_count < 10)
				relay_count++;
		}
	}

	if(TMR2IF)
	{
		TMR2IF=0;
		CCPR1L = sin_tab[sin_tab_index];
		sin_tab_index++;
		if (sin_tab_index >= 25)
		{
			sin_tab_index = 0;
		}
		
		if (sin_tab_index == 1)
		{
			if(first_time == 0)
			{
				str_1 = 1;
				str_2 = 0;
				first_time = 1;
			}
			else
			{
				str_1 = !str_1;
				str_2 = !str_2;
			}

//				str_1 = !str_1;
//				str_2 = !str_2;
		}
	}	
}



void func_disp(void)
{
	unsigned char DIG0, DIG1, DIG2;
	DIG0 = comp_value%10;
	DIG1 = (comp_value/10)%10;
	DIG2 = (comp_value/100)%10;
	//lcd_goto(64);
    ClearLCDScreen();
	WriteStringToLCD("Solar Usage:");
	WriteDataToLCD(DIG2+'0');
	WriteDataToLCD(DIG1+'0');
	WriteDataToLCD(DIG0+'0');
	WriteDataToLCD('%');
}