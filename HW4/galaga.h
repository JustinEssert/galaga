// Copyright (c) 2015-16, Joe Krachey
// All rights reserved.
//
// Redistribution and use in source or binary form, with or without modification, 
// are permitted provided that the following conditions are met:
//
// 1. Redistributions in source form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in 
//    the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "TM4C123.h"

// DEFINE PRESCALE VALUES =====================================================
// Define a value of 100 to TimerA/B's prescale register to allow for a greater
// maximum clock period.
#define TIMER_TAPR_PRESCALE 100
#define TIMER_TBPR_PRESCALE 100

// DEFINE INTERVAL VALUES =====================================================
// Define a ILR Value for Timer A to represent 10 ms
#define TIMER_TAILR_10MS_W_PRESCALE 5000		// (5000*100)  / 50*10^6 = 10*10^3

// Define a ILR Value for Timer B to represent 30 ms
#define TIMER_TBILR_30MS_W_PRESCALE 15000		// (15000*100) / 50*10^6 = 30*10^3

// DEFINE INTERRUPT CYCLES ====================================================
// Define a value of 10 for TimerA/B's Interrupt Handler
#define TIMER_A_CYCLES 10
#define TIMER_B_CYCLES 10

// DEFINE STATUS BITS FOR PS2 READ ============================================
// Define a value of 10 for TimerA/B's Interrupt Handler
#define MOVE_Y_M						3
#define Y_LESS_THAN_1_4			1
#define Y_GREATER_THAN_3_4	2
#define MOVE_X_M						12
#define X_LESS_THAN_1_4			4
#define X_GREATER_THAN_3_4	8

// DEFINE LCD VARS ============================================================
#define LCD_WIDTH						240;
#define LCD_HEIGHT					360;
#define LCD_TOTAL_PIX				LCD_WIDTH*LCD_HEIGHT;


extern void serialDebugInit(void);

#endif
