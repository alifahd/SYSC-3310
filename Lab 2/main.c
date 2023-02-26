// LAB 2
#include "msp.h"
int main(){
	// Stop watchdog timer
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	//GPIO function
	P1->SEL0 &= (uint8_t)(~((1 << 4) | (1 << 1) | (1 << 0)));
	P1->SEL1 &= (uint8_t)(~((1 << 4) | (1 << 1) | (1 << 0)));
	P2->SEL0 &= (uint8_t)(~((1 << 2) | (1 << 1) | (1 << 0)));
	P2->SEL1 &= (uint8_t)(~((1 << 2) | (1 << 1) | (1 << 0)));

	//Set Input direction
	P1->DIR &= (uint8_t)(~((1 << 4) | (1 << 1)));
	P1->DIR |= (uint8_t)((1 << 0));

	//Set Output direction
	P2->DIR |= (uint8_t)((1 << 2) | (1 << 1) | (1 << 0));

	//initialize LEDs
	P1->OUT &= (uint8_t)(~(1 << 0));
	P2->OUT &= (uint8_t)(~(1 << 2) | (1 << 1) | (1 << 0));

	//pull up resistor
	P1->REN |= (uint8_t)((1 << 4) | (1 << 1));
	P1->OUT |= (uint8_t)((1 << 4) | (1 << 1));

	while(1){
		int i = 10000;
		//while both pins are high
		while((P1IN & (uint8_t)(1<<1))&&(P1IN & (uint8_t)(1<<4))){
			//do nothing
		}
		//debouncing
		while (i>0){
			i--;
		}
		//here, at least one pin is low (hopefully)
		if(~(P1->IN & (uint8_t)(1<<1)))	{ 
			continue;
		}
		else {
			P1->OUT ^= (uint8_t) (1<<0);//toggle
		}
	}
}
z