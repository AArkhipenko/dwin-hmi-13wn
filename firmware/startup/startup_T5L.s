; -------------------------------------------------------------------
;  DWIN T5L SDCC Startup File
;  Copyright (c) 2025 Recep Şenbaş
;  License: CC BY-NC-SA 4.0 (https://creativecommons.org/licenses/by-nc-sa/4.0/)
;  Source: https://github.com/recepsenbas/DWIN-T5L-SDCC-Template
;  Contact: recepsenbas@gmail.com
;  Description: Startup and interrupt vector table for DWIN T5L (8051-like core)
; -------------------------------------------------------------------
.module startup_T5L
.optsdcc -mmcs51 --model-large

; SFR aliases for clarity
.equ PAGESEL,   0x94
.equ D_PAGESEL, 0x95

.globl __sdcc_gsinit_startup   ; SDCC runtime (initialises C globals)
.globl _main                   ; user main
.globl _Timer2_ISR
.globl _uart2_ISR

; -------------------------------------------------------------------
; Interrupt vector table (absolute addresses)
; -------------------------------------------------------------------
.area HOME (CODE, ABS)

; Reset vector – set external RAM paging and jump to SDCC init
.org 0x0000
    ljmp    __sdcc_gsinit_startup

; External interrupt 0 (unused)
.org 0x0003
    reti

; Timer0 interrupt
.org 0x000B
    reti

; External interrupt 1 (unused)
.org 0x0013
    reti

; Timer1 interrupt
.org 0x001B
    reti

; UART2 interrupt (serial slot)
.org 0x0023
    ljmp    _uart2_ISR

; Timer2 interrupt
.org 0x002B
    ljmp    _Timer2_ISR

; Extended UART4/5 interrupt vectors (T5L-specific)
.org 0x0053
    reti
.org 0x005B
    reti
.org 0x0063
    reti
.org 0x006B
    reti

; UART3 interrupt
.org 0x0083
    reti

; -------------------------------------------------------------------
; DWIN identification signature (kept at 0x00F8, like original firmware)
; -------------------------------------------------------------------
.area CSEG (CODE, ABS)
.org 0x00F8
__T5L_init__:
    .db 0xFF, 0xFF
    .db 'D','W','I','N','T','5'