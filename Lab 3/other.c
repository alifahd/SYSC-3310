#include "msp.h"

void PORT1_IRQHandler(void) {
	static int which_led = 0;
	static uint8_t rgb = 0x00;
	if ((P1IFG & (uint8_t) (1<<1)) != 0) {
		P1IFG &= ~(uint8_t) (1<<1);
		if (which_led == 1) which_led = 0; else which_led = 1;
	}
	else {
		if ((P1IFG & (uint8_t) (1<<4)) != 0) {
			P1IFG &= ~(uint8_t) (1<<4);
			if (which_led == 0) {
				P1OUT ^= (uint8_t) (1<<0);
			}
			else {
				if (rgb == 0x07) {
					rgb = 0x00;
				}
				else {
					rgb += 0x01;
				}
				P2OUT &=~ (uint8_t) ((1<<2) | (1<<1) | (1<<0));
				P2OUT |= rgb;
			}
		}
	}
}

int main(void) {
	//disable watchdog timer
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
	
	//GPIO
	P1SEL0 &= (uint8_t) (~((1<<4) | (1<<1) | (1<<0)));
	P1SEL1 &= (uint8_t) (~((1<<4) | (1<<1) | (1<<0)));
	P2SEL0 &= (uint8_t) (~((1<<2) | (1<<1) | (1<<0)));
	P2SEL1 &= (uint8_t) (~((1<<2) | (1<<1) | (1<<0)));
	
	//direction
	P1DIR &= (uint8_t) (~((1<<4) | (1<<1)));//input
	P1DIR |= (uint8_t) (1<<0);//output
	P2DIR |= (uint8_t) ((1<<2) | (1<<1) | (1<<0));//output
	
	//initialize outputs
	P1OUT &=~ (uint8_t) (1<<0);
	P2OUT &=~ (uint8_t) ((1<<2) | (1<<1) | (1<<0));
	
	//initialize input
	P1OUT |= (uint8_t) ((1<<4) | (1<<1));
	
	//enable resistors
	P1REN |= (uint8_t) ((1<<4) | (1<<1));
	
	//Configure Interrupts
	P1IES |= (uint8_t) ((1<<4) | (1<<1));
	P1IFG &= (uint8_t) (~((1<<4) | (1<<1)));
	P1IE |= (uint8_t) ((1<<4) | (1<<1));
	
	//Configure NVIC
	NVIC_SetPriority(PORT1_IRQn, 2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);
	
	//enable interrupts
	__ASM("CPSIE I");
	
	while(1);
}
