#include <msp.h>

//MARK: Constants
#define DEBOUNCE_TIME 1500

#define LED_RED 0
#define LED_RGB 1

#define RGB_LED_MASK 0x07
#define RGB_LED_OFFSET 0

// MARK: Function prototypes
static void select_led_button_action(void);
static void pause_button_action(void);

// MARK: ISR Prototypes
void HardFault_Handler(void);
void PORT1_IRQHandler(void);
void TA0_N_IRQHandler(void);

// MARK: Variable Definitions
static volatile uint8_t selected_led_g;


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
    
    /* LEDs (P1.0, P2.0, P2.1, P2.2) */
    // Set direction to output
    P1->DIR |= (1<<0);
    P2->DIR |= ((1<<0) | (1<<1) | (1<<2));
    // Ensure that high drive strength is disabled
    P1->DS &= ~(1<<0);
    P2->DS &= ~((1<<0) | (1<<1) | (1<<2));
    // Initialize to driven low
    P1->OUT &= ~(1<<0);
    P2->OUT &= ~((1<<0) | (1<<1) | (1<<2));
    // Ensure that interrupts are disabled
    P1->IE &= ~((1<<0));
    P1->IE &= ~((1<<0) | (1<<1) | (1<<2));
}

int main(void)
{
    /* Stop watchdog timer */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
    
    /* Configure GPIO */
    init_gpio();
    
    selected_led_g = LED_RED;
    
    /* Configure Interrupts */
    // Trigger interrupts on falling edge
    P1->IES |= ((1<<1) | (1<<4));
    // Clear interrupt flags
    P1->IFG &= ~((1<<1) | (1<<4));
    // Enable pin interrupts
    P1->IE |= ((1<<1) | (1<<4));
    
    NVIC_SetPriority(PORT1_IRQn, 2);
    NVIC->ICPR[PORT1_IRQn > 31] |= ((PORT1_IRQn % 32) << 1);
    NVIC_EnableIRQ(PORT1_IRQn);
    
    /* Configure and Start Timer */
    // Run from ACLK, divide by one, up mode, interrupt enabled
    TIMER_A0->CTL = (TIMER_A_CTL_TASSEL_1 | TIMER_A_CTL_ID_0 |
                     TIMER_A_CTL_MC_1 | TIMER_A_CTL_IE);
    // Set top to count for one second and start timer
    TIMER_A0->CCR[0] = 32768;
    
    NVIC_SetPriority(TA0_N_IRQn, 2);
    NVIC->ICPR[TA0_N_IRQn > 31] |= ((TA0_N_IRQn % 32) << 1);
    NVIC_EnableIRQ(TA0_N_IRQn);
    
    /* Main Loop */
    for (;;) {
        __WFI();
    }
}

void select_led_button_action(void)
{
    selected_led_g = (selected_led_g == LED_RED) ? LED_RGB : LED_RED;
}

void pause_button_action(void)
{
    TIMER_A0->CTL ^= (TIMER_A_CTL_MC_1);
}


/* Interrupt Service Routines */
void PORT1_IRQHandler(void)
{
    if (P1->IFG & (1<<1)) {
        // Select LED Button
        if (!(P1->IN & (1<<1))) {
            select_led_button_action();
        }
        P1->IFG &= ~(1<<1);
    }
    
    if (P1->IFG & (1<<4)) {
        // Select Mode Button
        if (!(P1->IN & (1<<4))) {
            pause_button_action();
        }
        P1->IFG &= ~(1<<4);
    }
}


void TA0_N_IRQHandler(void)
{
    if (TIMER_A0->CTL & TIMER_A_CTL_IFG) {
        if (selected_led_g == LED_RED) {
            P1->OUT ^= (1<<0);
        } else if (selected_led_g == LED_RGB) {
            uint8_t led_state = ((P2->OUT & RGB_LED_MASK) >> RGB_LED_OFFSET);
            led_state++;
            P2->OUT &= ~RGB_LED_MASK;
            P2->OUT |= (led_state & RGB_LED_MASK) << RGB_LED_OFFSET;
        }
        
        TIMER_A0->CTL &= ~(TIMER_A_CTL_IFG);
    }
}

void HardFault_Handler(void)
{
    // Flash the LED in a noticeable pattern
    for (;;) {
        P1->OUT |= (1<<0);
        P1->OUT &= ~(1<<0);
    }
}
