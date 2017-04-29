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

#include "main.h"
#include "timers.h"
#include "ps2.h"
#include "launchpad_io.h"
#include "lcd.h"
#include "galaga_bitmaps.h"

// 240 rows of 10*32 bits
static uint32_t shadow_memory[240][10];


void menu_init() {
	uint32_t x, y;
	char msg[] = "HIGH SCORE";
	char score[] = "000000";
	// Clear the screen to all black
	lcd_clear_screen(LCD_COLOR_BLACK);
	
	// 
	for(y=28; y < 30; y++){
		for(x=0; x < 240; x++){		
			lcd_draw_px(x, y, LCD_COLOR_WHITE);
		}
	}

	for(y=283; y < 285; y++){
		for(x=0; x < 240; x++){		
			lcd_draw_px(x, y, LCD_COLOR_WHITE);
		}
	}
	
	lcd_print_stringXY(msg, 2, 0, LCD_COLOR_RED, LCD_COLOR_BLACK );
	lcd_print_stringXY(score, 4, 1, LCD_COLOR_WHITE, LCD_COLOR_BLACK );
	
	lcd_print_Image(100, 40);

}
