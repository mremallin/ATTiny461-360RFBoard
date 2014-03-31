/******************************************************************************
 * main.c - Main source file for the Wireless 360 Controller Adapter
 *
 * Heavily influenced by:
 *   - http://dilisilib.wordpress.com/hacking/xbox-360-rf-module-arduino/
 *
 * Author: Mike Mallin
 * Date:   March 2014
 ******************************************************************************/

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Mike Mallin
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <inttypes.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>

#define SYNC_CMD     0x004
#define LED_CMD      0x084
#define LED_ANIM_CMD 0x085

#define RF_CMD_BITS  9

#define DATA_PORT    PORTB
#define DATA_DDR     DDRB
#define DATA_PIN     PB0
#define CLK_PORT     PORTA
#define CLK_PORT_PIN PINA
#define CLK_DDR      DDRA
#define CLK_PIN      PA0

#define digitalRead(pin, port_pin)  \
    ((port_pin & (1 << pin)) != 0)

#define digitalWrite(pin, data)     \
    if (data == 1) {                \
        (DATA_PORT |= (1 << pin));  \
    } else {                        \
        (DATA_PORT &= ~(1 << pin)); \
    }

#define OUTPUT  1
#define INPUT   0

#define pinMode(ddr, pin, dir)      \
    if (dir == OUTPUT) {            \
        (ddr |= (1 << pin));        \
    } else {                        \
        (ddr &= ~(1 << pin));       \
    }

typedef uint8_t bool;
#define TRUE  1
#define FALSE 0

void
_delay_10ms(uint16_t count)
{
    while (count--) {
        _delay_ms(10);
    }
}

void
send_data(uint32_t data)
{
    uint8_t i;
    
    //Output
    pinMode(DATA_DDR, DATA_PIN, OUTPUT);
    //Send start bit
    digitalWrite(DATA_PIN, 0);

    for (i = 0; i <= RF_CMD_BITS; i++) {
        // Waits for external clock
        while(digitalRead(CLK_PIN, CLK_PORT_PIN) != 0x00) {}

        if ((data & (1 << (RF_CMD_BITS - i))) != 0) {
            digitalWrite(DATA_PIN, 1);
        } else {
            digitalWrite(DATA_PIN, 0);
        }

        //Wait for rising edge
        while(digitalRead(CLK_PIN, CLK_PORT_PIN) == 0x00) {}
    }

    //Done
    digitalWrite(DATA_PIN, 1);
    pinMode(DATA_DDR, DATA_PIN, INPUT);
}

void
init_system()
{
    // Output high for data transmission
    pinMode(DATA_DDR, DATA_PIN, INPUT);

    // Input pins for interrupt triggering
    pinMode(CLK_DDR, CLK_PIN, INPUT);

    // Enable external interrupts

    //Can't use this or else can't wake from sleep
    //MCUCR  |= (1 << ISC00); // Interrupt on rising edge
    GIMSK  |= (1 << INT0);  // INT0 enable

    //Power saving
    power_all_disable();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    //Wait 2s for the RF board to init
    _delay_10ms(200);
}

void
init_rf_led()
{
    /* Send LED init command */
    send_data(LED_CMD);
    _delay_ms(50);

    /* Send LED startup animation command */
    send_data(LED_ANIM_CMD);
    _delay_ms(50);
}

void
send_rf_sync()
{
    send_data(SYNC_CMD);
    _delay_ms(50);
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

    //Time to snooze
    sleep_enable();

    while (1) {
        sleep_cpu();
    }

    return 0;
}
