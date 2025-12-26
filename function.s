/*
 * LED Control Functions in Assembly
 * SEH500 Project - Group 11
 */

.syntax unified             @ unified syntax used
.cpu cortex-m4              @ cpu is cortex-m4
.thumb                      @ use thumb encoding

.text                       @ put code in the code section

/*
 * Initialize all three LEDs (Red, Green, Blue)
 * Red LED: PTC9
 * Green LED: PTE6
 * Blue LED: PTA11
 */
.global asm_led_init
.type asm_led_init, %function

asm_led_init:
    push    {r4-r7, lr}             @ Save registers

    @ Enable clock for Port A, C, and E
    ldr     r1, =0x40048038         @ System Clock Gate Control Register 5
    ldr     r0, [r1]                @ read current register value
    orr     r0, r0, #(1<<9)         @ enable clock for port A (bit 9)
    orr     r0, r0, #(1<<11)        @ enable clock for port C (bit 11)
    orr     r0, r0, #(1<<13)        @ enable clock for port E (bit 13)
    str     r0, [r1]                @ apply the new settings

    @ Configure Red LED (PTC9) as GPIO
    ldr     r1, =0x4004B024         @ PTC9 Pin Control Register
    mov     r0, #0x00000100         @ set pin to GPIO mode
    str     r0, [r1]                @ apply the new settings

    @ Configure Green LED (PTE6) as GPIO
    ldr     r1, =0x4004D018         @ PTE6 Pin Control Register
    mov     r0, #0x00000100         @ set pin to GPIO mode
    str     r0, [r1]                @ apply the new settings

    @ Configure Blue LED (PTA11) as GPIO
    ldr     r1, =0x4004902C         @ PTA11 Pin Control Register
    mov     r0, #0x00000100         @ set pin to GPIO mode
    str     r0, [r1]                @ apply the new settings

    @ Set Red LED (PTC9) as output
    ldr     r1, =0x400FF094         @ GPIOC Port Data Direction Register
    ldr     r0, [r1]                @ read current value
    orr     r0, r0, #(1<<9)         @ set pin 9 to output mode
    str     r0, [r1]                @ apply the new settings

    @ Set Green LED (PTE6) as output
    ldr     r1, =0x400FF114         @ GPIOE Port Data Direction Register
    ldr     r0, [r1]                @ read current value
    orr     r0, r0, #(1<<6)         @ set pin 6 to output mode
    str     r0, [r1]                @ apply the new settings

    @ Set Blue LED (PTA11) as output
    ldr     r1, =0x400FF014         @ GPIOA Port Data Direction Register
    ldr     r0, [r1]                @ read current value
    orr     r0, r0, #(1<<11)        @ set pin 11 to output mode
    str     r0, [r1]                @ apply the new settings

    @ Turn off all LEDs initially (set HIGH for active-low LEDs)
    @ Red LED OFF
    ldr     r1, =0x400FF080         @ GPIOC Port Data Output Register
    ldr     r0, [r1]                @ read current value
    orr     r0, r0, #(1<<9)         @ set pin 9 HIGH (LED OFF)
    str     r0, [r1]                @ apply settings

    @ Green LED OFF
    ldr     r1, =0x400FF100         @ GPIOE Port Data Output Register
    ldr     r0, [r1]                @ read current value
    orr     r0, r0, #(1<<6)         @ set pin 6 HIGH (LED OFF)
    str     r0, [r1]                @ apply settings

    @ Blue LED OFF
    ldr     r1, =0x400FF000         @ GPIOA Port Data Output Register
    ldr     r0, [r1]                @ read current value
    orr     r0, r0, #(1<<11)        @ set pin 11 HIGH (LED OFF)
    str     r0, [r1]                @ apply settings

    pop     {r4-r7, pc}             @ Restore and return

/*
 * Set Red LED state
 * Input: r0 = state (0 = OFF, non-zero = ON)
 */
.global asm_led_set_red
.type asm_led_set_red, %function

asm_led_set_red:
    push    {r4, lr}                @ Save registers

    ldr     r1, =0x400FF080         @ GPIOC Port Data Output Register
    ldr     r2, [r1]                @ read current value

    cmp     r0, #0                  @ check if state is 0
    beq     red_off                 @ branch if OFF

red_on:
    bic     r2, r2, #(1<<9)         @ clear bit 9 (LED ON, active-low)
    b       red_done

red_off:
    orr     r2, r2, #(1<<9)         @ set bit 9 (LED OFF)

red_done:
    str     r2, [r1]                @ apply settings
    pop     {r4, pc}                @ Restore and return

/*
 * Set Green LED state
 * Input: r0 = state (0 = OFF, non-zero = ON)
 */
.global asm_led_set_green
.type asm_led_set_green, %function

asm_led_set_green:
    push    {r4, lr}                @ Save registers

    ldr     r1, =0x400FF100         @ GPIOE Port Data Output Register
    ldr     r2, [r1]                @ read current value

    cmp     r0, #0                  @ check if state is 0
    beq     green_off               @ branch if OFF

green_on:
    bic     r2, r2, #(1<<6)         @ clear bit 6 (LED ON, active-low)
    b       green_done

green_off:
    orr     r2, r2, #(1<<6)         @ set bit 6 (LED OFF)

green_done:
    str     r2, [r1]                @ apply settings
    pop     {r4, pc}                @ Restore and return

/*
 * Set Blue LED state
 * Input: r0 = state (0 = OFF, non-zero = ON)
 */
.global asm_led_set_blue
.type asm_led_set_blue, %function

asm_led_set_blue:
    push    {r4, lr}                @ Save registers

    ldr     r1, =0x400FF000         @ GPIOA Port Data Output Register
    ldr     r2, [r1]                @ read current value

    cmp     r0, #0                  @ check if state is 0
    beq     blue_off                @ branch if OFF

blue_on:
    bic     r2, r2, #(1<<11)        @ clear bit 11 (LED ON, active-low)
    b       blue_done

blue_off:
    orr     r2, r2, #(1<<11)        @ set bit 11 (LED OFF)

blue_done:
    str     r2, [r1]                @ apply settings
    pop     {r4, pc}                @ Restore and return

/*
 * Turn off all LEDs
 */
.global asm_led_all_off
.type asm_led_all_off, %function

asm_led_all_off:
    push    {r4-r6, lr}             @ Save registers

    @ Red LED OFF
    ldr     r1, =0x400FF080         @ GPIOC Port Data Output Register
    ldr     r0, [r1]
    orr     r0, r0, #(1<<9)         @ set bit 9 HIGH (LED OFF)
    str     r0, [r1]

    @ Green LED OFF
    ldr     r1, =0x400FF100         @ GPIOE Port Data Output Register
    ldr     r0, [r1]
    orr     r0, r0, #(1<<6)         @ set bit 6 HIGH (LED OFF)
    str     r0, [r1]

    @ Blue LED OFF
    ldr     r1, =0x400FF000         @ GPIOA Port Data Output Register
    ldr     r0, [r1]
    orr     r0, r0, #(1<<11)        @ set bit 11 HIGH (LED OFF)
    str     r0, [r1]

    pop     {r4-r6, pc}             @ Restore and return

.end
