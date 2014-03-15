#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <util/delay.h>

#include <stdint.h>

#define SYNC_CMD     0x004
#define LED_CMD      0x084
#define LED_ANIM_CMD 0x085

#define TIMEOUT_MAX  60000

#define RF_CMD_BITS  11

#define DATA_PORT    PORTB
#define DATA_DDR     DDRB
#define DATA_PIN     PB0
#define CLK_PIN      PA0

//For debugging
#define TIMEOUT_PIN  PA4    
#define CMD_IND_PIN  PA5

typedef uint8_t bool;
#define TRUE  1
#define FALSE 0

uint16_t timeout_count = 0;

bool
verify_timeout()
{
    if (timeout_count++ == TIMEOUT_MAX) {
        return TRUE;
    }
    return FALSE;
}

void
send_data(uint32_t data)
{
    uint8_t i;
    
    //For debugging
    PORTA ^= (1 << CMD_IND_PIN);

    //Send start bit
    DATA_PORT &= (0 << DATA_PIN);

    for (i = 0; i < RF_CMD_BITS; i++) {
        timeout_count = 0;

        // Waits for external clock
        while((PINA & (1 << CLK_PIN)) == 0x01) {
            if (verify_timeout()) {
                PORTA |= (1 << TIMEOUT_PIN);
                return;
            }
        }

        if (i == 10) {
            DATA_PORT |= (1 << DATA_PIN);
        } else {
            DATA_PORT = ((data & (1 << (9 - i))) << DATA_PIN);
        }
    }
    PORTA ^= (1 << CMD_IND_PIN);
}

void
init_system()
{
    // Output high for data transmission
    DATA_DDR  = (1 << DATA_PIN);
    DATA_PORT = (1 << DATA_PIN);

    // Input pins for interrupt triggering
    DDRA  = (0 << CLK_PIN) | (1 << TIMEOUT_PIN) | (1 << CMD_IND_PIN);
    PORTA = (1 << CLK_PIN) | (1 << TIMEOUT_PIN) | (1 << CMD_IND_PIN);

    // Enable external interrupts
    MCUCR  |= (1 << ISC00); // Interrupt on rising edge
    GIMSK  |= (1 << INT0);                 // INT0 enable
}

void
init_rf_led()
{
    _delay_ms(400);
    PORTA &= (0 << TIMEOUT_PIN) | (0 << CMD_IND_PIN);
    _delay_ms(50);

    /* Send LED init command */
    send_data(LED_CMD);
    _delay_ms(100);

    /* Send LED startup animation command */
    send_data(LED_ANIM_CMD);
    _delay_ms(100);
}

void
send_rf_sync()
{
    send_data(SYNC_CMD);
    _delay_ms(500);
}

/* Interrupt when power (sync) button is pressed */
ISR(INT0_vect)
{
    send_rf_sync();
}

int
main ()
{
    cli();      //Clear interrupts

    init_system();
    init_rf_led();

    sei();      //Enable interrupts

    while (1) {
    }

    return 0;
}
