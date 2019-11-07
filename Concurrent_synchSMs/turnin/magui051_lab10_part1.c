/*	Author: magui051
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerISR() {
	TimerFlag = 1;
}

void TimerOff(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char threeLEDs = 0x00; //for 1,2,3 leds
unsigned char blinkingLED = 0x00; //for 4th led

enum BL_States { BL_SMStart, BL_LedOff, BL_LedOn } BL_State;	//for 4th led
void TickFct_BlinkLed() {// Standard switch statements for SM
	switch(BL_State){	//transitions
		case BL_SMStart:
			BL_State = BL_LedOff;
			break;
		case BL_LedOff:
			BL_State = BL_LedOn;
			break;
		case BL_LedOn:
			BL_State = BL_LedOff;
			break;
	}
	switch(BL_State){	//actions
		case BL_SMStart:
			break;
		case BL_LedOff:
			blinkingLED = 0x00;
			break;
		case BL_LedOn:
			blinkingLED = 0x08;
			break;
	}
}

enum TL_States { TL_SMStart, TL_T0, TL_T1, TL_T2 } TL_State;
void TickFct_ThreeLeds() {// Standard switch statements for SM
	switch(TL_State){//transitions
		case TL_SMStart:
			TL_State = TL_T0;
			break;
		case TL_T0:
			TL_State = TL_T1;
			break;
		case TL_T1:
			TL_State = TL_T2;
			break;
		 case TL_T2:
			TL_State = TL_T0;
			break;
	}
	switch (TL_State) {
		case TL_SMStart:
			break;
		case TL_T0:
			threeLEDs = 0x01;
			break;
		case TL_T1:
			threeLEDs = 0x02;
			break;
		case TL_T2:
			threeLEDs = 0x04;
			break;
	}
}
	
enum CL_States { CL_SMStart, CL_Blink, CL_Three } CL_State;
void CombineLEDsSM(){
	switch(CL_State){
		case CL_SMStart:
			CL_State = CL_Blink;
			break;
		case CL_Blink:
			CL_State = CL_Three;
			break;
		case CL_Three:
			CL_State = CL_Blink;
			break;
	}
	switch(CL_State){
		case CL_SMStart:
			break;
		case CL_Blink:
			TickFct_BlinkLed();
			TickFct_ThreeLeds();
			PORTB = blinkingLED + threeLEDs;
			break;
		case CL_Three:
			break;		
	}
}

int main(void) {
	DDRB = 0xFF; // Init outputs
	PORTB = 0x00;
	TimerSet(1000);
	TimerOn();
	BL_State = BL_SMStart;
	TL_State = TL_SMStart;
	CL_State = CL_SMStart;
	while (1) {
		//TickFct_BlinkLed();    // Tick the BlinkLed synchSM
		//TickFct_ThreeLeds();   // Tick the ThreeLeds synchSM
		CombineLEDsSM();
		while (!TimerFlag){}   // Wait for timer period
		TimerFlag = 0;         // Lower flag raised by timer
		
	}
}
