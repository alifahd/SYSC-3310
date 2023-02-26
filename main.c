//Author: Ali Fahd
//Student#: 101107270

#include <msp.h>

//Constants
#define DEBOUNCE_TIME 1500

//Function prototypes
static void forward_state(void);
static void previous_state(void);
static void update_leds(void);

//ISR Prototypes
void PORT1_IRQHandler(void);

//Variable Definitions
static uint8_t led_state;

/**
 *  Initialize the GPIO pins as required for the application.
 */
static void init_gpio (void)
{
    /* Buttons (P1.1, p1.4) */
    // Set direction to input
    P1->DIR &= ~((1<<1) | (1<<4));
    // Enable pull resistor
    P1->REN |= ((1<<1) | (1<<4));
    // Set pull direction to up
    P1->OUT |= ((1<<1) | (1<<4));
    // Ensure that interrupts are disabled
    P1->IE &= ~((1<<1) | (1<<4));
    
    /* LEDs (P1.0, P2.0, P2.1) */
    // Set direction to output
    P1->DIR |= (1<<0);
    P2->DIR |= ((1<<0) | (1<<1));
    // Ensure that high drive strength is disabled
    P1->DS &= ~(1<<0);
    P2->DS &= ~((1<<0) | (1<<1));
    // Initialize to driven low
    P1->OUT &= ~(1<<0);
    P2->OUT &= ~((1<<0) | (1<<1));
    // Ensure that interrupts are disabled
    P1->IE &= ~((1<<0));
    P1->IE &= ~((1<<0) | (1<<1));
}

int main(void)
{
    /* Stop watchdog timer */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
	
		CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
            CS_CTL1_SELS_3 |                // SMCLK = DCO
            CS_CTL1_SELM_3;                 // MCLK = DCO
    CS->KEY = 0;                            // Lock CS module from unintended accesses

    // Configure UART pins
    P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
            EUSCI_B_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK
	
    EUSCI_A0->BRW = 78;                     // 12000000/16/9600
    EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) |
            EUSCI_A_MCTLW_OS16;

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        // Enable USCI_A0 RX interrupt

    // Enable sleep on exit from ISR
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;


    // initialize gpios
    init_gpio();
    
    led_state = 0;
    
    // Trigger interrupts on falling edge
    P1->IES |= ((1<<1) | (1<<4));
    // Clear interrupt flags
    P1->IFG &= ~((1<<1) | (1<<4));
    // Enable pin interrupts
    P1->IE |= ((1<<1) | (1<<4));
    
  

    // Enable global interrupt
    __enable_irq();

    // Enable eUSCIA0 interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);
		NVIC_SetPriority(PORT1_IRQn, 2);
    NVIC->ICPR[PORT1_IRQn > 31] |= ((PORT1_IRQn % 32) << 1);
    NVIC_EnableIRQ(PORT1_IRQn);

    // Enter LPM0
    __sleep();
    __no_operation();                       // For debugger
	
    
    // Main Loop
    while(1);
}

/**
*  State 0: Both LEDs should be off, clear them both
*	 State 1: Red LED should be on, clear RGB LED set Red LED
*	 State 2: RGB LED should be on, clear Red LED set RGB LED
*  State 3: Both LEDs should be on, set them both
 */
void update_leds(void)
{
    if (led_state == 0) {
        P1->OUT &= ~(uint8_t)(1<<0);
				P2->OUT &= ~(uint8_t)((1<<0) | (1<<1));
    } else if (led_state == 1){
        P1->OUT |= (uint8_t)(1<<0);
				P2->OUT &= ~(uint8_t)((1<<0) | (1<<1));
    } else if (led_state == 2){
        P1->OUT &= ~(uint8_t)(1<<0);
        P2->OUT |= (uint8_t)((1<<0) | (1<<1));
    } else if (led_state == 3){
        P1->OUT |= (uint8_t)(1<<0);
        P2->OUT |= (uint8_t)((1<<0) | (1<<1));
    }
		EUSCI_A0->TXBUF = led_state+'0';// update for monitor app, add 0 to put in char form so python can read it
}

/**
*  Increment state by 1 or if at max reset it
 */
void forward_state(void)
{
    if (led_state == 3) {
		led_state = 0;
    } else {
        led_state++;
    }
	update_leds();
}

/**
*  Decrement state by 1 or if at min reset it
 */
void previous_state(void)
{
    if (led_state == 0) {
			led_state = 3;
    } else {
			led_state--;
    }
	update_leds();
}


/* Interrupt Service Routines */
void PORT1_IRQHandler(void)
{
    if (P1->IFG & (1<<1)) {
				uint16_t n = DEBOUNCE_TIME;
        while (n--);//debouncing, convinving compiler something is here
        // Select Forward State Button
        if (!(P1->IN & (1<<1))) {
            forward_state();
        }
        P1->IFG &= ~(1<<1);//clear
    }
    
    if (P1->IFG & (1<<4)) {
				uint16_t n = DEBOUNCE_TIME;
        while (n--);//debouncing, convinving compiler something is here
        // Select Previous State Button
        if (!(P1->IN & (1<<4))) {
            previous_state();
        }
        P1->IFG &= ~(1<<4);//clear
    }
}

// UART interrupt service routine
void EUSCIA0_IRQHandler(void)
{
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG)
    {
        // Check if the TX buffer is empty first
        while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));

			//update LEDs according to inputted state from monitor app
        if(EUSCI_A0->RXBUF == '0'){
					led_state = 0;
					update_leds();
				}else if(EUSCI_A0->RXBUF == '1'){
					led_state = 1;
					update_leds();
				}else if(EUSCI_A0->RXBUF == '2'){
					led_state = 2;
					update_leds();
				}else if(EUSCI_A0->RXBUF == '3'){
					led_state = 3;
					update_leds();
				}else if(EUSCI_A0->RXBUF == 'n'){
					forward_state();
				}else if(EUSCI_A0->RXBUF == 'p'){
					previous_state();
				}
    }
}
