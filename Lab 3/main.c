// LAB 3
#include "msp.h"

int led_selected = 0; // If 0, RED LED is selected; if 1, RGB LED is selected
int state = 0; //LED is off at 0
 
void PORT1_IRQHandler(void){
	
	//Test for pin 1 interrupt flag P1.1
	if((P1IFG & (uint8_t) (1 << 1)) != 0){
		
		//Yes, pin 1: clear flag
		P1IFG &= ~(uint8_t)(1 << 1);
	
		//if RBG LED selected, toggling that and selecting RED
		if(led_selected == 1){
			P1 -> OUT &= ~(uint8_t)(0x01);
			//P1 -> OUT ^= 0x01;
			led_selected = 0;
		}
		
		//if RED LED selected, toggle that and select RGB LED
		else{
			P1 -> OUT &= ~(uint8_t)(0x00);
			led_selected = 1;
		}
	}
	
	//Something else if pressed other than P1.1
	else{
		//clearing flag
		P1IFG &= (uint8_t)~0x10;
		
		//Toggling RED LED
		if(led_selected == 0){
			P1->OUT ^= 0x01;
		}
		
		//Toggling RGB LED
		else{
		//	P2->OUT ^= 0x01;
			
			//reset to first state if state reaches 8
			if(state == 8){
				state = 0;
			}
			
			else{
				//increment state 
				state++;
				//bitwise shift and toggle through states
				P2->OUT ^= (uint8_t)(1 << state);
			}
		}
	}
		
}

int main(void){
	//Stop watchdog timer
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;  
	//output is 1
	//input is 0
	
	//GPIO function
	//initialize GPIO to 0 
	P1->SEL0 &= (uint8_t)(~((1 << 1) | (1 << 4) | (1 << 0))); //part 2: clear bits 1 and 4 to 0 for switches
	P1->SEL1 &= (uint8_t)(~((1 << 1) | (1 << 4) | (1 << 0))); //part 2: clear bits 1 and 4 to 0 for switches
	
	//Set Output direction- Part2
	P1->DIR &= (uint8_t)(~((1 << 1) | (1 << 4))); //set the pins to either 0 or 1 since input with pull-up resistor
	P1->REN |= (uint8_t)((1 << 1) | (1 << 4)); //set P1->REN to 1 since its input with pull-up resistor
	//SWITCHES ARE ACTIVE LOW
	P1->OUT |= (uint8_t)((1 << 1) | (1 << 4)); //set P1 output to 1 since its input with pull-up resistor
	
	//Part 3 
	P2->SEL0 &= (uint8_t)(~((1 << 0) | (1 << 2) | (1 << 1))); //part3: clearing bit 0 
	P2->SEL1 &= (uint8_t)(~((1 << 0) | (1 << 2) | (1 << 1))); //part3: clearing bit 0
	
	P1->DIR |= (uint8_t)((1 << 0)); //setting bit 0 to be 1
	P2->DIR |= (uint8_t)((1 << 0) | (1 << 1) | (1 << 2)); //setting bit 0, 1 and 2 to be 1
	
	//Configure interrupt configuration for P1.1
	P1IES |= (uint8_t) 0x02; //Interrupt edge type
	P1IFG &= (uint8_t) ~0x02; //Interrupt pending flags
	P1IE |= (uint8_t) 0x02; //IE: Interrupt Enable
	
	//Configure interrupt configuration for P1.4
	P1IES |= (uint8_t) 0x10; //Interrupt edge type
	P1IFG &= (uint8_t) ~0x10; //Interrupt pending flags
	P1IE |= (uint8_t) 0x010; //IE: Interrupt Enable
	
	//Configure NVIC
	NVIC_SetPriority(PORT1_IRQn, 2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);
	
	//Enable global interrupts
	__ASM("CPSIE I");
	
	//Initialize LEDs states
	P1->OUT &= (uint8_t)(~(1 < 0));
	P2->OUT &= (uint8_t)(~((1 << 0) | (1 << 1) | (1 << 2)));
	
	while(1){
			
	}
	
}