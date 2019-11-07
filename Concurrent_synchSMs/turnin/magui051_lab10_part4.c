#include <avr/io.h>
#include <AVR/interrupt.h>

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
unsigned char speakerLEDs = 0x00;

unsigned short i = 0;//for blink
unsigned short j = 0;//for three
unsigned char k = 0;//for frequ comparison
unsigned long frequ = 0;

#define Switch (~PINA & 0x04)
#define b1 (~PINA & 0x01)	//lowers frequ
#define b2 (~PINA & 0x02)	//raises frequ


enum Button_States{ Button_SMStart, Button1, Button2, Release1, Release2 } Button_State;
void ButtonLEDsSM(){
	switch(Button_State) {	//transitions
		case Button_SMStart:
			if(b1) {
				Button_State = Button1;
			}
			else if(b2) {
				Button_State = Button2;
			}
			else {
				Button_State = Button_SMStart;
			}
			break;		
		case Button1:
			Button_State = Release1;
			break;
		case Button2:
			Button_State = Release2;
			break;
		case Release1:
			if(!b1){
				Button_State = Button_SMStart;
			}
			else {
				Button_State = Release1;
			}
			break;
		case Release2:
			if(!b2) {
				Button_State = Button_SMStart;
			}
			else {
				Button_State = Release2;
			}
			break;		
	}
	switch(Button_State) {	//actions
		case Button_SMStart:
			break;
		case Button1:
			frequ++;
			break;
		case Button2:
			if(frequ > 0) {
				frequ--;
			}
			break;
		case Release1:
			break;
		case Release2:
			break;
	}
}

enum BL_States { BL_SMStart, BL_LedOff, BL_LedOn } BL_State;	//for 4th led
void TickFct_BlinkLed() {	// Standard switch statements for SM
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
void TickFct_ThreeLeds() {	// Standard switch statements for SM
	switch(TL_State){	//transitions
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
	switch (TL_State) {//actions
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

enum Speaker_States { Speaker_SMStart, Speaker_On, Speaker_Off } Speaker_State;
void SpeakerSM() {
	switch(Speaker_State){ //transitions
		case Speaker_SMStart:
			Speaker_State = Speaker_On;
			break;
		case Speaker_On:
			Speaker_State = Speaker_Off;
			break;
		case Speaker_Off:
			Speaker_State = Speaker_On;
			break;
		default:
			Speaker_State = Speaker_SMStart;
			break;
	}
	switch(Speaker_State) {	//actions
		case Speaker_SMStart:
			speakerLEDs = 0;
			break;
		case Speaker_On:
			speakerLEDs = 0x10;
			break;
		case Speaker_Off:
			speakerLEDs = 0x00;
			break;
		default:
			speakerLEDs = 0;
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
	switch(CL_State){//actions
		case CL_SMStart:
			break;
		case CL_Blink:
			if(i == 1000){	//1 sec
				TickFct_BlinkLed();
				i = 0;
			}
			i++;	//without this the led will only blink once
			if(j == 300){
				TickFct_ThreeLeds();
				j = 0;
			}
			j++;	//w/o this the three leds wont turn on
			if(Switch) {
				//call new button state machine here to change the frequency to low/high
				ButtonLEDsSM();	//determines frequ ouputed by speaker
				if(k == frequ){
					SpeakerSM();	//turn speaker on/off
					k = 0;
				}
				k++;	//w/o tgis speaker wont work
			}
		
			PORTB = blinkingLED | threeLEDs | speakerLEDs;
			break;
		case CL_Three:
			break;
	}
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; // Init outputs
	PORTB = 0x00;
	TimerSet(1);	//1ms
	TimerOn();
	BL_State = BL_SMStart;
	TL_State = TL_SMStart;
	Speaker_State = Speaker_SMStart;
	CL_State = CL_SMStart;
	i = 0;
	j = 0;
	k = 0;
	frequ = 1;
	
	while (1) {
		//TickFct_BlinkLed();    // Tick the BlinkLed synchSM
		//TickFct_ThreeLeds();   // Tick the ThreeLeds synchSM
		CombineLEDsSM();
		while (!TimerFlag){}   // Wait for timer period
		TimerFlag = 0;         // Lower flag raised by timer
		
	}
}
