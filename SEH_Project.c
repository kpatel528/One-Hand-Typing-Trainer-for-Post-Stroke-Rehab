/*
 * One-Hand Typing Trainer for Post-Stroke Rehab
 * SEH500 Course Project - Fall 2025
 * Group 11: Krish Patel, Mit Patel
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK66F18.h"
#include "fsl_debug_console.h"

/* Assembly LED control functions */
void asm_led_init(void);
void asm_led_set_red(uint8_t state);
void asm_led_set_green(uint8_t state);
void asm_led_set_blue(uint8_t state);
void asm_led_all_off(void);

/* Global timing/session variables */
volatile uint32_t tick_count = 0;         // Global millisecond tick
volatile uint32_t beat_counter = 0;       // Counts total beats in session
volatile uint32_t last_beat_time = 0;     // Timestamp of last LED beat
volatile uint8_t session_active = 0;      // 1 = session running

uint32_t bpm = 60;                        // Beats per minute
uint32_t beat_interval_ms = 1000;         // Interval between beats
uint32_t total_presses = 0;               // Number of button presses
uint32_t accurate_presses = 0;            // Number of presses within tolerance
uint32_t error_sum = 0;                   // Sum of timing errors
uint32_t error_count = 0;                 // Number of recorded errors

#define MAX_SESSION_BEATS 120             // Total beats per session
#define TOLERANCE_MS 100                  // Max acceptable timing error

/* Convenience wrapper to set RGB LED color using assembly functions */
void LED_SetColor(uint8_t red, uint8_t green, uint8_t blue)
{
    asm_led_set_red(red);
    asm_led_set_green(green);
    asm_led_set_blue(blue);
}

/* PIT0 Interrupt Handler - triggers every 1 ms */
void PIT0_IRQHandler(void)
{
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    tick_count++;

    /* Generate a beat at the programmed interval */
    if(session_active && (tick_count - last_beat_time >= beat_interval_ms))
    {
        last_beat_time = tick_count;
        beat_counter++;

        LED_SetColor(0, 0, 1);   // Blue flash to indicate beat timing

        /* End session after preset beat count */
        if(beat_counter >= MAX_SESSION_BEATS)
        {
            session_active = 0;
            asm_led_all_off();

            PRINTF("\r\n=== Session Complete ===\r\n");
            PRINTF("Total presses: %d\r\n", total_presses);
            PRINTF("Accurate presses: %d\r\n", accurate_presses);
            PRINTF("Accuracy: %d%%\r\n", (accurate_presses * 100) / total_presses);
            PRINTF("Mean error: %d ms\r\n", error_sum / error_count);
            PRINTF("Type 's' to start\r\n");
        }
    }

    /* Turn off blue LED 50 ms after flash */
    if(session_active &&
       (tick_count - last_beat_time >= 50) &&
       (tick_count - last_beat_time < beat_interval_ms))
    {
        asm_led_all_off();
    }
}

/* SW2 Handler (PTD11) - user presses button in sync with beat */
void PORTD_IRQHandler(void)
{
    uint32_t flags = GPIO_PortGetInterruptFlags(GPIOD);
    GPIO_PortClearInterruptFlags(GPIOD, flags);

    if((flags & (1U << 11U)) && session_active)
    {
        /* Calculate how close the press is to the beat */
        uint32_t time_since_beat = tick_count - last_beat_time;

        /* Wrap-around correction to measure shortest distance to beat */
        uint32_t timing_error = (time_since_beat > beat_interval_ms / 2) ?
                                (beat_interval_ms - time_since_beat) :
                                 time_since_beat;

        total_presses++;
        error_sum += timing_error;
        error_count++;

        /* Determine accuracy and give feedback */
        if(timing_error <= TOLERANCE_MS)
        {
            LED_SetColor(0, 1, 0);     // Green = correct timing
            accurate_presses++;
            PRINTF("Good! Error: %d ms\r\n", timing_error);
        }
        else
        {
            LED_SetColor(1, 0, 0);     // Red = off-beat
            PRINTF("Off beat. Error: %d ms\r\n", timing_error);
        }
    }
}

/* SW3 Handler (PTA10) - emergency stop button */
void PORTA_IRQHandler(void)
{
    uint32_t flags = GPIO_PortGetInterruptFlags(GPIOA);
    GPIO_PortClearInterruptFlags(GPIOA, flags);

    if((flags & (1U << 10U)) && session_active)
    {
        session_active = 0;
        asm_led_all_off();

        PRINTF("\r\n=== Session Stopped ===\r\n");
        PRINTF("Total presses: %d\r\n", total_presses);
        PRINTF("Accurate presses: %d\r\n", accurate_presses);

        if(total_presses > 0)
        {
            PRINTF("Accuracy: %d%%\r\n", (accurate_presses * 100) / total_presses);
            PRINTF("Mean error: %d ms\r\n", error_sum / error_count);
        }

        PRINTF("Type 's' to start\r\n");
    }
}

/* Processes incoming serial commands from the user */
void handle_serial_command(char ch)
{
    if(ch == 's' || ch == 'S')
    {
        /* Start a new training session */
        PRINTF("\r\n=== Session Started ===\r\n");
        PRINTF("Press SW2 in sync with blue LED\r\n");
        PRINTF("Press SW3 to stop\r\n");
        PRINTF("BPM: %d\r\n", bpm);

        /* Reset all counters and activate session */
        tick_count = 0;
        beat_counter = 0;
        last_beat_time = 0;
        total_presses = 0;
        accurate_presses = 0;
        error_sum = 0;
        error_count = 0;
        session_active = 1;
    }
    else if(ch == '+' && bpm < 120)
    {
        /* Increase BPM by 10 */
        bpm += 10;
        beat_interval_ms = 60000 / bpm;
        PRINTF("BPM: %d\r\n", bpm);
    }
    else if(ch == '-' && bpm > 40)
    {
        /* Decrease BPM by 10 */
        bpm -= 10;
        beat_interval_ms = 60000 / bpm;
        PRINTF("BPM: %d\r\n", bpm);
    }
    else if(ch == 'h' || ch == 'H')
    {
        /* Command help menu */
        PRINTF("\r\n=== Commands ===\r\n");
        PRINTF("s - Start session\r\n");
        PRINTF("+ - Increase BPM\r\n");
        PRINTF("- - Decrease BPM\r\n");
        PRINTF("h - Help\r\n");
    }
}

int main(void)
{
    /* Initialize board hardware */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();

    /* Enable interrupts for PIT and buttons - (This code was suggested by Gemini as button)*/
    EnableIRQ(PIT0_IRQn);
    EnableIRQ(PORTA_IRQn);
    EnableIRQ(PORTD_IRQn);
    /*Gemini suggestion ends here*/

    /* Initialize RGB LED via assembly routine */
    asm_led_init();

    /* Startup message */
    PRINTF("\r\n=================================\r\n");
    PRINTF("One-Hand Typing Trainer\r\n");
    PRINTF("SEH500 Project - Group 11\r\n");
    PRINTF("=================================\r\n");
    PRINTF("Type 'h' for help\r\n");

    /* Main loop: wait for serial commands */
    while(1)
    {
        handle_serial_command(GETCHAR());
    }

    return 0;
}
