/*

* PLATFORM : PIC18F452
* COMPILER : MICROCHIP C18 COMPILER
* DESIGNER : DALI FETHI ABDELLATIF
* PROJECTS : CONTROLLING A DC MOTOR SPEED BY POTENTIOMETER

* THIS SOURCE FILE IS USED TO CONTROL A DC MOTOR VIA A POTENTIOMETER
* ADC MODULE AND THE CCP1 MODULE(PWM)


*/
#include <p18f452.h>
#pragma config WDT = OFF
#define value 9
#define MAX 1023

unsigned int result = 0, oldValue = 0;
unsigned int arraySpeedValue[] = {0, 10, 20, 30, 50, 70, 90, 100, 120, 124}; // ARRAY CONTAINS SPEED VALUES AND EACH SPEED IS REFERENCED VIA ITS INDEX		

void function(void);				// FUNCTION TO CONVERT THE OUTPUT OF ADC INTO A VALUE BETWEEN [0 9] AND THEN WE TURN THE DC MOTOR BASED ON ARRAY[RESULT]
void timeAcquisition(void);			// DELAY TO MAKE SURE THE CAPACITOR WILL FULLY CHARGE BY THE SIGNAL PRESENTS AT RA0 PIN(CHANNEL OF CONVERSION)

#pragma interrupt myFunction
void myFunction(void)
{	
	if(PIR1bits.TMR2IF == 1)		// TIMER 2 INTERRUPT CHECK ?
		PIR1bits.TMR2IF = 0;
	else if(PIR1bits.ADIF == 1)		// ADC INTERRUPT CHECK ?
	{
		PIR1bits.ADIF = 0;
		result = ADRESH;	
		result <<= 8;
		result += ADRESL;
		function();
		timeAcquisition();				// WAIT AMOUNT OF TIME
		ADCON0bits.GO = 1;				// START CONVERSION
	} 
}

#pragma code myVector = 0x00008			// INTERRUPT VECTOR TABLE
void myVector(void)
{
	_asm 
		GOTO myFunction
	_endasm
}
#pragma code

void main(void)
{
	TRISAbits.TRISA0 = 1;				// RA0 AS ANALOG INPUT
	TRISCbits.TRISC2 = 0;				// RC2 AS DIGITAL OUTPUT 
	TRISD = 0xF0;						// LOWEST NIBBLE AS DIGITAL OUTPUT
	ADCON0 = 0x41;						// CONVERSION CLOCK, CHANNEL INPUT, ADC ON 
	ADCON1 = 0x8E;						// RIGHT JUSTIFIED, RA0 AS ANALOG REST DIGITAL, Vref = Vdd
	CCP1CON = 0x0F;						// PWM MODE
	PR2 = 124;							// 2 ms PERIODE OF PWM
	T2CON = 0x03;						// 1:16 PRESCALER NO POSTSCALER
	INTCONbits.GIE = 1;					// GLOBAL INTERRUPT IS ALLOWED
	INTCONbits.PEIE = 1;				// PERIPHERAL INTERRUPT IS ALLOWED
	PIE1bits.ADIE = 1;					// ADC INTERRUPT IS ALLOWED
	PIR1bits.ADIF = 0;					
	PIE1bits.TMR2IE = 1;				// TIMER 2 INTERRUPT IS ALLOWED
	PIR1bits.TMR2IF = 0;	
	TMR2 = 0x00;					
	timeAcquisition();				
	ADCON0bits.GO = 1;
	while(1);							// RUN INFITIVELY
}
void timeAcquisition(void)
{
	T0CON = 0x08;
	TMR0L = 236;
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 1;
	while(INTCONbits.TMR0IF == 0);
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 0;
}
void function(void)
{
	static unsigned char state = 0;
	result = (unsigned int)((result*value)/MAX);				// CONVERT THE ADC OUTPUT INTO RANGE BETWEEN [0 9] 
	if(state == 0)												// THIS SECTION FOR THE FIRST TIME ADC ON 
	{
		LATD = result;
		state = 1;
		oldValue = result;										// STORE THE VALUE IN oldValue
		T2CONbits.TMR2ON = 1;									// START TIMER2 FOR GENERATING PWM WAVEFORM
		CCPR1L = arraySpeedValue[result];						// TURN SPEED AS THE SPECIFIED RESULT VALUE
	}
	else
	{
		if(result != oldValue)									// IF THE NEW VALUE DOES NOT MATH THE OLD VALUE EXECUTE THE BODY
		{
			LATD = result;
			oldValue = result;
			CCPR1L = arraySpeedValue[result];
		}
	}
}